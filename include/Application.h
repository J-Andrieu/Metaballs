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

    run();

private:
    cmdParams m_params;
    bool parseCMD(int argc, char* argv[]);

    TermFormatter::Formatter m_green;
    TermFormatter::Formatter m_red;
    TermFormatter::Formatter m_reset;

    float m_FPS;
    size_t m_frameCount;

    Graphics m_graphics;
};