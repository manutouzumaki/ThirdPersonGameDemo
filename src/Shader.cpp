#include "Shader.h"
#include <windows.h>
#include <assert.h>
#include <glad/glad.h>
#include <stdio.h>

FileResult ReadFile(const char* filepath)
{
    FileResult result = {};
    HANDLE file_handle = CreateFileA(filepath, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
    if (file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;
        if (GetFileSizeEx(file_handle, &file_size)) {
            assert(file_size.QuadPart <= 0xFFFFFFFF);
            result.data = malloc((size_t)file_size.QuadPart + 1);
            result.size = (size_t)file_size.QuadPart;
            if (ReadFile(file_handle, result.data, (DWORD)result.size, 0, 0)) {
                unsigned char* last_byte = (unsigned char*)result.data + result.size;
                *last_byte = 0;
                return result;
            }
        }
    }
    FileResult zeroResult = {};
    return zeroResult;
}

void DeleteFile(FileResult *file) {
    if(file->data) free(file->data);
}

void Shader::Initialize(const char *vertexPath, const char *fragmentPath) {
    FileResult vertexResult = ReadFile(vertexPath);
    FileResult fragmentResult = ReadFile(fragmentPath);
    const char *vertexSrc = (const char *)vertexResult.data;
    const char *fragmentSrc = (const char *)fragmentResult.data;

    int succes;
    char infoLog[512];

    int vertexId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexId, 1, &vertexSrc, 0);
    glCompileShader(vertexId);
    glGetShaderiv(vertexId, GL_COMPILE_STATUS, &succes);
    if (!succes) {
        printf("Error Compiling: %s\n", vertexPath);
        glGetShaderInfoLog(vertexId, 512, 0, infoLog);
        printf("%s\n", infoLog);
        assert(!"Error Vertex Shader");
    }

    int fragmentId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentId, 1, &fragmentSrc, 0);
    glCompileShader(fragmentId);
    glGetShaderiv(fragmentId, GL_COMPILE_STATUS, &succes);
    if (!succes) {
        printf("Error Compiling: %s\n", fragmentPath);
        glGetShaderInfoLog(fragmentId, 512, 0, infoLog);
        printf("%s\n", infoLog);
        assert(!"Error Fragment Shader");
    }

    mProgram = glCreateProgram();
    glAttachShader(mProgram, vertexId);
    glAttachShader(mProgram, fragmentId);
    glLinkProgram(mProgram);
    glGetProgramiv(mProgram, GL_LINK_STATUS, &succes);
    if (!succes) {
        printf("Error Linking Shader Program\n");
        glGetProgramInfoLog(mProgram, 512, 0, infoLog);
        printf("%s\n", infoLog);
    }

    glDeleteShader(vertexId);
    glDeleteShader(fragmentId);
    
    DeleteFile(&vertexResult);
    DeleteFile(&fragmentResult);
}

void Shader::Shutdown() {
    glDeleteProgram(mProgram);
}

void Shader::Bind() {
    glUseProgram(mProgram);
}

void Shader::Unbind() {
    glUseProgram(0);
}

void Shader::UpdateVec3(const char* varName, vec3 vector) {
    int varLoc = glGetUniformLocation(mProgram, varName);
    Bind();
    glUniform3fv(varLoc, 1, &vector.v[0]);
}

void Shader::UpdateVec4(const char* varName, vec4 vector) {
    int varLoc = glGetUniformLocation(mProgram, varName);
    Bind();
    glUniform4fv(varLoc, 1, &vector.v[0]);
}

void Shader::UpdateMat4(const char* varName, mat4 matrix) {
    int varLoc = glGetUniformLocation(mProgram, varName);
    Bind();
    glUniformMatrix4fv(varLoc, 1, false, &matrix.v[0]);
}

void Shader::UpdateInt(const char* varName, int value) {
    int varLoc = glGetUniformLocation(mProgram, varName);
    Bind();
    glUniform1i(varLoc, value);
}

void Shader::UpdateIntArray(const char* varName, int size, int* array) {
    int varLoc = glGetUniformLocation(mProgram, varName);
    Bind();
    glUniform1iv(varLoc, size, array);
}

void Shader::UpdateMat4Array(const char* varName, int size, mat4* array) {
    int varLoc = glGetUniformLocation(mProgram, varName);
    Bind();
    glUniformMatrix4fv(varLoc, size, false, (float *)&array[0]);
}
