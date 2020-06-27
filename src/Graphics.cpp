#include "Graphics.h"

GLfloat Graphics::s_quadVertexBufferData[8] = {-1.0f, 1.0f, -1.0f, -1.0f,
                                               1.0f,  1.0f, 1.0f,  -1.0f};

Graphics::Graphics(int height, int width)
    : m_height(height),
      m_width(m_width),
      m_texOut(0),
      m_timeOffset(0),
      m_sizeChanged(true),
      m_wigglyMovement(false),
      m_numBalls(0),
      m_metaballs(0),
      m_colors(0) {
    // initialize the window
    m_window = new GUIWindow("Metaballs", height, width,
                             Window::DefaultWindowFlags() | SDL_WINDOW_UTILITY);
    SDL_SetWindowMinimumSize(*m_window, 700, 400);
    SDL_GL_MakeCurrent(*m_window, m_window->getContext());
    ImGui::StyleColorsDark();

    // attach window draw functions
    m_gradientSpeed = 0.01f;
    m_sliderQuad[0] = 42;
    m_sliderQuad[1] = 69;
    m_sliderQuad[2] = 420;
    m_sliderQuad[3] = 9001;
    m_window->setDrawFunc(m_drawFunc);
    m_window->setGUIFunc(m_drawGUIFunc);

    // prepare the graphics pipeline
    std::ifstream passthroughFS("shaders/pass_through.vert");
    std::ifstream tex2ScreenFS("shaders/tex2screen.frag");
    Shader::shader passthroughVertex(passthroughFS, GL_VERTEX_SHADER);
    Shader::shader tex2ScreenFrag(tex2ScreenFS, GL_FRAGMENT_SHADER);
    passthroughVertex.compile();
    tex2ScreenFrag.compile();
    m_tex2ScreenRender =
        new Shader::GraphicsProgram({passthroughVertex, tex2ScreenFrag});
    m_tex2ScreenRender->build();
    passthroughFS.close();
    tex2ScreenFS.close();

    // prepare the compute shader
    std::ifstream computeFS("shaders/standard_render.comp");
    Shader::shader computeShader(computeFS, GL_COMPUTE_SHADER);
    computeShader.compile();
    m_computeShaders.resize(NumShaderTypes);
    m_computeShaders[Default] = new Shader::ComputeProgram(computeShader);
    m_computeShaders[Default]->build();
    computeFS.close();

    // attach uniform variables to shaders
    m_renderUniformSize = glGetUniformLocation(*m_tex2ScreenRender, "tex_size");
    if (m_renderUniformSize == INVALID_UNIFORM_LOCATION) {
        throw std::runtime_error(
            std::string("Uniform time could not be found"));
    }
    m_computeUniformTime =
        glGetUniformLocation(*m_computeShaders[ShaderType::Default], "time");
    if (m_computeUniformTime == INVALID_UNIFORM_LOCATION) {
        throw std::runtime_error(
            std::string("Uniform time could not be found"));
    }

    // prepare vertex array
    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);
    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_quadVertexBufferData),
                 s_quadVertexBufferData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &quadVBO);

    // attach parameters for drawing functions to windows
    m_params = (drawParams){this};
    m_window->setDrawParams((void*)&m_params);
    m_window->setGUIParams((void*)&m_params);

    for(int i = 0; i < 30; i++) {
        pushBall();
    }
}

Graphics::~Graphics() {
    glDeleteTextures(1, &m_texOut);
    glDeleteVertexArrays(1, &m_quadVAO);
    delete m_window;
    delete m_tex2ScreenRender;
    for (auto ptr : m_computeShaders) {
        delete ptr;
    }
}

GUIWindow* Graphics::Window() { return m_window; }

int Graphics::height() { return m_height; }

int Graphics::width() { return m_width; }

void Graphics::updateDimensions() {
    SDL_GetWindowSize(*m_window, &m_width, &m_height);
    m_sizeChanged = true;
}

