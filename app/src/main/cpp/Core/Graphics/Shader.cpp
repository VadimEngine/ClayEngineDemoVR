// core
#include "Core/Graphics/Shader.h"

Shader::Shader(const char* vertexShaderCode, const char* fragmentShaderCode) {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderCode, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderCode, nullptr);
    glCompileShader(fragmentShader);

    // Create and link the shader program
    mProgramId_ = glCreateProgram();
    glAttachShader(mProgramId_, vertexShader);
    glAttachShader(mProgramId_, fragmentShader);
    glLinkProgram(mProgramId_);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

Shader::~Shader() {

}

void Shader::bind() {
    glUseProgram(mProgramId_);
}

void Shader::unbind() {
    glUseProgram(0);
}

void Shader::setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(mProgramId_,name.c_str()), (int)value);
}

void Shader::setMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(mProgramId_,name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

void Shader::setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(mProgramId_,name.c_str()), value);
}
void Shader::setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(mProgramId_,name.c_str()), value);
}

void Shader::setVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(mProgramId_,name.c_str()), 1, &value[0]);
}

void Shader::setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(mProgramId_,name.c_str()), x, y);
}

void Shader::setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(mProgramId_,name.c_str()), 1, &value[0]);
}

void Shader::setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(mProgramId_,name.c_str()), x, y, z);
}
// glGetUniformLocation(mProgramId_,name.c_str())

void Shader::setVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(mProgramId_,name.c_str()), 1, &value[0]);
}

void Shader::setVec4(const std::string &name, float x, float y, float z, float w) const {
    glUniform4f(glGetUniformLocation(mProgramId_,name.c_str()), x, y, z, w);
}

void Shader::setMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(
            glGetUniformLocation(mProgramId_,name.c_str()),
            1,
            GL_FALSE,
            &mat[0][0]
    );
}

void Shader::setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(mProgramId_,name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

GLuint Shader::getProgramId() const {
    return mProgramId_;
}
