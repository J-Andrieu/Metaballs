#include "Ball.h"
#include "Shader.h"

#define INVALID_UNIFORM_LOCATION 0x7fffffff

class Graphics;

// required by the rendering functions
typedef struct {
    Graphics* graphics;
} drawParams;

class Graphics {
public:
    Graphics(int height, int width);
    ~Graphics();

    GUIWindow* Window();
    int height();
    int width();
    void updateDimensions();

    void update();// Update positions of metaballs

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
    float m_menuWidth;

    // members related to the main window
    GUIWindow* m_window;
    drawParams m_params;
    static void m_drawGUIFunc(void*);// This will also update general settings
    static void m_drawFunc(void*);

    // shader variables
    typedef enum {
        Default,
        NumShaderTypes
    } ShaderType;
    Shader::GraphicsProgram* m_tex2ScreenRender;
    std::vector<Shader::ComputeProgram*> m_computeShaders;
    GLuint m_computeUniformTime;
    GLuint m_renderUniformSize;
    GLuint m_metaballsSSBO;
    GLuint m_ssboBindingIndex;
    void bindSSBO();

    //metaball data
    bool m_genSSBO;
    bool m_wigglyMovement;
    size_t m_numBalls;//needed for shaders
    std::vector<Ball> m_metaballs;
    std::vector<ImVec4> m_colors;
    void pushBall(Ball ball);
    void pushBall();
    void popBall();
    void drawBallInterface();
};