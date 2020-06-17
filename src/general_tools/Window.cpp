#include <Window.h>

int Window::s_windowCount = 0;
bool Window::s_glewInitialized = false;

Window::Window(const std::string& name, size_t height, size_t width, int sdlWindowFlags) :
    m_window(nullptr), m_drawParams(nullptr) {

    if (s_windowCount == 0) {
        //initialize SDL
        if(SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::string msg = std::string("SDL failed to initialize: ") + SDL_GetError();
            throw(std::runtime_error(msg));
        }

        //get SDL ready for OpenGL
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 5);
        SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 5);
    }

    m_shown = sdlWindowFlags & SDL_WINDOW_SHOWN;
    m_hidden = sdlWindowFlags & SDL_WINDOW_HIDDEN;
    m_minimized = sdlWindowFlags & SDL_WINDOW_MINIMIZED;

    //time to make a friggen window!
    //but are we full screen?
    SDL_DisplayMode display;
    SDL_GetDesktopDisplayMode(0, &display);
    m_height = height ? height : display.h;
    m_width = width ? width : display.w;
    m_name = name;

    m_window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                m_width, m_height,
                                SDL_WINDOW_OPENGL | sdlWindowFlags);
    if (!m_window) {
        std::string msg = std::string("Failed to create window: ") + SDL_GetError();
        throw(std::runtime_error(msg));
    }

    //create da context :P
    m_context = SDL_GL_CreateContext(m_window);
    if(!m_context) {
        std::string msg = std::string("OpenGL context not created: ") + SDL_GetError();
        throw(std::runtime_error(msg));
    }

    //use vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        std::string msg = std::string("Unable to use VSync: ") + SDL_GetError();
        throw(std::runtime_error(msg));
    }

    if (!s_glewInitialized) {
        s_glewInitialized = true;
        GLenum err = glewInit();

        if (err != GLEW_OK) {
            std::string msg = std::string("GLEW Error: ") + std::string((char*) glewGetErrorString(err));
		    throw(std::runtime_error(msg));
        }
    }
}

Window::~Window() {
    SDL_DestroyWindow(m_window);
    SDL_GL_DeleteContext(m_context);
    s_windowCount--;
    if (s_windowCount == 0) {
        SDL_Quit();
    }
}

void Window::focus() {
    if (!m_shown) {
        SDL_ShowWindow(m_window);
    }
    SDL_RaiseWindow(m_window);
}

void Window::swap() {
    SDL_GL_SwapWindow(m_window);
}

void Window::hide() {
    m_hidden = true;
    m_shown = false;
    SDL_HideWindow(m_window);
}

void Window::show() {
    m_hidden = false;
    m_shown = true;
    SDL_ShowWindow(m_window);
}

void Window::rename(const std::string& name) {
    m_name = name;
    SDL_SetWindowTitle(m_window, name.c_str());
}

void Window::handleEvent(SDL_WindowEvent& event) {
    //if it is indeed a window event meant for this window
    if (event.windowID == SDL_GetWindowID(m_window)) {
        switch (event.event) {
            case SDL_WINDOWEVENT_SHOWN:
                show();
                break;

            case SDL_WINDOWEVENT_HIDDEN:
                hide();
                break;

            case SDL_WINDOWEVENT_SIZE_CHANGED:
                m_width = event.data1;
                m_height = event.data2;
                break;

            case SDL_WINDOWEVENT_CLOSE:
                hide();
                break;

            //all those other events that should be handeled explicitly :P
            default:
                break;
        }
    }
}

SDL_Window* Window::getWindow() {
    return m_window;
}

SDL_GLContext& Window::getContext() {
    return m_context;
}

void Window::resize(size_t height, size_t width) {
    SDL_SetWindowSize(m_window, height, width);
    m_height = height;
    m_width = width;
}

size_t Window::getHeight() const {
    return m_height;
}

size_t Window::getWidth() const {
    return m_width;
}

bool Window::isShown() const {
    return m_shown;
}

bool Window::isHidden() const {
    return m_hidden;
}

bool Window::isMinimized() const {
    return m_minimized;
}

Window::operator SDL_Window*() {
    return m_window;
}

void Window::setDrawFunc(const std::function<void(void*)>& drawFunctor) {
    m_drawFunc = drawFunctor;
}

void Window::setDrawParams(void* params) {
    m_drawParams = params;
}

void Window::draw() {
    m_drawFunc(m_drawParams);
}