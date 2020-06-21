#include "Graphics.h"

GLfloat Graphics::s_quadVertexBufferData[8] = {-1.0f, 1.0f, -1.0f, -1.0f,
                                               1.0f,  1.0f, 1.0f,  -1.0f};

Graphics::Graphics(int height, int width)
    : m_height(height),
      m_width(m_width),
      m_texOut(0),
      m_timeOffset(0),
      m_sizeChanged(true) {
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
    m_defaultCompute = new Shader::ComputeProgram(computeShader);
    m_defaultCompute->build();
    computeFS.close();

    // attach uniform variables to shaders
    m_renderUniformSize = glGetUniformLocation(*m_tex2ScreenRender, "tex_size");
    if (m_renderUniformSize == INVALID_UNIFORM_LOCATION) {
        throw std::runtime_error(
            std::string("Uniform time could not be found"));
    }
    m_computeUniformTime = glGetUniformLocation(*m_defaultCompute, "time");
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
    m_params = (drawParams){m_window,
                            &m_sizeChanged,
                            &m_texOut,
                            &m_height,
                            &m_width,
                            m_defaultCompute,
                            m_tex2ScreenRender,
                            &m_timeOffset,
                            &m_gradientSpeed,
                            &m_computeUniformTime,
                            &m_renderUniformSize,
                            &m_quadVAO,
                            m_sliderQuad};
    m_window->setDrawParams((void*)&m_params);
    m_window->setGUIParams((void*)&m_params);
}

Graphics::~Graphics() {
    glDeleteTextures(1, &m_texOut);
    glDeleteVertexArrays(1, &m_quadVAO);
    delete m_window;
    delete m_tex2ScreenRender;
    delete m_defaultCompute;
}

GUIWindow* Graphics::Window() { return m_window; }

int Graphics::Height() { return m_height; }

int Graphics::Width() { return m_width; }

void Graphics::updateDimensions() {
    SDL_GetWindowSize(*m_window, &m_width, &m_height);
    m_sizeChanged = true;
}

void Graphics::m_drawFunc(void* _params) {
    drawParams* params = (drawParams*)_params;

    int width = params->window->getWidth();
    int height = params->window->getHeight();
    // this is made of memory leaks, should be stored in object
    // when finalized for proper resource freeing
    if (*params->sizeChanged || *params->texOut == 0) {
        if (*params->texOut != 0) {
            glDeleteTextures(1, params->texOut);
        }
        glGenTextures(1, params->texOut);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, *params->texOut);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width - 300, height, 0,
                     GL_RGBA, GL_FLOAT, NULL);
        glBindImageTexture(0, *params->texOut, 0, GL_FALSE, 0, GL_WRITE_ONLY,
                           GL_RGBA32F);
        *params->height = height;
        *params->width = width;
        *params->sizeChanged = false;
    }

    // empty the window
    glClearColor(0, 123, 225, 225);
    glClear(GL_COLOR_BUFFER_BIT);

    // compute the gradient
    {
        params->computeProg->setActiveProgram();
        *params->timeOffset += *params->gradientSpeed;
        float i = std::sin(*params->timeOffset);
        i += 1.0f;
        i /= 2.0f;
        glUniform1f(*params->uniformTime, i);
        glDispatchCompute((GLuint)width - 300, (GLuint)height, 1);
    }
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // render the texture
    {
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(300, 0, width - 300, height);
        params->tex2ScreenProg->setActiveProgram();
        glUniform2f(*params->uniformSize, (float)width, (float)height);
        glBindVertexArray(*params->quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, *params->texOut);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }
}

// draw the test panel
void Graphics::m_drawGUIFunc(void* _params) {
    drawParams* params = (drawParams*)_params;

    // start the frame
    GUIWindow::NewFrame(*params->window);

    // set up styling and location
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Appearing);
    int width = params->window->getWidth();
    int height = params->window->getHeight();
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
    ImGui::SliderFloat("Gradient Speed", params->gradientSpeed, 0.0f, 0.1f);
    for (int i = 0; i < 4; i++) {
        ImGui::SliderFloat4("lol,", params->sliderQuad, 0, 42);
        ImGui::SliderFloat4("random", params->sliderQuad, 0, 42);
        if (i != 3) {
            ImGui::SliderFloat4("sliders", params->sliderQuad, 0, 42);
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
    for (int i = 0; i < 150; i++) {
        ImGui::Text("Lol, this is where the graphs would go");
    }
    ImGui::Text(">.>");
    ImGui::EndChildFrame();

    ImGui::End();

    // end the frame
    GUIWindow::RenderFrame();
}