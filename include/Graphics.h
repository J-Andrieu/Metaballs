#include "Shader.h"
#include "Ball.h"

#define INVALID_UNIFORM_LOCATION 0x7fffffff

class Graphics;

typedef struct {
  GUIWindow* window;
  float* vals;
  float* speed;
} guiParams;

typedef struct {
  GLuint computeProg;
  GLuint renderProg;
  float* speed;
  Graphics* graphics;
} graphicsParams;

class Graphics {
public:
    Graphics(int height, int width);
    ~Graphics();

    GUIWindow& Window();
    size_t Height();
    size_t Width();
    void updateDimensions();

private:
    float m_sliderQuad[4];
    float m_gradientSpeed;
    bool m_sizeChanged;
    size_t m_height;
    size_t m_width;
    static GLfloat s_quadVertexBufferData*;
    GLuint m_quadVAO;
    GLuint m_texOut;
    GLuint m_timeOffset;
    
    GUIWindow m_window;
    void m_drawGUIFunc(void*);
    void m_drawFunc(void*);
    

    Shader::GraphicsProgram m_tex2ScreenRender;
    Shader::CopmuteProgram m_defaultCompute;
};