// application
#include "Application/BasicScene.h"

float BasicScene::triangleVertices[] = {
        0.0f, 0.5f, 0.0f,   // Vertex 1 (x, y)
        -0.5f, -0.5f, 0.0f, // Vertex 2 (x, y)
        0.5f, -0.5f, 0.0f   // Vertex 3 (x, y)
};

BasicScene::BasicScene() {
    mShader_ = new Shader(vertexShaderSource, fragmentShaderSource);
    mShader_->bind();
    // Set up vertex data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, triangleVertices); // is this duplicated?

    // Generate and bind a vertex array object (VAO)
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Generate and bind a vertex buffer object (VBO)
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // Enable vertex array
    glEnableVertexAttribArray(0);

    // unbind?
    glBindVertexArray(0);
    mShader_->unbind();
}

BasicScene::~BasicScene() {
    // TODO CLEAN UP SHADER/OPENGL
}

void BasicScene::render() {
    //glUseProgram(shaderProgram);
    mShader_->bind();
    glBindVertexArray(vao);

    glm::vec3 mRotation_(0,0,0);
    glm::vec3 mScale_(1,1,1);
    glm::vec3 mPosition_(0,0,0);

    glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1), glm::radians(mRotation_.x), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), mScale_);
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), mPosition_);


    mShader_->setMat4("model", translationMatrix * rotationMatrix * scaleMatrix);
    mShader_->setMat4("view", mCamera_.getViewMatrix());
    mShader_->setMat4("projection", mCamera_.getProjectionMatrix());

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0); // unbind
}

Camera* BasicScene::getCamera() {
    return &mCamera_;
}
