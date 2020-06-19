#include "Graphics.h"

typedef std::chrono::duration<float, std::micro> microseconds;
typedef std::chrono::duration<float, std::ratio<1, 1>> seconds;

//struct to hold application parameters
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
    //command line handling
    cmdParams m_params;
    bool parseCMD(int argc, char* argv[]);

    //Terminal formats for coloring output
    static TermFormatter::Formatter s_green;
    static TermFormatter::Formatter s_red;
    static TermFormatter::Formatter s_reset;

    //variables for framerate
    float m_FPS;
    size_t m_frameCount;

    //graphics variables
    Graphics* m_graphics;
    EventHandler m_handler;
};