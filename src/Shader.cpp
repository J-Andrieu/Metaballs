#include "Shader.h"

/// Returns a shader type string from a GLenum
std::string resolveShaderType(GLenum shaderType) {
    switch (shaderType) {
        case GL_VERTEX_SHADER:
            return "GL_VERTEX_SHADER";
        case GL_TESS_CONTROL_SHADER:
            return "GL_TESS_CONTROL_SHADER";
        case GL_TESS_EVALUATION_SHADER:
            return "GL_TESS_EVALUATION_SHADER";
        case GL_GEOMETRY_SHADER:
            return "GL_GEOMETRY_SHADER";
        case GL_FRAGMENT_SHADER:
            return "GL_FRAGMENT_SHADER";
        case GL_COMPUTE_SHADER:
            return "GL_COMPUTE_SHADER";
        default:
            return "INVALID_SHADER_TYPE";
    }
}

using namespace Shader;

/** Shader Constructor
 *  @param shaderType The GLenum for the desired shader
 *
 *  @note Throws a runtime error if a shader cannot be constructed
 */
shader::shader(GLenum shaderType) {
    m_shaderType = shaderType;
    m_shaderObj = glCreateShader(shaderType);

    if (m_shaderObj == 0) {
        throw std::runtime_error(std::string("Error creating shader of type ") +
                                 resolveShaderType(shaderType));
    }
}

/** Shader Constructor
 *  @param shaderFile An open file stream to the file containing the shader
 * source code
 *  @param shaderType The GLenum for the desired shader
 *
 *  @note Throws a runtime error if a shader cannot be constructed
 */
shader::shader(std::ifstream& shaderFile, GLenum shaderType)
    : shader(shaderType) {
    setSource(shaderFile);
}

/** Shader Constructor
 *  @param shaderString A string contiaining the shader source code
 *  @param shaderType The GLenum for the desired shader
 *
 *  @note Throws a runtime error if a shader cannot be constructed
 */
shader::shader(std::string& shaderString, GLenum shaderType)
    : shader(shaderType) {
    setSource(shaderString);
}

/// Shader destructor
shader::~shader() { glDeleteShader(m_shaderObj); }

/** Sets the source code for a shader
 *  @param numStrings The number of individual strings containing the source
 * code
 *  @param strings An array of strings containing the source
 *  @param lengths An array containing the length of each string
 */
void shader::setSource(GLsizei numStrings, const GLchar** strings,
                       GLint* lengths) {
    glShaderSource(m_shaderObj, numStrings, strings, lengths);
}

/** Sets the source code for a shader
 *  @param shaderString An std::string containing the shader source code
 */
void shader::setSource(std::string& shaderString) {
    const GLchar* p[1];
    p[0] = shaderString.c_str();
    GLint lengths[1] = {(GLint)shaderString.size()};

    setSource(1, p, lengths);
}

/** Sets the source code for a shader
 *  @param shaderFile An open file stream for the file containing the source
 * code
 */
void shader::setSource(std::ifstream& shaderFile) {
#if 1
    // use string stream to read in entire file
    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    m_shaderSource = buffer.str();

    setSource(m_shaderSource);
#else
    // reserve string same size as file then read in the file
    shaderFile.seekg(0, std::ios::end);
    std::streampos fileLen = shaderFile.tellg();
    shaderFile.seekg(0, std::ios::beg);

    m_shaderSource.reserve(fileLen);

    shaderFile.read(m_shaderSource.data(), fileLen);
    setSource(m_shaderSource);
#endif
}

/** Compiles the shader
 *  @note Throws a runtime error if compilation fails and prints the log to the
 * console
 */
void shader::compile() {
    glCompileShader(m_shaderObj);

    GLint success;
    glGetShaderiv(m_shaderObj, GL_COMPILE_STATUS, &success);

    if (!success) {
        GLchar log[1024];
        glGetShaderInfoLog(m_shaderObj, 1024, NULL, log);
        std::cerr << "Error compiling: " << log << std::endl;
        throw std::runtime_error(
            std::string("Error compiling shader of type ") +
            resolveShaderType(m_shaderType));
    }
}

/// Returns the GLuint shader object for OpenGL/GLEW functions
GLuint shader::object() { return m_shaderObj; }

/// Casts to a GLuint shader object for OpenGL/GLEW functions
shader::operator GLuint() { return m_shaderObj; }

/** ProgramBase constructor
 *  @note Throws a runtime error if a program can't be created
 */
ProgramBase::ProgramBase() {
    m_program = glCreateProgram();

    if (m_program == 0) {
        throw std::runtime_error(std::string("Error creating shader program"));
    }
}

/// ProgramBase destructor
ProgramBase::~ProgramBase() { glDeleteProgram(m_program); }

/** Attaches a shader to the program
 *  @param shaderObj The shader to attach to the program
 */
void ProgramBase::attachShader(shader& shaderObj) {
    glAttachShader(m_program, shaderObj);
}

/** Links the program
 *  @note Throws a runtime error if linking fails or the program is invalid
 */
void ProgramBase::build() {
    GLint success = 0;
    GLchar errorLog[1024] = {0};

    glLinkProgram(m_program);

    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (success == 0) {
        glGetProgramInfoLog(m_program, sizeof(errorLog), NULL, errorLog);
        std::cerr << "Error linking shader program: " << errorLog << std::endl;
        throw std::runtime_error(
            std::string("Error linking shader program"));
    }

    glValidateProgram(m_program);
    glGetProgramiv(m_program, GL_VALIDATE_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(m_program, sizeof(errorLog), NULL, errorLog);
        std::cerr << "Invalid shader program: " << errorLog << std::endl;
        throw std::runtime_error(
            std::string("GPU program is invalid"));
    }
}

/// Sets the calling program as the active program for OpenGL
void ProgramBase::setActiveProgram() { glUseProgram(m_program); }

/// Returns the GLuint program for OpenGL/GLEW functions
GLuint ProgramBase::program() { return m_program; }

/// Casts to a GLuint program for OpenGL/GLEW functions
ProgramBase::operator GLuint() { return m_program; }

/** GraphicsProgram parametarized constructor
 *  @param shaders A list of shaders to attach to the program
 *
 *  @note Throws a runtime error if a program can't be created
 */
GraphicsProgram::GraphicsProgram(std::initializer_list<shader> shaders)
    : GraphicsProgram() {
    for (auto _shader : shaders) {
        attachShader(_shader);
    }
}

/** ComputeProgram parameterized constructor
 *  @param shaderObj The compute shader to attach to the program
 *
 *  @note Throws a runtime error if a program can't be created
 */
ComputeProgram::ComputeProgram(shader& shaderObj) : ComputeProgram() {
    attachShader(shaderObj);
}

/** Dispatches the compute program
 *  @param x The x dimension of the data
 *  @param y the y dimension of the data
 *  @param z the z dimension of the data
 */
void ComputeProgram::dispatch(GLuint x, GLuint y, GLuint z) {
    glDispatchCompute(x, y, z);
}

/// Indirectly dispatches the compute program
void ComputeProgram::dispatchIndirect(GLintptr indirect) {
    glDispatchComputeIndirect(indirect);
}
