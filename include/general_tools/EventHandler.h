#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <SDL2/SDL.h>
#include <vector>
#include <variant>

//Later this will actually process stuff, not just make a vector for me to search lol

class EventHandler {
public:

    //vectors for storing events from poll (if desired)
    bool storeAllEvents = true;
    std::vector<SDL_Event> events;
    bool storeWindowEvents = false;
    std::vector<SDL_WindowEvent> windowEvents;
    bool storeMouseEvents = false;
    std::vector<std::variant<SDL_MouseButtonEvent, SDL_MouseWheelEvent, SDL_MouseMotionEvent>> mouseEvents;
    bool storeKeyboardEvents = false;
    std::vector<SDL_KeyboardEvent> keyboardEvents;

    //"cache" for storing all dem ~important~ valse for easy access
    typedef enum {
        A = 10,
        B,
        C,
        D,
        E,
        F,
        G,
        H,
        I,
        J,
        K,
        L,
        M,
        N,
        O,
        P,
        Q,
        R,
        S,
        T,
        U,
        V,
        W,
        X,
        Y,
        Z,
        SHIFT,
        CTRL,
        ALT,
        ENTER,
        SPACE,
        TAB,
        ESC,
        DOWN,
        LEFT,
        UP,
        RIGHT,
        dash,
        equals,
        numKeys
    } keys;
    bool keyDown[keys::numKeys] = {false};

    float mouseX = 0.0f;
    float mouseY = 0.0f;
    float wheelX = 0.0f;
    float wheelY = 0.0f;
    typedef enum {
        left,
        right,
        middle,
        numButtons
    } buttons;
    bool buttonDown[buttons::numButtons] = {false};

    //reset the cache
    void clearCache();

    //poll the event key and process accordingly 
    void poll();

};

#endif /* EVENT_HANDLER_H */