#include "Graphics.h"

GLfloat Graphics::s_quadVertexBufferData[8] = {-1.0f, 1.0f, -1.0f, -1.0f,
                                               1.0f,  1.0f, 1.0f,  -1.0f};

Graphics::Graphics(int height, int width)
    : m_height(height),
      m_width(width),
      m_menuWidth(350),
      m_texOut(0),
      m_timeOffset(0),
      m_sizeChanged(true),
      m_wigglyMovement(false),
      m_numBalls(0),
      m_metaballs(0),
      m_colors(0),
      m_metaballsSSBO(0),
      m_genSSBO(true),
      m_ssboBindingIndex(1) {
    // initialize the window
    m_window = new GUIWindow("Metaballs", height, width,
                             Window::DefaultWindowFlags() | SDL_WINDOW_UTILITY);
    SDL_SetWindowMinimumSize(*m_window, 400 + m_menuWidth, 400);
    SDL_GL_MakeCurrent(*m_window, m_window->getContext());
    ImGui::StyleColorsDark();

    // attach window draw functions
    m_window->setDrawFunc(m_drawFunc);
    m_window->setGUIFunc(m_drawGUIFunc);

    // prepare the compute shader
    // std::ifstream computeFS("shaders/standard_render.comp");
    std::ifstream computeFS("shaders/circles.comp");
    Shader::shader computeShader(computeFS, GL_COMPUTE_SHADER);
    computeShader.compile();
    m_computeShaders.resize(NumShaderTypes);
    m_computeShaders[Default] = new Shader::ComputeProgram(computeShader);
    m_computeShaders[Default]->build();
    computeFS.close();

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

    for (int i = 0; i < 5; i++) {
        pushBall();
    }
}

Graphics::~Graphics() {
    glDeleteTextures(1, &m_texOut);
    glDeleteVertexArrays(1, &m_quadVAO);
    glDeleteBuffers(1, &m_metaballsSSBO);
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

void Graphics::update() {
    if (m_wigglyMovement) {
        updateMetaballs_RandomPath(m_metaballs, 2.0f, m_width - m_menuWidth,
                                   m_height);
    } else {
        updateMetaballs_StraightPath(m_metaballs, m_width - m_menuWidth,
                                     m_height);
    }

    for (int i = 0; i < m_numBalls; i++) {
        m_metaballs[i].color = {m_colors[i].x, m_colors[i].y, m_colors[i].z};
    }
}

void Graphics::m_drawFunc(void* _params) {
    drawParams* params = (drawParams*)_params;
    Graphics* graphics = params->graphics;

    glClear(GL_COLOR_BUFFER_BIT);
    //start the frame for both render functions
    GUIWindow::NewFrame(*graphics->m_window);

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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
                     width - graphics->m_menuWidth, height, 0, GL_RGBA,
                     GL_FLOAT, NULL);
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
        graphics->bindSSBO();
        graphics->m_computeShaders[Default]->setActiveProgram();
        glDispatchCompute((GLuint)width - graphics->m_menuWidth, (GLuint)height,
                          1);
    }
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // render the texture
    {
        // set up styling and location
        ImGuiStyle& style = ImGui::GetStyle();
        style.WindowRounding = 0.0f;
        style.WindowPadding = ImVec2(0.0f, 0.0f);
        style.WindowBorderSize = 3.0f;
        style.ScrollbarSize = 5.0f;
        ImGui::SetNextWindowPos(ImVec2(graphics->m_menuWidth, 0),
                                ImGuiCond_Appearing);
        int width = graphics->m_window->getWidth();
        int height = graphics->m_window->getHeight();
        ImGui::SetNextWindowSize(ImVec2(width - graphics->m_menuWidth, height),
                                 ImGuiCond_Always);

        // begin the viewport
        bool viewport = true;
        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_NoCollapse;
        window_flags |= ImGuiWindowFlags_NoResize;
        window_flags |= ImGuiWindowFlags_NoTitleBar;
        window_flags |= ImGuiWindowFlags_NoMove;
        window_flags |= ImGuiWindowFlags_NoScrollbar;
        window_flags |= ImGuiWindowFlags_NoScrollWithMouse;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::Begin("Viewport", &viewport, window_flags);
        
        ImGui::Image((ImTextureID) (intptr_t) graphics->m_texOut,
                     ImVec2(width - graphics->m_menuWidth, height));
        
        ImGui::End();
        ImGui::PopStyleVar();
    }
}

