#include "Renderer.h"
#include <glad/glad.h>

void Renderer::Initialize() {

}

void Renderer::Shutdown() {

}

void Renderer::DrawIndex(unsigned int indicesCount) {
    glDrawElements(GL_TRIANGLES, (GLsizei)indicesCount, GL_UNSIGNED_INT, 0);
}

void Renderer::DrawArray(unsigned int verticesCount) {
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei)verticesCount);
}
