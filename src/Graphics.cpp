#include "Graphics.h"

#define GRAPHICS_USE_SPIRV 1

GLfloat Graphics::s_quadVertexBufferData[8] = {-1.0f, 1.0f, -1.0f, -1.0f,
                                               1.0f,  1.0f, 1.0f,  -1.0f};

Graphics::Graphics(int height, int width)
    : m_height(height),
      m_width(width),
      m_menuWidth(400),
      m_texOut(0),
      m_timeOffset(0),
      m_sizeChanged(true),
      m_wigglyMovement(false),
      m_numBalls(0),
      m_metaballs(0),
      m_colors(0),
      m_metaballsSSBO(0),
      m_genSSBO(true),
      m_ssboBindingIndex(1),
      m_currentShader(Default),
      m_shaderName("Circles"),
      m_cellsThresh(1),
      m_metaBGRadiusMult(100),
      m_metaRORadiusMult(400),
      m_metaRGBRadiusMult(1000),
      m_metaParamRadiusMult(100),
      m_metaParamRed(true),
      m_metaParamGreen(false),
      m_metaParamBlue(false),
      m_metaParamHigh(false),
      m_ssboData(NULL) {
    // initialize the window
    m_window = new GUIWindow("Metaballs", height, width,
                             Window::DefaultWindowFlags() | SDL_WINDOW_UTILITY);
    SDL_SetWindowMinimumSize(*m_window, 400 + m_menuWidth, 400);
    SDL_GL_MakeCurrent(*m_window, m_window->getContext());
    ImGui::StyleColorsDark();

    // attach window draw functions
    m_window->setDrawFunc(m_drawFunc);
    m_window->setGUIFunc(m_drawGUIFunc);

    // prepare the compute shaders
    // Circles, Cells, Meta_BlueGreen, Meta_RegOrange, Meta_RGB
#if GRAPHICS_USE_SPIRV
    std::vector<std::string> shaderFiles = {
        "circles.comp.spv", "cells.comp.spv",    "meta_bg.comp.spv",
        "meta_ro.comp.spv", "meta_rgb.comp.spv", "meta_params.comp.spv"};
#else
    std::vector<std::string> shaderFiles = {
        "circles.comp", "cells.comp",    "meta_bg.comp",
        "meta_ro.comp", "meta_rgb.comp", "meta_params.comp"};
#endif
    m_computeShaders.resize(NumShaderTypes);
    for (int i = 0; i < NumShaderTypes; i++) {
        try {
            std::ifstream computeFS(std::string("shaders/") + shaderFiles[i]);
            Shader::shader computeShader(computeFS, GL_COMPUTE_SHADER, GRAPHICS_USE_SPIRV);
#if GRAPHICS_USE_SPIRV
            computeShader.specialize();
#else
            computeShader.compile();
#endif

            m_computeShaders[i] = new Shader::ComputeProgram(computeShader);
            m_computeShaders[i]->build();
        } catch (std::exception& e) {
            printf("Error occurred while compiling %s\n",
                   shaderFiles[i].c_str());
            printf("%s", e.what());
            exit(-1);
        }
    }

    m_cellsUniform_thresh =
        glGetUniformLocation(*m_computeShaders[Cells], "sumThresh");
    m_metaBGUniform_radiusMult =
        glGetUniformLocation(*m_computeShaders[Meta_BlueGreen], "radiusMult");
    m_metaROUniform_radiusMult =
        glGetUniformLocation(*m_computeShaders[Meta_RedOrange], "radiusMult");
    m_metaRGBUniform_radiusMult =
        glGetUniformLocation(*m_computeShaders[Meta_RGB], "radiusMult");
    m_metaParamUniform_radiusMult =
        glGetUniformLocation(*m_computeShaders[Meta_Params], "radiusMult");
    m_metaParamUniform_red =
        glGetUniformLocation(*m_computeShaders[Meta_Params], "red");
    m_metaParamUniform_green =
        glGetUniformLocation(*m_computeShaders[Meta_Params], "green");
    m_metaParamUniform_blue =
        glGetUniformLocation(*m_computeShaders[Meta_Params], "blue");
    m_metaParamUniform_high =
        glGetUniformLocation(*m_computeShaders[Meta_Params], "high");

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
    bindSSBO();
}

Graphics::~Graphics() {
    glDeleteTextures(1, &m_texOut);
    glDeleteVertexArrays(1, &m_quadVAO);
    glDeleteBuffers(1, &m_metaballsSSBO);
    delete m_window;
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
        if (!m_ssboData) {
            updateMetaballs_RandomPath(m_metaballs, 2.0f, m_width - m_menuWidth,
                                       m_height);
        } else {
            updateMetaballs_RandomPath(m_metaballs.size(), m_ssboData, 2.0f,
                                       m_width - m_menuWidth, m_height);
        }
    } else {
        if (!m_ssboData) {
            updateMetaballs_StraightPath(m_metaballs, m_width - m_menuWidth,
                                         m_height);
        } else {
            updateMetaballs_StraightPath(m_metaballs.size(), m_ssboData,
                                         m_width - m_menuWidth, m_height);
        }
    }

    if (!m_ssboData) {
        for (int i = 0; i < m_numBalls; i++) {
            m_metaballs[i].color = {m_colors[i].x, m_colors[i].y,
                                    m_colors[i].z};
        }
    } else {
        for (int i = 0; i < m_numBalls; i++) {
            m_ssboData[i].color = {m_colors[i].x, m_colors[i].y, m_colors[i].z};
        }
    }
}