// draw the test panel
void Graphics::m_drawGUIFunc(void* _params) {
    drawParams* params = (drawParams*)_params;
    Graphics* graphics = params->graphics;

    // set location
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Appearing);
    int width = graphics->m_window->getWidth();
    int height = graphics->m_window->getHeight();
    ImGui::SetNextWindowSize(ImVec2(graphics->m_menuWidth, height),
                             ImGuiCond_Always);

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
    

    // block of graphs (scrollable)
    window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    ImGui::BeginChildFrame(ImGui::GetID("Bar and stuffs"),
                           ImVec2(graphics->m_menuWidth, height - (ImGui::GetCursorPosY() + 5)),
                           window_flags);
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
    m_colors[m_numBalls - 1] =
        ImVec4(ball.color.r, ball.color.g, ball.color.b, 1.0f);
}

void Graphics::pushBall() {
    Ball ball;
    ball.size = std::rand() % 100;
    // for some reason this allocation results in unknown address w/ address
    // sanitizer ball.position = {(float)(std::rand() % m_width),
    //                 (float)(std::rand() % m_height)};
    ball.position = {(float)(std::rand() % 1000), (float)(std::rand() % 1000)};
    ball.velocity = {(float)(std::rand() % 10) - 5,
                     (float)(std::rand() % 10) - 5};
    ball.color = {(float)std::rand() / RAND_MAX, (float)std::rand() / RAND_MAX,
                  (float)std::rand() / RAND_MAX};
    pushBall(ball);
}

void Graphics::popBall() {
    m_metaballs.pop_back();
    m_colors.pop_back();
    m_numBalls--;
}

void Graphics::drawBallInterface() {
    // ImDrawList* drawList = ImGui::GetWindowDrawList();
    for (int i = 0; i < m_numBalls; i++) {
        ImGui::PushID(i + 42);
        /// drawList->AddCircleFilled(ImVec2(), m_metaballs[i].size,
        /// ImColor(m_colors[i]));
        ImGui::SliderFloat("Radius", &m_metaballs[i].size, 1.0f, 100.0f, "");
        ImGui::SliderFloat2("Velocity", (float*)&m_metaballs[i].velocity, -5.0f,
                            5.0f, "");
        ImGui::SliderFloat("Pos X", (float*)&m_metaballs[i].position.x, 0.0f,
                           m_width - m_menuWidth, "");
        ImGui::SliderFloat("Pos Y", (float*)&m_metaballs[i].position.y, 0.0f,
                           m_height, "");
        ImGui::ColorEdit3("Color", &m_colors[i].x);
        if (i < m_numBalls - 1) {
            ImGui::Separator();
        }
        ImGui::PopID();
    }
}

void Graphics::bindSSBO() {
    if (m_genSSBO) {
        m_genSSBO = false;
        if (m_metaballsSSBO) {
            glDeleteBuffers(1, &m_metaballsSSBO);
        }
        glGenBuffers(1, &m_metaballsSSBO);
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_metaballsSSBO);
        glBufferData(GL_SHADER_STORAGE_BUFFER,
                     sizeof(size_t) + sizeof(Ball) * m_metaballs.size(), NULL,
                     GL_DYNAMIC_COPY);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_ssboBindingIndex,
                         m_metaballsSSBO);
        GLuint index = glGetProgramResourceIndex(*m_computeShaders[Default],
                                                 GL_SHADER_STORAGE_BUFFER,
                                                 "metaball_data");
        glShaderStorageBlockBinding(*m_computeShaders[Default], index,
                                    m_ssboBindingIndex);
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_metaballsSSBO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_ssboBindingIndex,
                     m_metaballsSSBO);
    uint* lenptr = (uint*)glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 0, sizeof(uint),
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    *lenptr = (uint)m_numBalls;
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    Ball* ballptr = (Ball*)glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, sizeof(uint), sizeof(Ball) * m_numBalls,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    for (int i = 0; i < m_numBalls; i++) {
        ballptr[i] = m_metaballs[i];
    }
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}