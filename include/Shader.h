#include "general_tools/All.h"

#include <fstream>
#include <string>
#include <sstream>
#include <initializer_list>
#include <cstdarg>

namespace Shader {

    class shader {
    public:
        shader(GLenum shaderType);
        shader(std::ifstream& shaderFile, GLenum shaderType);
        shader(std::string& shaderString, GLenum shaderType);

        ~shader();

        void setSource(GLsizei numStrings, const GLchar** strings, GLint* lengths);
        void setSource(std::ifstream& shaderFile);
        void setSource(std::string& shaderString);
        
        void compile();

        GLuint object();
        operator GLuint();
    
    private:
        GLenum m_shaderType;
        GLuint m_shaderObj;
    };

    class GraphicsProgram {
    public:
        GraphicsProgram();
        GraphicsProgram(std::initializer_list<shader> shaders);

        ~GraphicsProgram();

        void attachShader(shader& shaderObj);
        void build();

        void setActiveProgram();

        GLuint program();
        operator GLuint();

    private:
        GLuint m_program;
    };

    class ComputeProgram {
    public:
        ComputeProgram();
        ComputeProgram(shader& shaderObj);

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