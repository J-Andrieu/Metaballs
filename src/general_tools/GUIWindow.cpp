#include "GUIWindow.h"

bool GUIWindow::s_exists = false;

GUIWindow::GUIWindow(const std::string &name, size_t height, size_t width, int sdlWindowFlags) : 
    Window(name, height, width, sdlWindowFlags), m_guiParams(nullptr) {

    //If a GUIWindow already exists, then there's been a mistake
    assert(s_exists == false);
    s_exists = true;

    //initialize ImGUI
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    //initialize ImGUI OpenGL
    ImGui_ImplOpenGL3_Init("#version 430");

    //initialize ImGUI SDL2/OpenGL, and attatch it to the window
    ImGui_ImplSDL2_InitForOpenGL(m_window, m_context);
}

GUIWindow::~GUIWindow() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    s_exists = false;
}

void GUIWindow::setGUIFunc(const std::function<void(void*)>& guiFunctor) {
    m_drawGUIFunc = guiFunctor;
}

void GUIWindow::setGUIParams(void* params) {
    m_guiParams = params;
}

void GUIWindow::NewFrame(SDL_Window* window) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
}

void GUIWindow::RenderFrame() {
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void GUIWindow::drawGUI() {
    m_drawGUIFunc(m_guiParams);
}

void GUIWindow::handleEvent(SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_WINDOWEVENT) {
        Window::handleEvent(event.window);
    }
}