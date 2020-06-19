#include "general_tools/All.h"

#include <fstream>
#include <string>
#include <sstream>
#include <initializer_list>
#include <cstdarg>

/** Namespace for shader classes and functions
 *  @namespace Shader
 */
namespace Shader {

    /** Shader construction class
     *  @class shader
     */
    class shader {
    public:
        shader() = delete;
        shader(GLenum shaderType);
        shader(std::ifstream& shaderFile, GLenum shaderType);
        shader(std::string& shaderString, GLenum shaderType);
        
        //shader(shader&& shaderObj);
        //shader& operator=(shader&& shaderObj);
        //shader(const shader& shaderObj) = delete;
        //shader& operator=(const shader& shaderObj) = delete;

        ~shader();

        void setSource(GLsizei numStrings, const GLchar** strings, GLint* lengths);
        void setSource(std::ifstream& shaderFile);
        void setSource(std::string& shaderString);
        
        void compile();

        GLuint object();
        operator GLuint();
    
    private:
        std::string m_shaderSource;///< holds generated string for initializing from file
        GLenum m_shaderType;
        GLuint m_shaderObj;
    };

    /** Class for storing a graphics pipeline shader program
     *  @class GraphicsProgram
     */
    class GraphicsProgram {
    public:
        GraphicsProgram();
        GraphicsProgram(std::initializer_list<shader> shaders);

        GraphicsProgram(GraphicsProgram&& program);
        GraphicsProgram& operator=(GraphicsProgram&& program);
        GraphicsProgram(const GraphicsProgram& program) = delete;
        GraphicsProgram& operator=(const GraphicsProgram& program) = delete;

        ~GraphicsProgram();

        void attachShader(shader& shaderObj);
        void build();

        void setActiveProgram();

        GLuint program();
        operator GLuint();

    private:
        GLuint m_program;
    };

    /** Class for storing a compute shader program
     *  @class ComputeProgram
     */
    class ComputeProgram {
    public:
        ComputeProgram();
        ComputeProgram(shader& shaderObj);

        ComputeProgram(ComputeProgram&& program);
        ComputeProgram& operator=(ComputeProgram&& program);
        ComputeProgram(const ComputeProgram& program) = delete;
        ComputeProgram& operator=(const ComputeProgram& program) = delete;

        ~ComputeProgram();

        void attachShader(shader& shaderObj);
        void build();

        void setActiveProgram();
        void dispatch(GLuint x, GLuint y, GLuint z);
        void dispatchIndirect(GLintptr indirect);

        GLuint program();
        operator GLuint();

    private:
        GLuint m_program;
    };

};