void Graphics::m_drawFunc(void* _params) {
    drawParams* params = (drawParams*)_params;
    Graphics* graphics = params->graphics;

    glClear(GL_COLOR_BUFFER_BIT);
    // start the frame for both render functions
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
        graphics->m_computeShaders[graphics->m_currentShader]
            ->setActiveProgram();
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

        ImGui::Image((ImTextureID)(intptr_t)graphics->m_texOut,
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

    // block of configurable values
    ImGui::Text(" ");
    ImGui::Text(" Currently selected shader: ");
    ImGui::SameLine();
    if (ImGui::CollapsingHeader(graphics->m_shaderName.c_str())) {
        if (ImGui::Button("Circles")) {
            graphics->m_currentShader = Circles;
            graphics->m_shaderName = "Circles";
        }
        if (ImGui::Button("Cells")) {
            graphics->m_currentShader = Cells;
            graphics->m_shaderName = "Cells";
        }
        if (ImGui::Button("Blue/Green Metaballs")) {
            graphics->m_currentShader = Meta_BlueGreen;
            graphics->m_shaderName = "Blue/Green Metaballs";
        }
        if (ImGui::Button("Red/Orange Metaballs")) {
            graphics->m_currentShader = Meta_RedOrange;
            graphics->m_shaderName = "Red/Orange Metaballs";
        }
        if (ImGui::Button("RGB Metaballs")) {
            graphics->m_currentShader = Meta_RGB;
            graphics->m_shaderName = "RGB Metaballs";
        }
        if (ImGui::Button("Parameterized Metaballs")) {
            graphics->m_currentShader = Meta_Params;
            graphics->m_shaderName = "Parameterized Metaballs";
        }
    }

    switch (graphics->m_currentShader) {
        case Cells:
            ImGui::SliderFloat("Threshold", &graphics->m_cellsThresh, 0.1, 5,
                               "");
            glUniform1f(graphics->m_cellsUniform_thresh,
                        1.0f / graphics->m_cellsThresh);
            break;

        case Meta_BlueGreen:
            ImGui::SliderFloat("Radius Multiplier",
                               &graphics->m_metaBGRadiusMult, 0.01, 1000, "");
            glUniform1f(graphics->m_metaBGUniform_radiusMult,
                        graphics->m_metaBGRadiusMult);
            break;

        case Meta_RedOrange:
            ImGui::SliderFloat("Radius Multiplier",
                               &graphics->m_metaRORadiusMult, 0.01, 1000, "");
            glUniform1f(graphics->m_metaROUniform_radiusMult,
                        graphics->m_metaRORadiusMult);
            break;

        case Meta_RGB:
            ImGui::SliderFloat("Radius Multiplier",
                               &graphics->m_metaRGBRadiusMult, 0.01, 2000, "");
            glUniform1f(graphics->m_metaRGBUniform_radiusMult,
                        graphics->m_metaRGBRadiusMult);
            break;

        case Meta_Params:
            ImGui::SliderFloat("Radius Multiplier",
                               &graphics->m_metaParamRadiusMult, 0.01, 1000,
                               "");
            ImGui::Text(" Affected channels: ");
            ImGui::SameLine();
            ImGui::Checkbox("Red", &graphics->m_metaParamRed);
            ImGui::SameLine();
            ImGui::Checkbox("Green", &graphics->m_metaParamGreen);
            ImGui::SameLine();
            ImGui::Checkbox("Blue", &graphics->m_metaParamBlue);
            ImGui::Checkbox("Default values to high",
                            &graphics->m_metaParamHigh);

            glUniform1f(graphics->m_metaParamUniform_radiusMult,
                        graphics->m_metaParamRadiusMult);
            glUniform1i(graphics->m_metaParamUniform_red,
                        graphics->m_metaParamRed);
            glUniform1i(graphics->m_metaParamUniform_green,
                        graphics->m_metaParamGreen);
            glUniform1i(graphics->m_metaParamUniform_blue,
                        graphics->m_metaParamBlue);
            glUniform1i(graphics->m_metaParamUniform_high,
                        graphics->m_metaParamHigh);
            break;
    }

    if (ImGui::Button("Add Ball")) {
        graphics->pushBall();
        graphics->bindSSBO();
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Ball")) {
        graphics->popBall();
        graphics->bindSSBO();
    }

    // block of graphs (scrollable)
    window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoTitleBar;
    window_flags |= ImGuiWindowFlags_NoMove;
    ImGui::BeginChildFrame(
        ImGui::GetID("Bar and stuffs"),
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

    m_genSSBO = true;
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

    m_genSSBO = true;
}

void Graphics::drawBallInterface() {
    for (int i = 0; i < m_numBalls; i++) {
        ImGui::PushID(i + 42);

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
    if (m_ssboData) {
        for (int i = 0; i < m_numBalls; i++) {
            m_metaballs[i] = m_ssboData[i];
        }
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
        m_ssboData = NULL;
    }

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
    m_ssboData = (Ball*)glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, sizeof(uint), sizeof(Ball) * m_numBalls,
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    for (int i = 0; i < m_numBalls; i++) {
        m_ssboData[i] = m_metaballs[i];
    }
}