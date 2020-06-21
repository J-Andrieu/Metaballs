#include "Ball.h"
#include "Shader.h"

#define INVALID_UNIFORM_LOCATION 0x7fffffff

// struct to hold pointers to all class members
// required by the rendering functions
typedef struct {
    GUIWindow* window;
    bool* sizeChanged;
    GLuint* texOut;
    int* height;
    int* width;
    Shader::ComputeProgram* computeProg;
    Shader::GraphicsProgram* tex2ScreenProg;
    float* timeOffset;
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
    // members utilized by rendering functions
    float m_sliderQuad[4];
    float m_gradientSpeed;
    bool m_sizeChanged;
    int m_height;
    int m_width;
    static GLfloat s_quadVertexBufferData[8];
    GLuint m_quadVAO;
    GLuint m_texOut;
    float m_timeOffset;

    // members related to the main window
    GUIWindow* m_window;
    drawParams m_params;
    static void m_drawGUIFunc(void*);
    static void m_drawFunc(void*);

    // shader variables
    Shader::GraphicsProgram* m_tex2ScreenRender;
    Shader::ComputeProgram* m_defaultCompute;
    GLuint m_computeUniformTime;
    GLuint m_renderUniformSize;
};