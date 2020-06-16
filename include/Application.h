#include "Graphics.h"

typedef std::chrono::duration<float, std::micro> microseconds;
typedef std::chrono::duration<float, std::ratio<1, 1>> seconds;

typedef struct {
  int height;
  int width;
  double fps_cap;
} cmdParams;

class Application {
public:
    Application(int argc, char* argv[]);
    ~Application();

    void run();

private:
    cmdParams m_params;
    bool parseCMD(int argc, char* argv[]);

    static TermFormatter::Formatter s_green;
    static TermFormatter::Formatter s_red;
    static TermFormatter::Formatter s_reset;

    float m_FPS;
    size_t m_frameCount;

    Graphics* m_graphics;
    EventHandler m_handler;
};