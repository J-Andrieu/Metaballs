#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <SDL2/SDL.h>
#include <vector>
#include <variant>

/** Class used for handling events
 *  @class EventHandler
 */
class EventHandler {
public:
    //vectors for storing events from poll (if desired)
    bool storeAllEvents = true; ///< Should the events vector be populated
    std::vector<SDL_Event> events; ///< Vector to store all SDL events after polling
    bool storeWindowEvents = false; ///< Should the windowEvents vector be populated
    std::vector<SDL_WindowEvent> windowEvents; ///< Stored only window events after polling
    bool storeMouseEvents = false; ///< Should the mouseEvenets vector be populated
    std::vector<std::variant<SDL_MouseButtonEvent, SDL_MouseWheelEvent, SDL_MouseMotionEvent>> mouseEvents; ///< Stores only mouse events after polling
    bool storeKeyboardEvents = false; ///< Should the keyboardEvents vector be populated
    std::vector<SDL_KeyboardEvent> keyboardEvents; ///< Stores only keyboard events after polling

    ///Enum for all tracked keys
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
    ///Vector to store wether a specified key in EventHandler::keys is down
    bool keyDown[keys::numKeys] = {false};

    float mouseX = 0.0f; ///< Mouse x movement
    float mouseY = 0.0f; ///< Mouse y movement
    float wheelX = 0.0f; ///< Scroll wheel x movement
    float wheelY = 0.0f; ///< Scroll wheel y movement
    ///Enum for mouse buttons
    typedef enum {
        left,
        right,
        middle,
        numButtons
    } buttons;
    ///Vector to store whether a specified mouse button is down
    bool buttonDown[buttons::numButtons] = {false};

    //reset the cache
    void clearCache();

    //poll the event queue and process accordingly 
    void poll();

};

#endif /* EVENT_HANDLER_H */