#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>

#define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#include<GL/glew.h>

#include <string>
#include <stdexcept>
#include <functional>

/** Window class
 *  @class Window
 * 
 *  @note Requires OpenGL 4.3+ and SDL2
 *  @note Cannot be copied, move semantics only
 */
class Window {
public:
    Window() = delete; ///< No default constructor, would be a bit useless
    Window(const std::string& name, size_t height, size_t width, int sdlWindowFlags = DefaultWindowFlags());
    ~Window();

    //No copying allowed! >:( (moving is probs ok tho)
    //Window(Window&& other);
    //Window& operator=(Window&& other);
    Window(const Window& other) = delete;
    Window& operator=(const Window& other) = delete;

    void swap();
    void focus();
    void hide();
    void show();
    void rename(const std::string& name);
    //only handles (some) window events >.>
    void handleEvent(SDL_WindowEvent& event);

    SDL_Window* getWindow();
    SDL_GLContext& getContext();

    void resize(size_t height, size_t width);
    size_t getHeight() const;
    size_t getWidth() const;

    bool isShown() const;
    bool isHidden() const;
    bool isMinimized() const;

    //functions to make drawing look a tad cleaner
    void setDrawFunc(const std::function<void(void*)>& drawFunctor);
    void setDrawParams(void* params);
    void draw();

    ///Returns the default window flags: SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    constexpr static inline int DefaultWindowFlags() {
        return SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE;
    }

    //directly pass window to SDL functions
    operator SDL_Window*();

protected:
    //SDL window and context are only protected so derived classes can use them
    SDL_Window* m_window;
    SDL_GLContext m_context;

private:
    //Basic window information
    std::string m_name;
    size_t m_height;
    size_t m_width;

    //Window visibility tracking
    bool m_shown;
    bool m_minimized;
    bool m_hidden;

    //Stored function and params for drawing into the window
    std::function<void(void*)> m_drawFunc;
    void* m_drawParams;

    //Static variables for tracking global window status
    static int s_windowCount;
    static bool s_glewInitialized;
};

#endif /* WINDOW_H */