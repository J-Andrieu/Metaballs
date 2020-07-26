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
        Circles = Default,
        Cells,
        Meta_BlueGreen,
        Meta_RedOrange,
        Meta_RGB,
        Meta_Params,
        NumShaderTypes
    } ShaderType;
    ShaderType m_currentShader;
    std::string m_shaderName;
    std::vector<Shader::ComputeProgram*> m_computeShaders;
    GLuint m_metaballsSSBO;
    GLuint m_ssboBindingIndex;
    GLuint m_cellsUniform_thresh;
    float m_cellsThresh;
    GLuint m_metaBGUniform_radiusMult;
    float m_metaBGRadiusMult;
    GLuint m_metaROUniform_radiusMult;
    float m_metaRORadiusMult;
    GLuint m_metaRGBUniform_radiusMult;
    float m_metaRGBRadiusMult;
    GLuint m_metaParamUniform_radiusMult;
    GLuint m_metaParamUniform_red;
    GLuint m_metaParamUniform_green;
    GLuint m_metaParamUniform_blue;
    GLuint m_metaParamUniform_high;
    float m_metaParamRadiusMult;
    bool m_metaParamRed;
    bool m_metaParamGreen;
    bool m_metaParamBlue;
    bool m_metaParamHigh;
    void bindSSBO();
    Ball* m_ssboData;

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