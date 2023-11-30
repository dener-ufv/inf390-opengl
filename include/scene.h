#ifndef SCENE_H__
#define SCENE_H__

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "utils.h"
using namespace std;
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "object.h"

class scene {
   public:
    scene();
    ~scene();
    void render();
    void set_color(float r, float g, float b);
    void set_ambient(float r, float g, float b);
    void Ortho3D(float WL, float WR, float WB, float WT, float zNear, float zFar);
    void perspective(float fovy, float aspect, float zNear, float zFar);
    void LookAt(float eyex, float eyey, float eyez,
                float centerx, float centery, float centerz,
                float upx, float upy, float upz);
    void Model(glm::mat4 model_matrix);
    void push_back_object(object *new_object);
    void push_back_objects(vector<object *> new_objects);

   private:
    GLuint ShaderProgram;
    glm::mat4 Projection_matrix;
    glm::mat4 View;
    
    glm::vec3 LightAmbient;
    glm::vec3 LightColor;
    glm::vec3 LightPosition;
    float Shininess;
    float Strength;

    vector<object *> my_objects;
};
scene::~scene() {
    for (auto it = my_objects.begin(); it != my_objects.end(); ++it) {
        delete (*it);
    }
}

scene::scene() {
    const char *pVSFileName = "../src/shader.vs";
    const char *pFSFileName = "../src/shader.fs";
    ShaderProgram = CompileShaders(pVSFileName, pFSFileName);
    Projection_matrix = glm::mat4(1.0);
    View = glm::mat4(1.0);

    LightAmbient = glm::vec3(0.3, 0.3, 0.3);
    LightColor = glm::vec3(1.0, 1.0, 1.0);
    LightPosition = glm::vec3(0.0, 10.0, 5.0);
    Shininess = 50.0;
    Strength = 1.0;
}

void scene::push_back_object(object *new_object) {
    my_objects.push_back(new_object);
}

void scene::render() {
    glUseProgram(ShaderProgram);
    GLint position = glGetAttribLocation(ShaderProgram, "Position");
    GLint normal = glGetAttribLocation(ShaderProgram, "Normal");
    GLint texture = glGetAttribLocation(ShaderProgram, "Texture");
    GLint color_u = glGetUniformLocation(ShaderProgram, "Color");

    GLint mvp_matrix_u = glGetUniformLocation(ShaderProgram, "MVPMatrix");
    GLint mv_matrix_u = glGetUniformLocation(ShaderProgram, "MVMatrix");
    GLint normal_matrix_u = glGetUniformLocation(ShaderProgram, "NormalMatrix");

    GLint ambient_u = glGetUniformLocation(ShaderProgram, "Ambient");
    GLint light_color_u = glGetUniformLocation(ShaderProgram, "LightColor");
    GLint light_position_u = glGetUniformLocation(ShaderProgram, "LightPosition");
    GLint shininess_u = glGetUniformLocation(ShaderProgram, "Shininess");
    GLint strength_u = glGetUniformLocation(ShaderProgram, "Strength");
    
    glEnableVertexAttribArray(position);
    glEnableVertexAttribArray(normal);
    glEnableVertexAttribArray(texture);

    glUniform3fv(ambient_u, 1, glm::value_ptr(LightAmbient));
    glUniform3fv(light_color_u, 1, glm::value_ptr(LightColor));

    glUniform3fv(light_position_u, 1, glm::value_ptr(
            glm::vec3(
                View * glm::vec4(LightPosition, 1.0)
            )
        )
    );
    glUniform1f(shininess_u, Shininess);
    glUniform1f(strength_u, Strength);

    for (auto it = my_objects.begin(); it != my_objects.end(); ++it) {
        // color
        glUniform3fv(color_u, 1, (**it).color);
        
        // mvp matrix
        glUniformMatrix4fv(mvp_matrix_u, 1, GL_FALSE, glm::value_ptr(
            Projection_matrix * View * (**it).Model_matrix));
        
        // mv matrix
        glUniformMatrix4fv(mv_matrix_u, 1, GL_FALSE, glm::value_ptr(
            View * (**it).Model_matrix));

        // normal matrix
        glUniformMatrix3fv(normal_matrix_u, 1, GL_FALSE, glm::value_ptr(
            glm::inverseTranspose(glm::mat3(View * (**it).Model_matrix))
        ));

        (**it).render(position, normal, texture);
    }
}

void scene::set_color(float r, float g, float b) {
    for (auto it = my_objects.begin(); it != my_objects.end(); ++it) {
        (**it).set_color(r, g, b);
    }
    return;
}

void scene::set_ambient(float r, float g, float b) {
    LightAmbient = glm::vec3(r, g, b);
}

void scene::Ortho3D(float WL, float WR, float WB, float WT, float zNear, float zFar) {
    Projection_matrix = glm::ortho(
        WL, WR,
        WB, WT,
        zNear, zFar);

    return;
}

void scene::Model(glm::mat4 model_matrix) {
    for (auto it = my_objects.begin(); it != my_objects.end(); ++it) {
        (**it).push_left_matrix(model_matrix);
    }
    return;
}

void scene::LookAt(float eyex, float eyey, float eyez,
                   float centerx, float centery, float centerz,
                   float upx, float upy, float upz) {
    View = glm::lookAt(
        glm::vec3(eyex, eyey, eyez),
        glm::vec3(centerx, centery, centerz),
        glm::vec3(upx, upy, upz));
}

void scene::perspective(float fovy, float aspect, float zNear, float zFar) {
    Projection_matrix = glm::perspective(fovy, aspect, zNear, zFar);
}

void scene::push_back_objects(vector<object *> new_objects) {
    for (auto it = new_objects.begin(); it != new_objects.end(); ++it) {
        my_objects.push_back(*it);
    }
}

#endif