void Graphics::m_drawFunc(void* _params) {
    drawParams* params = (drawParams*)_params;
    Graphics* graphics = params->graphics;

    int width = graphics->m_window->getWidth();
    int height = graphics->m_window->getHeight();
    // this is made of memory leaks, should be stored in object
    // when finalized for proper resource freeing
    if (graphics->m_sizeChanged || graphics->m_texOut == 0) {
        if (graphics->m_texOut != 0) {
            glDeleteTextures(1, &graphics->m_texOut);
        }
        glGenTextures(1, &graphics->m_texOut);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, graphics->m_texOut);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width - 300, height, 0,
                     GL_RGBA, GL_FLOAT, NULL);
        glBindImageTexture(0, graphics->m_texOut, 0, GL_FALSE, 0, GL_WRITE_ONLY,
                           GL_RGBA32F);
        graphics->m_height = height;
        graphics->m_width = width;
        graphics->m_sizeChanged = false;
    }

    // empty the window
    glClearColor(0, 123, 225, 225);
    glClear(GL_COLOR_BUFFER_BIT);

    // compute the gradient
    {
        graphics->m_computeShaders[Default]->setActiveProgram();
        graphics->m_timeOffset += graphics->m_gradientSpeed;
        float i = std::sin(graphics->m_timeOffset);
        i += 1.0f;
        i /= 2.0f;
        glUniform1f(graphics->m_computeUniformTime, i);
        glDispatchCompute((GLuint)width - 300, (GLuint)height, 1);
    }
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // render the texture
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(300, 0, width - 300, height);
        graphics->m_tex2ScreenRender->setActiveProgram();
        glUniform2f(graphics->m_renderUniformSize, (float)width, (float)height);
        glBindVertexArray(graphics->m_quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, graphics->m_texOut);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

// draw the test panel
void Graphics::m_drawGUIFunc(void* _params) {
    drawParams* params = (drawParams*)_params;
    Graphics* graphics = params->graphics;

    // start the frame
    GUIWindow::NewFrame(*graphics->m_window);

    // set up styling and location
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Appearing);
    int width = graphics->m_window->getWidth();
    int height = graphics->m_window->getHeight();
    ImGui::SetNextWindowSize(ImVec2(300, height), ImGuiCond_Always);

    // begin the side panel
    bool barAndStuffs = true;
    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoScrollbar;
    window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
    ImGui::Begin("Bar and stuffs", &barAndStuffs, window_flags);

    // zeh stuff in the panel
    ImGui::Text("Important stuff will go here >.>");

    // block of configurable values
    ImGui::SliderFloat("Gradient Speed", &graphics->m_gradientSpeed, 0.0f,
                       0.1f);
    for (int i = 0; i < 4; i++) {
        ImGui::SliderFloat4("lol,", graphics->m_sliderQuad, 0, 42);
        ImGui::SliderFloat4("random", graphics->m_sliderQuad, 0, 42);
        if (i != 3) {
            ImGui::SliderFloat4("sliders", graphics->m_sliderQuad, 0, 42);
        }
    }

    // block of graphs (scrollable)
    window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    ImGui::BeginChildFrame(ImGui::GetID("Bar and stuffs"),
                           ImVec2(300, height - 305), window_flags);
    graphics->drawBallInterface(); 
    ImGui::EndChildFrame();
    ImGui::End();

    // end the frame
    GUIWindow::RenderFrame();
}

void Graphics::pushBall(Ball ball) {
    m_metaballs.resize(++m_numBalls);
    m_colors.resize(m_numBalls);
    m_metaballs[m_numBalls - 1] = ball;
    m_colors[m_numBalls - 1] = ImVec4(ball.color.r, ball.color.g, ball.color.b, 1.0f);
}

void Graphics::pushBall() {
    Ball ball;
    ball.size = std::rand() % 20;
    //ball.position = {(float) (std::rand() % m_width), (float) (std::rand() % m_height)};
    ball.velocity = {(float) (std::rand() % 20), (float) (std::rand() % 20)};
    ball.color = {(float) std::rand() / RAND_MAX, (float) std::rand() / RAND_MAX,
                  (float) std::rand() / RAND_MAX};
    pushBall(ball);
}

void Graphics::popBall() {
    m_metaballs.pop_back();
    m_colors.pop_back();
    m_numBalls--;
}

void Graphics::drawBallInterface() {
    //ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (int i = 0; i < m_numBalls; i++) {
        ///drawList->AddCircleFilled(ImVec2(), m_metaballs[i].size, ImColor(m_colors[i]));
        ImGui::SliderFloat("Radius", &m_metaballs[i].size, 1.0f, 20.0f, "");
        ImGui::SliderFloat2("Velocity", (float*) &m_metaballs[i].velocity, 1.0f, 20.0f, "");
        ImGui::ColorEdit3("Color", &m_colors[i].x);
        if(i < m_numBalls - 1) {
            ImGui::Separator();
        }
    }
}