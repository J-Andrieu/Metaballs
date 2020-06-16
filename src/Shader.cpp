#include "Shader.h"

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

shader::shader(GLenum shaderType) {
    m_shaderType = shaderType;
    m_shaderObj = glCreateShader(shaderType);

    if (m_shaderObj == 0) {
		throw std::runtime_error(std::string("Error creating shader of type ") + resolveShaderType(shaderType));
	}
}

shader::shader(std::ifstream& shaderFile, GLenum shaderType) : shader(shaderType) {
    setSource(shaderFile);
}

shader::shader(std::string& shaderString, GLenum shaderType) : shader(shaderType) {
    setSource(shaderString);
}

shader::~shader() {
    glDeleteShader(m_shaderObj);
}

void shader::setSource(GLsizei numStrings, const GLchar** strings, GLint* lengths) {
    glShaderSource(m_shaderObj, numStrings, strings, lengths);
}

void shader::setSource(std::string& shaderString) {
    const GLchar* p[1];
    p[0] = shaderString.c_str();
    GLint lengths[1] = { (GLint) shaderString.size() };

    setSource(1, p, lengths);
}

void shader::setSource(std::ifstream& shaderFile) {
    std::stringstream buffer;
    buffer << shaderFile.rdbuf();
    std::string shaderSource = buffer.str();
    setSource(shaderSource);
}

void shader::compile() {
    glCompileShader(m_shaderObj);

    GLint success;
	glGetShaderiv(m_shaderObj, GL_COMPILE_STATUS, &success);

    if (!success) {
		GLchar log[1024];
		glGetShaderInfoLog(m_shaderObj, 1024, NULL, log);
		std::cerr << "Error compiling: " << log << std::endl;
        throw std::runtime_error(std::string("Error compiling shader of type ") + resolveShaderType(m_shaderType));
	}
}

GLuint shader::object() {
    return m_shaderObj;
}

shader::operator GLuint() {
    return m_shaderObj;
}

GraphicsProgram::GraphicsProgram() {
    m_program = glCreateProgram();
}

GraphicsProgram::GraphicsProgram(std::initializer_list<shader> shaders) : GraphicsProgram() {
    for (auto _shader : shaders) {
        attachShader(_shader);
    }
}

GraphicsProgram::~GraphicsProgram() {
    glDeleteProgram(m_program);
}

void GraphicsProgram::attachShader(shader& shaderObj) {
    glAttachShader(m_program, shaderObj);
}

void GraphicsProgram::build() {
    GLint success = 0;
	GLchar errorLog[1024] = { 0 };

	glLinkProgram (m_program);

    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
	if (success == 0) {
		glGetProgramInfoLog(m_program, sizeof(errorLog), NULL, errorLog);
		std::cerr << "Error linking shader program: " << errorLog << std::endl;
		throw std::runtime_error(std::string("Error linking graphical shader program"));
	}

    glValidateProgram(m_program);
	glGetProgramiv(m_program, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_program, sizeof(errorLog), NULL, errorLog);
		std::cerr << "Invalid shader program: " << errorLog << std::endl;
		throw std::runtime_error(std::string("Rendering pipeline program is invalid"));
	}
}

void GraphicsProgram::setActiveProgram() {
    glUseProgram(m_program);
}

GLuint GraphicsProgram::program() {
    return m_program;
}

GraphicsProgram::operator GLuint() {
    return m_program;
}

ComputeProgram::ComputeProgram() {
    m_program = glCreateProgram();
}

ComputeProgram::ComputeProgram(shader& shaderObj) : ComputeProgram() {
    attachShader(shaderObj);
}

ComputeProgram::~ComputeProgram() {
    glDeleteProgram(m_program);
}

void ComputeProgram::attachShader(shader& shaderObj) {
    glAttachShader(m_program, shaderObj);
}

void ComputeProgram::build() {
    GLint success = 0;
	GLchar errorLog[1024] = { 0 };

	glLinkProgram (m_program);

    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
	if (success == 0) {
		glGetProgramInfoLog(m_program, sizeof(errorLog), NULL, errorLog);
		std::cerr << "Error linking shader program: " << errorLog << std::endl;
		throw std::runtime_error(std::string("Error linking graphical shader program"));
	}

    glValidateProgram(m_program);
	glGetProgramiv(m_program, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_program, sizeof(errorLog), NULL, errorLog);
		std::cerr << "Invalid shader program: " << errorLog << std::endl;
		throw std::runtime_error(std::string("Rendering pipeline program is invalid"));
	}
}

void ComputeProgram::setActiveProgram() {
    glUseProgram(m_program);
}

void ComputeProgram::dispatch(GLuint x, GLuint y, GLuint z) {
    glDispatchCompute(x, y, z);
}

void ComputeProgram::dispatchIndirect(GLintptr indirect) {
    glDispatchComputeIndirect(indirect);
}

GLuint ComputeProgram::program() {
    return m_program;
}

ComputeProgram::operator GLuint() {
    return m_program;
}