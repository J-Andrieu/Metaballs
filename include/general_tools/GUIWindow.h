#ifndef GUI_WINDOW_H
#define GUI_WINDOW_H

//inherits SDL2 and OpenGL from Window
#include "Window.h"

//dear ImGUI for zeh gui :P
#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl3.h"

#include <cassert>

class GUIWindow : public Window {
public:
    GUIWindow() = delete;
    GUIWindow(const std::string& name, size_t height, size_t width, int sdlWindowFlags = Window::DefaultWindowFlags());
    ~GUIWindow();

    //Only one! >.>
    GUIWindow(const GUIWindow& other) = delete;
    GUIWindow& operator=(const GUIWindow& other) = delete;

    //some more drawing functions for abstracting gui
    void setGUIFunc(const std::function<void(void*)>& guiFunctor);
    void setGUIParams(void* params);
    void drawGUI();

    //helper functions for abstracting the draw function
    static void NewFrame(SDL_Window* window);
    static void RenderFrame();

    //doesn't override Window::handleEvent
    //allows all event types for ImGUI to process
    void handleEvent(SDL_Event& event);
private:
    //wrapper containers for holding ImGUI drawing functions
    std::function<void(void*)> m_drawGUIFunc;
    void* m_guiParams;

    //there can only be one >.>
    static bool s_exists;
};

#endif /* GUI_WINDOW_H */