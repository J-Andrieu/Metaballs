#ifndef EVENT_HANDLER_H
#define EVENT_HANDLER_H

#include <SDL2/SDL.h>
#include <vector>
#include <variant>

//Later this will actually process stuff, not just make a vector for me to search lol

namespace EventHandler {

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
    void clearCache() {
        events.clear();
        windowEvents.clear();
        mouseEvents.clear();
        keyboardEvents.clear();
        mouseX = 0;
        mouseY = 0;
        wheelX = 0;
        wheelY = 0;

        for (int i = 0; i < keys::numKeys; i++) {
            keyDown[i] = false;
        }
        
        mouseX = mouseY = wheelX = wheelY = 0.0f;

        for (int i = 0; i < buttons::numButtons; i++) {
            buttonDown[i] = false;
        }
    }

    //poll the event key and process accordingly 
    void poll() {
        events.clear();
        windowEvents.clear();
        mouseEvents.clear();
        keyboardEvents.clear();
        wheelX = 0;
        wheelY = 0;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (storeAllEvents) {
                events.push_back(event);
            }
            if (storeWindowEvents && event.type == SDL_WINDOWEVENT) {
                windowEvents.push_back(event.window);
            }
            if (storeMouseEvents) {
                switch (event.type) {
                    case SDL_MOUSEWHEEL:
                        mouseEvents.push_back(event.wheel);
                        break;
                    
                    case SDL_MOUSEMOTION:
                        mouseEvents.push_back(event.motion);
                        break;

                    case SDL_MOUSEBUTTONDOWN:
                    case SDL_MOUSEBUTTONUP:
                        mouseEvents.push_back(event.button);
                        break;
                }
            }
            if (storeKeyboardEvents) {
                switch (event.type) {
                    case SDL_KEYDOWN:
                    case SDL_KEYUP:
                        keyboardEvents.push_back(event.key);
                        break;
                }
            }

            //now to populate the state cache thing
            switch (event.type) {
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_0:
                            keyDown[0] = false;
                            break;
                        
                        case SDLK_1:
                            keyDown[1] = false;
                            break;
                        
                        case SDLK_2:
                            keyDown[2] = false;
                            break;
                        
                        case SDLK_3:
                            keyDown[3] = false;
                            break;
                        
                        case SDLK_4:
                            keyDown[4] = false;
                            break;
                        
                        case SDLK_5:
                            keyDown[5] = false;
                            break;
                        
                        case SDLK_6:
                            keyDown[6] = false;
                            break;
                        
                        case SDLK_7:
                            keyDown[7] = false;
                            break;
                        
                        case SDLK_8:
                            keyDown[8] = false;
                            break;
                        
                        case SDLK_9:
                            keyDown[9] = false;
                            break;
                        
                        case SDLK_a:
                            keyDown[A] = false;
                            break;
                        
                        case SDLK_b:
                            keyDown[B] = false;
                            break;
                        
                        case SDLK_c:
                            keyDown[C] = false;
                            break;
                        
                        case SDLK_d:
                            keyDown[D] = false;
                            break;
                        
                        case SDLK_e:
                            keyDown[E] = false;
                            break;
                        
                        case SDLK_f:
                            keyDown[F] = false;
                            break;
                        
                        case SDLK_g:
                            keyDown[G] = false;
                            break;
                        
                        case SDLK_h:
                            keyDown[H] = false;
                            break;
                        
                        case SDLK_i:
                            keyDown[I] = false;
                            break;
                        
                        case SDLK_j:
                            keyDown[J] = false;
                            break;
                        
                        case SDLK_k:
                            keyDown[K] = false;
                            break;
                        
                        case SDLK_l:
                            keyDown[L] = false;
                            break;
                        
                        case SDLK_m:
                            keyDown[M] = false;
                            break;
                        
                        case SDLK_n:
                            keyDown[N] = false;
                            break;
                        
                        case SDLK_o:
                            keyDown[O] = false;
                            break;
                        
                        case SDLK_p:
                            keyDown[P] = false;
                            break;
                        
                        case SDLK_q:
                            keyDown[Q] = false;
                            break;
                        
                        case SDLK_r:
                            keyDown[R] = false;
                            break;
                        
                        case SDLK_s:
                            keyDown[S] = false;
                            break;
                        
                        case SDLK_t:
                            keyDown[T] = false;
                            break;
                        
                        case SDLK_u:
                            keyDown[U] = false;
                            break;
                        
                        case SDLK_v:
                            keyDown[V] = false;
                            break;
                        
                        case SDLK_w:
                            keyDown[W] = false;
                            break;
                        
                        case SDLK_x:
                            keyDown[X] = false;
                            break;
                        
                        case SDLK_y:
                            keyDown[Y] = false;
                            break;
                        
                        case SDLK_z:
                            keyDown[Z] = false;
                            break;
                        
                        case SDLK_ESCAPE:
                            keyDown[ESC] = false;
                            break;

                        case SDLK_RETURN:
                            keyDown[ENTER] = false;
                            break;

                        case SDLK_TAB:
                            keyDown[TAB] = false;
                            break;

                        case SDLK_UNDERSCORE:
                            keyDown[SHIFT] = false;
                        case SDLK_MINUS:
                            keyDown[dash] = false;
                            break;

                        case SDLK_EQUALS:
                            keyDown[SHIFT] = false;
                        case SDLK_PLUS:
                            keyDown[equals] = false;
                            break;

                        case SDLK_RIGHT:
                            keyDown[RIGHT] = false;
                            break;

                        case SDLK_LEFT:
                            keyDown[RIGHT] = false;
                            break;

                        case SDLK_UP:
                            keyDown[RIGHT] = false;
                            break;
                            
                        case SDLK_DOWN:
                            keyDown[RIGHT] = false;
                            break;

                        case SDLK_LCTRL:
                        case SDLK_RCTRL:
                            keyDown[CTRL] = false;
                            break;

                        case SDLK_LSHIFT:
                        case SDLK_RSHIFT:
                            keyDown[SHIFT] = false;
                            break;

                        case SDLK_LALT:
                        case SDLK_RALT:
                            keyDown[ALT] = false;
                            break;
                    }
                    break;

                case SDL_KEYDOWN:
                    switch (event.key.keysym.sym) {
                        case SDLK_0:
                            keyDown[0] = true;
                            break;
                        
                        case SDLK_1:
                            keyDown[1] = true;
                            break;
                        
                        case SDLK_2:
                            keyDown[2] = true;
                            break;
                        
                        case SDLK_3:
                            keyDown[3] = true;
                            break;
                        
                        case SDLK_4:
                            keyDown[4] = true;
                            break;
                        
                        case SDLK_5:
                            keyDown[5] = true;
                            break;
                        
                        case SDLK_6:
                            keyDown[6] = true;
                            break;
                        
                        case SDLK_7:
                            keyDown[7] = true;
                            break;
                        
                        case SDLK_8:
                            keyDown[8] = true;
                            break;
                        
                        case SDLK_9:
                            keyDown[9] = true;
                            break;
                        
                        case SDLK_a:
                            keyDown[A] = true;
                            break;
                        
                        case SDLK_b:
                            keyDown[B] = true;
                            break;
                        
                        case SDLK_c:
                            keyDown[C] = true;
                            break;
                        
                        case SDLK_d:
                            keyDown[D] = true;
                            break;
                        
                        case SDLK_e:
                            keyDown[E] = true;
                            break;
                        
                        case SDLK_f:
                            keyDown[F] = true;
                            break;
                        
                        case SDLK_g:
                            keyDown[G] = true;
                            break;
                        
                        case SDLK_h:
                            keyDown[H] = true;
                            break;
                        
                        case SDLK_i:
                            keyDown[I] = true;
                            break;
                        
                        case SDLK_j:
                            keyDown[J] = true;
                            break;
                        
                        case SDLK_k:
                            keyDown[K] = true;
                            break;
                        
                        case SDLK_l:
                            keyDown[L] = true;
                            break;
                        
                        case SDLK_m:
                            keyDown[M] = true;
                            break;
                        
                        case SDLK_n:
                            keyDown[N] = true;
                            break;
                        
                        case SDLK_o:
                            keyDown[O] = true;
                            break;
                        
                        case SDLK_p:
                            keyDown[P] = true;
                            break;
                        
                        case SDLK_q:
                            keyDown[Q] = true;
                            break;
                        
                        case SDLK_r:
                            keyDown[R] = true;
                            break;
                        
                        case SDLK_s:
                            keyDown[S] = true;
                            break;
                        
                        case SDLK_t:
                            keyDown[T] = true;
                            break;
                        
                        case SDLK_u:
                            keyDown[U] = true;
                            break;
                        
                        case SDLK_v:
                            keyDown[V] = true;
                            break;
                        
                        case SDLK_w:
                            keyDown[W] = true;
                            break;
                        
                        case SDLK_x:
                            keyDown[X] = true;
                            break;
                        
                        case SDLK_y:
                            keyDown[Y] = true;
                            break;
                        
                        case SDLK_z:
                            keyDown[Z] = true;
                            break;
                        
                        case SDLK_ESCAPE:
                            keyDown[ESC] = true;
                            break;

                        case SDLK_RETURN:
                            keyDown[ENTER] = true;
                            break;

                        case SDLK_TAB:
                            keyDown[TAB] = true;
                            break;

                        case SDLK_UNDERSCORE:
                            keyDown[SHIFT] = true;
                        case SDLK_MINUS:
                            keyDown[dash] = true;
                            break;

                        case SDLK_EQUALS:
                            keyDown[SHIFT] = true;
                        case SDLK_PLUS:
                            keyDown[equals] = true;
                            break;

                        case SDLK_RIGHT:
                            keyDown[RIGHT] = true;
                            break;

                        case SDLK_LEFT:
                            keyDown[RIGHT] = true;
                            break;

                        case SDLK_UP:
                            keyDown[RIGHT] = true;
                            break;

                        case SDLK_DOWN:
                            keyDown[RIGHT] = true;
                            break;

                        case SDLK_LCTRL:
                        case SDLK_RCTRL:
                            keyDown[CTRL] = true;
                            break;

                        case SDLK_LSHIFT:
                        case SDLK_RSHIFT:
                            keyDown[SHIFT] = true;
                            break;

                        case SDLK_LALT:
                        case SDLK_RALT:
                            keyDown[ALT] = true;
                            break;
                    }
                    break;

                case SDL_MOUSEBUTTONUP:
                    mouseX = event.button.x;
                    mouseY = event.button.y;
                    switch(event.button.button) {
                        case SDL_BUTTON_LEFT:
                            buttonDown[left] = false;
                            break;

                        case SDL_BUTTON_RIGHT:
                            buttonDown[right] = false;
                            break;

                        case SDL_BUTTON_MIDDLE:
                            buttonDown[middle] = false;
                            break;
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    mouseX = event.button.x;
                    mouseY = event.button.y;
                    switch(event.button.button) {
                        case SDL_BUTTON_LEFT:
                            buttonDown[left] = true;
                            break;

                        case SDL_BUTTON_RIGHT:
                            buttonDown[right] = true;
                            break;

                        case SDL_BUTTON_MIDDLE:
                            buttonDown[middle] = true;
                            break;
                    }
                    break;

                case SDL_MOUSEMOTION:
                    mouseX = event.motion.x;
                    mouseY = event.motion.y;
                    break;

                case SDL_MOUSEWHEEL:
                    wheelX = event.wheel.x;
                    wheelY = event.wheel.y;
                    break;
            }
        }
    }

};

#endif /* EVENT_HANDLER_H */