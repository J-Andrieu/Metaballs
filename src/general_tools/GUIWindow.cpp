#include "GUIWindow.h"

bool GUIWindow::s_exists = false;

/** GUIWindow constructor
 *  @param name The title for the window being created
 *  @param height The desired height of the window
 *  @param width The desired width of the window
 *  @param sdlWindowFlage The window flags to be passed to SDL. Default: SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
 * 
 *  @note Initializes SDL2 for OpenGL as well as GLEW
 *  @note Initialized ImGui for SDL2 and OpenGL
 *  @note Throws a runtime error if the window cannot be created
 *  @note This doesn't use the ImGui docking branch, so there can only be one GUIWindow object
 */
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

///GUIWindow destructor
GUIWindow::~GUIWindow() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    s_exists = false;
}

/** Stores a function for drawing the gui
 *  @param guiFunctor The function that will draw the gui
 */
void GUIWindow::setGUIFunc(const std::function<void(void*)>& guiFunctor) {
    m_drawGUIFunc = guiFunctor;
}

/** Stores a void* to an object containing parameters for the draw gui function
 *  @param params The pointer to the parameters
 */
void GUIWindow::setGUIParams(void* params) {
    m_guiParams = params;
}

/** Initializes a new ImGui fram
 *  @param window The window to initialize the frame for
 */
void GUIWindow::NewFrame(SDL_Window* window) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplSDL2_NewFrame(window);
    ImGui::NewFrame();
}

///Renders the current ImGui frame
void GUIWindow::RenderFrame() {
    ImGui::Render();
    ImGuiIO& io = ImGui::GetIO();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

///Calls the stored function for drawing the GUI on the stored parameters
void GUIWindow::drawGUI() {
    m_drawGUIFunc(m_guiParams);
}

/** Handles the provided SDL event
 *  @param even The SDL_Event to pass to ImGui for handling
 * 
 *  @note If the event is an SDL_WindowEvent, this function will
 *        call Window::handleEvent on it as well
 */
void GUIWindow::handleEvent(SDL_Event& event) {
    ImGui_ImplSDL2_ProcessEvent(&event);
    if (event.type == SDL_WINDOWEVENT) {
        Window::handleEvent(event.window);
    }
}