#include "Application.h"

TermFormatter::Formatter Application::s_green = TermFormatter::Formatter({TermFormatter::FG_Green});
TermFormatter::Formatter Application::s_red = TermFormatter::Formatter({TermFormatter::FG_Red});
TermFormatter::Formatter Application::s_reset = TermFormatter::Formatter({TermFormatter::ResetAll});

Application::Application(int argc, char* argv[]) {
    m_params.height = 0;
    m_params.width = 0;
    m_params.fps_cap = 60;
    if (!parseCMD(argc, argv)) {
        exit(-1);
    }
    m_graphics = new Graphics(m_params.height, m_params.width);

    m_FPS = m_params.fps_cap;
    m_frameCount = 0;
}

Application::~Application() {
    delete m_graphics;
}


void Application::run() {
    Timer frameTimer;
    bool running = true;
    while (running) {
        frameTimer.start();
        m_handler.poll();

        for (auto event : m_handler.events) {
            m_graphics->Window()->handleEvent(event);
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        if (m_handler.keyDown[EventHandler::keys::ESC] || m_graphics->Window()->isHidden()) {
            running = false;
        }

        //update window
        if (running) {
            int newWidth, newHeight;
            SDL_GetWindowSize(*m_graphics->Window(), &newWidth, &newHeight);
            if (newWidth != m_graphics->Height() || newHeight != m_graphics->Width()) {
                m_graphics->updateDimensions();
            }
            m_graphics->Window()->draw();
            m_graphics->Window()->drawGUI();
            m_graphics->Window()->swap();
        }

        auto endTime = (microseconds) frameTimer.getMicrosecondsElapsed();
        //print out current frame rate, green if it's fast, red if it's slow
        if(m_frameCount % 15 == 0) {
            float currFPS = 1.0f / (endTime.count() / 1000000);
            std::cout << "\r                      \r" << std::flush;
            if (currFPS >= m_FPS) {
                std::cout << s_green << currFPS;
            } else {
                std::cout << s_red << currFPS;
            }
            std::cout << std::flush;
        }
        m_frameCount++;
    }
  std::cout << s_reset << '\r';
}

bool Application::parseCMD(int argc, char* argv[]) {
  std::string size; //HxW
  CMDParser parser;
  parser.bindVar<std::string>("-size", size, 1, "<Height>x<Width> of the window");
  parser.bindVar<double>("-fps", m_params.fps_cap, 1, "FPS cap");
  if (!parser.parse(argc, argv)) {
    return false;
  }
  if (size.size() != 0) {
    std::string height;
    std::string width;
    size_t x_index = 0;

    //split on x
    while (x_index < size.size() && size[x_index] != 'x') {
      x_index++;
    }
    if (x_index >= size.size()) {
      std::cout << "Incorrect format, -size requires format <Height>x<Width>" << std::endl;
      parser.printHelp();
      return false;
    }
    height = size.substr(0, x_index);
    width = size.substr(x_index + 1, size.size() - x_index - 1);

    //attempt to convert substrings to integers
    try {
      m_params.height = std::stoi(height);
    } catch (...) {
      std::cout << "Could not convert " << height << " to integer height" << std::endl;
      parser.printHelp();
      return false;
    }
    try {
      m_params.width = std::stoi(width);
    } catch (...) {
      std::cout << "Could not convert " << width << " to integer width" << std::endl;
      parser.printHelp();
      return false;
    }
  }
  return true;
}