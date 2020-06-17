#include "Shader.h"
#include "Ball.h"

#define INVALID_UNIFORM_LOCATION 0x7fffffff

typedef struct {
    GUIWindow* window;
    bool* sizeChanged;
    GLuint* texOut;
    int* height;
    int* width;
    Shader::ComputeProgram* computeProg;
    Shader::GraphicsProgram* tex2ScreenProg;
    GLuint* timeOffset;
    float* gradientSpeed;
    GLuint* uniformTime;
    GLuint* uniformSize;
    GLuint* quadVAO;
    float* sliderQuad;
} drawParams;

class Graphics {
public:
    Graphics(int height, int width);
    ~Graphics();

    GUIWindow* Window();
    int Height();
    int Width();
    void updateDimensions();

private:
    float m_sliderQuad[4];
    float m_gradientSpeed;
    bool m_sizeChanged;
    int m_height;
    int m_width;
    static GLfloat s_quadVertexBufferData[8];
    GLuint m_quadVAO;
    GLuint m_texOut;
    GLuint m_timeOffset;
    
    GUIWindow* m_window;
    drawParams m_params;
    static void m_drawGUIFunc(void*);
    static void m_drawFunc(void*);
    

    Shader::GraphicsProgram* m_tex2ScreenRender;
    Shader::ComputeProgram* m_defaultCompute;
    GLuint m_computeUniformTime;
    GLuint m_renderUniformSize;
};