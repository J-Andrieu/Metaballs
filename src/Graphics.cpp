GLfloat Graphics::s_quadVertexBufferData* = {-1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, -1.0f};

Graphics::Graphics(int height, int width) {
    m_sizeChanged = true;
    m_window = GUIWindow("Metaballs", height, width, Window::DefaultWindowFlags() | SDL_WINDOW_UTILITY);
    SDL_SetWindowMinimumSize(m_window, 700, 400);
    SDL_GL_MakeCurrent(m_window, m_window.getContext());
    ImGui::StyleColorsDark();

    m_gradientSpeed = 0.01f;
    m_window.setDrawFunc(renderGraphics);
    m_graphicsParams = {m_defaultCompute, m_tex2ScreenRender, &gradient_speed};
    m_window.setDrawParams((void*) &m_graphicsParams);
    m_window.setGUIFunc(drawMenu);
    m_sliderQuad[0] = 42;
    m_sliderQuad[1] = 69;
    m_sliderQuad[2] = 420;
    m_sliderQuad[3] = 9001;
    m_guiParams = {&m_window, m_sliderQuad, &m_gradientSpeed};
    m_window.setGUIParams((void*) &m_guiParams);

    m_defaultCompute = Shader::ComputeProgram(
        Shader::shader(std::ifstream("shaders/standard_render.comp"), GL_COMPUTE_SHADER));
    m_defaultCompute = Shader::GraphicsProgram({
        Shader::shader(std::ifstream("shaders/pass_through.vert"), GL_VERTEX_SHADER),
        Shader::shader(std::ifstream("shaders/tex2screen.frag"), GL_FRAGMENT_SHADER)});

    m_renderUniformSize = glGetUniformLocation(m_tex2ScreenRender, "tex_size");
    if (m_renderUniformSize == INVALID_UNIFORM_LOCATION) {
        throw std::runtime_error(std::string("Uniform time could not be found"));
    }
    m_computeUniformTime = glGetUniformLocation(m_defaultCompute, "time");
    if (m_computeUniformTime == INVALID_UNIFORM_LOCATION) {
        throw std::runtime_error(std::string("Uniform time could not be found"));
    }
    
    glGenVertexArrays(1, &m_quadVAO);
    glBindVertexArray(m_quadVAO);
    GLuint quadVBO;
    glGenBuffers(1, &quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(s_quadVertexBufferData), s_quadVertexBufferData, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);    
    glDeleteBuffers(1, &quadVBO);

    m_texOut = 0;
    m_timeOffset = 0;
}

Graphics::~Graphics() {
    glDeleteTextures(1, &m_texOut);
    glDeleteVertexArrays(1, &m_quadVAO);
}

GUIWindow& Graphics::Window() {
    return m_window;
}

size_t Graphics::Height() {
    return m_height;
}

size_t Graphics::Width() {
    return m_width;
}

void Graphics::updateDimensions() {
    SDL_GetWindowSize(m_window, &m_width, &m_height);
    m_sizeChanged = true;
}

void m_drawFunc(void* _params) {
  int width = m_window.getWidth();
  int height = m_window.getHeight();
  //this is made of memory leaks, should be stored in object 
  //when finalized for proper resource freeing
  if (m_sizeChanged || m_texOut == 0) {
    if (tex_out != 0) {
      glDeleteTextures(1, &m_texOut);
    }
    glGenTextures(1, &m_texOut);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texOut);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width - 300, height, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(0, m_texOut, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    m_height = height;
    m_width = width;
    m_sizeChanged = false;
  }

  //empty the window
  glClearColor(0, 123, 225, 225);
  glClear(GL_COLOR_BUFFER_BIT);
  
  //compute the gradient
  {
    m_defaultCompute.setActive();
    m_timeOffset += m_gradientSpeed;
    float i = std::sin(m_timeOffset);
    i += 1.0f;
    i /= 2.0f;
    glUniform1f(m_computeUniformTime, i);
    glDispatchCompute((GLuint) width - 300, (GLuint) height, 1);
  }
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  //render the texture
  {
    glClear(GL_COLOR_BUFFER_BIT);
    glViewport(300, 0, width - 300, height);
    m_tex2ScreenRender.setActiveProgram();
    glUniform2f(renderUniformSize, (float) width, (float) height);
    glBindVertexArray(m_quadVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_texOut);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  }
}

//draw the test panel
void m_drawGUIFunc(void* _params) {
  //start the frame
  GUIWindow::NewFrame(m_window);

  //set up styling and location
  ImGuiStyle& style = ImGui::GetStyle();
  style.WindowRounding = 0.0f;
  ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Appearing);
  int width = WindowWidth;
  int height = WindowHeight;
  ImGui::SetNextWindowSize(ImVec2(300, height), ImGuiCond_Always);

  //begin the side panel
  bool barAndStuffs = true;
  ImGuiWindowFlags window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoMove;
  window_flags |= ImGuiWindowFlags_NoScrollbar;
  window_flags |= ImGuiWindowFlags_NoScrollWithMouse;
  ImGui::Begin("Bar and stuffs", &barAndStuffs, window_flags);
  
  //zeh stuff in the panel
  ImGui::Text("Important stuff will go here >.>");

  //block of configurable values
  ImGui::SliderFloat("Gradient Speed", m_gradientSpeed, 0.0f, 0.1f);
  for (int i = 0; i < 4; i++) {
    ImGui::SliderFloat4("lol,", m_sliderQuad, 0, 42);
    ImGui::SliderFloat4("random", m_sliderQuad, 0, 42);
    if (i != 3) { 
      ImGui::SliderFloat4("sliders", m_sliderQuad, 0, 42);
    }
  }

  //block of graphs (scrollable)
  window_flags = 0;
  window_flags |= ImGuiWindowFlags_NoCollapse;
  window_flags |= ImGuiWindowFlags_NoResize;
  window_flags |= ImGuiWindowFlags_NoTitleBar;
  window_flags |= ImGuiWindowFlags_NoMove;
  ImGui::BeginChildFrame(ImGui::GetID("Bar and stuffs"), ImVec2(300, height - 305), window_flags);
  for (int i = 0; i < 150; i++) {
    ImGui::Text("Lol, this is where the graphs would go");
  }
  ImGui::Text(">.>");
  ImGui::EndChildFrame();

  ImGui::End();

  //end the frame
  GUIWindow::RenderFrame();
}