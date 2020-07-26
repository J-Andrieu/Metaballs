#include <cstdarg>
#include <fstream>
#include <initializer_list>
#include <sstream>
#include <string>

#include "general_tools/All.h"

/** Namespace for shader classes and functions
 *  @namespace Shader
 */
namespace Shader {

    /** Shader construction class
     *  @class shader
     */
    class shader {
    public:

        typedef enum {
            GLSL,
            SPIRV
        } SourceType;
        
        shader() = delete;
        shader(GLenum shaderType);
        shader(std::ifstream& shaderFile, GLenum shaderType, bool isSPIRV = false);
        shader(std::string& shaderString, GLenum shaderType, bool isSPIRV = false);

        // shader(shader&& shaderObj);
        // shader& operator=(shader&& shaderObj);
        // shader(const shader& shaderObj) = delete;
        // shader& operator=(const shader& shaderObj) = delete;

        ~shader();

        void setSource(GLsizei numStrings, const GLchar** strings,
                       GLint* lengths, bool isSPIRV = false);
        void setSource(std::ifstream& shaderFile, bool isSPIRV = false);
        void setSource(std::string& shaderString, bool isSPIRV = false);

        void specialize(std::string entryPoint = std::string("main"), int numSpecializationConstants = 0, const GLuint *pConstantIndex​ = nullptr, const GLuint *pConstantValue​ = nullptr);
        void compile();

        GLuint object();
        operator GLuint();

    private:
        std::string m_shaderSource;  ///< holds generated string for
                                     ///< initializing from file
        GLenum m_shaderType;
        GLuint m_shaderObj;
        SourceType m_sourceType;
    };

    /// Base class for OpenGL program containers
    class ProgramBase {
    protected:
        ProgramBase();
        ~ProgramBase();

    public:
        void attachShader(shader& shaderObj);
        void build();

        void setActiveProgram();

        GLuint program();
        operator GLuint();

    private:
        GLuint m_program;
    };

    /** Class for storing a graphics pipeline shader program
     *  @class GraphicsProgram
     */
    class GraphicsProgram : public ProgramBase {
    public:
        /** GraphicsProgram default constructor
         *  @note Throws a runtime error if a program can't be created
         */
        GraphicsProgram() : ProgramBase(){};
        GraphicsProgram(std::initializer_list<shader> shaders);

        GraphicsProgram(GraphicsProgram&& program);
        GraphicsProgram& operator=(GraphicsProgram&& program);
        GraphicsProgram(const GraphicsProgram& program) = delete;
        GraphicsProgram& operator=(const GraphicsProgram& program) = delete;
    };

    /** Class for storing a compute shader program
     *  @class ComputeProgram
     */
    class ComputeProgram : public ProgramBase {
    public:
        /** ComputeProgram default constructor
         *  @note Throws a runtime error if a program can't be created
         */
        ComputeProgram() : ProgramBase(){};
        ComputeProgram(shader& shaderObj);

        ComputeProgram(ComputeProgram&& program);
        ComputeProgram& operator=(ComputeProgram&& program);
        ComputeProgram(const ComputeProgram& program) = delete;
        ComputeProgram& operator=(const ComputeProgram& program) = delete;

        void dispatch(GLuint x, GLuint y, GLuint z);
        void dispatchIndirect(GLintptr indirect);
    };

};  // namespace Shader