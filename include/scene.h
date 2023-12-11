#ifndef SCENE_H__
#define SCENE_H__

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "utils.h"
using namespace std;
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "object.h"
#include "shadow_map.h"

struct LightProperties {
    bool is_camera_coordinate;
    bool isEnabled;
    bool isLocal;
    bool isSpot;
    glm::vec3 ambient;
    glm::vec3 color;
    glm::vec3 position;
    glm::vec3 coneDirection;
    float spotCosCutoff;
    float spotExponent;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
};

class scene {
   public:
    scene();
    ~scene();
    void render();
    void set_color(float r, float g, float b);
    void Ortho3D(float WL, float WR, float WB, float WT, float zNear, float zFar);
    void perspective(float fovy, float aspect, float zNear, float zFar);
    void LookAt(float eyex, float eyey, float eyez,
                float centerx, float centery, float centerz,
                float upx, float upy, float upz);
    void Model(glm::mat4 model_matrix);
    void set_light(int light_number, LightProperties my_light);
    void push_back_object(object* new_object);
    void push_back_objects(vector<object*> new_objects);
    void set_Viewport(int X0, int Y0, int Width, int Height);

   private:
    GLuint ShaderProgram;
    glm::mat4 Projection_matrix;
    glm::mat4 View;
    vector<object*> my_objects;
    LightProperties lights[8];
    const int MaxLights = 8;
    int ViewportX0, ViewportY0, ViewportWidth, ViewportHeight;

    // shadow properties
    GLuint ShaderProgramShadow;
    glm::mat4 ProjectionMatrixShadow;
    glm::mat4 ViewMatrixShadow;
    ShadowMapFBO shadow_map;

    // render methods
    void render_shadow();
    void render_light();

    // focus
    glm::vec3 focus;
};

scene::~scene() {
    for (auto it = my_objects.begin(); it != my_objects.end(); ++it) {
        delete (*it);
    }
}

scene::scene() {
    const char* pVSFileName = "../src/shader.vs";
    const char* pFSFileName = "../src/shader.fs";
    ShaderProgram = CompileShaders(pVSFileName, pFSFileName);
    Projection_matrix = glm::mat4(1.0);
    View = glm::mat4(1.0);

    for (int i = 0; i < MaxLights; i++)
        lights[i].isEnabled = false;

    ViewportX0 = 0;
    ViewportY0 = 0;
    ViewportWidth = 800;
    ViewportHeight = 800;

    // shadow properties
    const char* pVSFileNameShadow = "../src/shader_shadow.vs";
    const char* pFSFileNameShadow = "../src/shader_shadow.fs";
    ShaderProgramShadow = CompileShaders(pVSFileNameShadow, pFSFileNameShadow);
}

void scene::push_back_object(object* new_object) {
    my_objects.push_back(new_object);
}

void scene::render() {
    render_shadow();
    render_light();
}

void scene::render_shadow() {

    shadow_map.BindForWriting(); 

    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(ShaderProgramShadow);

    GLint mvp_u = glGetUniformLocation(ShaderProgramShadow, "MVPMatrixShadow");
    
    GLint position = glGetAttribLocation(ShaderProgramShadow, "PositionShadow");
    glEnableVertexAttribArray(position);

    ViewMatrixShadow = glm::lookAt(lights[0].position, focus, glm::vec3(0.0, 1.0, 0.0));
    ProjectionMatrixShadow = glm::perspective(0.75, 1.0, 0.1, 200.0);

    for(object *obj : my_objects) {
        glUniformMatrix4fv(mvp_u, 1, GL_FALSE, glm::value_ptr(ProjectionMatrixShadow * ViewMatrixShadow * obj->Model_matrix));
        obj->render_position(position);
    }
}

void scene::render_light() {
    shadow_map.BindForReading(GL_TEXTURE1);

    // SalvarTexura2Image("texture.bmp", 800, 800, 1);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(ViewportX0, ViewportY0, ViewportWidth, ViewportHeight);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(ShaderProgram);

    GLint color_u = glGetUniformLocation(ShaderProgram, "Color");
    GLint mvp_u = glGetUniformLocation(ShaderProgram, "MVPMatrix");
    GLint light_mvp_u = glGetUniformLocation(ShaderProgram, "LightMVPMatrix");
    GLint mv_u = glGetUniformLocation(ShaderProgram, "MVMatrix");
    GLint normal_u = glGetUniformLocation(ShaderProgram, "NormalMatrix");

    GLint position = glGetAttribLocation(ShaderProgram, "Position");
    glEnableVertexAttribArray(position);

    GLint normal = glGetAttribLocation(ShaderProgram, "Normal");
    glEnableVertexAttribArray(normal);

    GLint texcoord = glGetAttribLocation(ShaderProgram, "TexCoord");
    glEnableVertexAttribArray(texcoord);

    // Light
    for (int i = 0; i < MaxLights; i++) {
        string light_name = "Lights[" + std::to_string(i) + "]";
        GLint isEnabled_u = glGetUniformLocation(ShaderProgram, (light_name + ".isEnabled").c_str());
        GLint isLocal_u = glGetUniformLocation(ShaderProgram, (light_name + ".isLocal").c_str());
        GLint isSpot_u = glGetUniformLocation(ShaderProgram, (light_name + ".isSpot").c_str());
        GLint ambient_u = glGetUniformLocation(ShaderProgram, (light_name + ".ambient").c_str());
        GLint lightcolor_u = glGetUniformLocation(ShaderProgram, (light_name + ".color").c_str());
        GLint lightposition_u = glGetUniformLocation(ShaderProgram, (light_name + ".position").c_str());

        GLint constantAttenuation_u = glGetUniformLocation(ShaderProgram, (light_name + ".constantAttenuation").c_str());
        GLint linearAttenuation_u = glGetUniformLocation(ShaderProgram, (light_name + ".linearAttenuation").c_str());
        GLint quadraticAttenuation_u = glGetUniformLocation(ShaderProgram, (light_name + ".quadraticAttenuation").c_str());

        glUniform1i(isEnabled_u, lights[i].isEnabled);
        glUniform1i(isLocal_u, lights[i].isLocal);
        glUniform1i(isSpot_u, lights[i].isSpot);
        glUniform3fv(ambient_u, 1, glm::value_ptr(lights[i].ambient));
        glUniform3fv(lightcolor_u, 1, glm::value_ptr(lights[i].color));

        // TODO: TEST is_camera_coordinate and isLocal
        if(lights[i].is_camera_coordinate) {
            glUniform3fv(lightposition_u, 1, glm::value_ptr(lights[i].position));
        } else {
            glUniform3fv(lightposition_u, 1, glm::value_ptr(
                    glm::vec3(
                        View * glm::vec4(lights[i].position, 1.0)
                    )
                )
            );
        }


        // TODO: INCLUDE OTHER Light Properties

        glUniform1f(constantAttenuation_u, lights[i].constantAttenuation);
        glUniform1f(linearAttenuation_u, lights[i].linearAttenuation);
        glUniform1f(quadraticAttenuation_u, lights[i].quadraticAttenuation);
    }

    // Material
    GLint shininess_u = glGetUniformLocation(ShaderProgram, "Shininess");
    GLint strength_u = glGetUniformLocation(ShaderProgram, "Strength");
    GLint texture_combiner_u = glGetUniformLocation(ShaderProgram, "Texture_combiner");

    // TEXTURE0 is difuse color
    GLint gSampler = glGetUniformLocation(ShaderProgram, "gSampler");
    glUniform1i(gSampler, 0);

    // TEXTURE1 is shadow
    GLint gSamplerShadowMap = glGetUniformLocation(ShaderProgram, "gSamplerShadowMap");
    glUniform1i(gSamplerShadowMap, 1);

    for (object *obj : my_objects) {
        glUniform1f(shininess_u, obj->Shininess);
        glUniform1f(strength_u, obj->Strength);

        glUniformMatrix4fv(mvp_u, 1, GL_FALSE, glm::value_ptr(Projection_matrix * View * (obj->Model_matrix)));
        glUniformMatrix4fv(mv_u, 1, GL_FALSE, glm::value_ptr(View * (obj->Model_matrix)));
        glUniformMatrix3fv(normal_u, 1, GL_FALSE, glm::value_ptr(glm::inverseTranspose(glm::mat3(View * (obj->Model_matrix)))));
        
        // light MVP matrix
        glUniformMatrix4fv(light_mvp_u, 1, GL_FALSE, glm::value_ptr(ProjectionMatrixShadow * ViewMatrixShadow * obj->Model_matrix));

        obj->render(position, normal, texcoord, color_u, texture_combiner_u);
    }
}

void scene::set_color(float r, float g, float b) {
    for (object *obj : my_objects) {
        obj->set_color(r, g, b);
    }
    return;
}

void scene::Ortho3D(float WL, float WR, float WB, float WT, float zNear, float zFar) {
    Projection_matrix = glm::ortho(
        WL, WR,
        WB, WT,
        zNear, zFar);

    return;
}

void scene::Model(glm::mat4 model_matrix) {
    for (object *obj : my_objects) {
        obj->push_left_matrix(model_matrix);
    }
    return;
}

void scene::LookAt(float eyex, float eyey, float eyez,
                   float centerx, float centery, float centerz,
                   float upx, float upy, float upz) {
    
    focus = glm::vec3(centerx, centery, centerz);

    View = glm::lookAt(
        glm::vec3(eyex, eyey, eyez),
        glm::vec3(centerx, centery, centerz),
        glm::vec3(upx, upy, upz));
}

void scene::perspective(float fovy, float aspect, float zNear, float zFar) {
    Projection_matrix = glm::perspective(fovy, aspect, zNear, zFar);
}

void scene::push_back_objects(vector<object*> new_objects) {
    for (auto it = new_objects.begin(); it != new_objects.end(); ++it) {
        my_objects.push_back(*it);
    }
}

void scene::set_light(int light_number, LightProperties my_light) {
    if (light_number >= MaxLights || light_number < 0)
        return;
    lights[light_number] = my_light;
    return;
}

void scene::set_Viewport(int X0, int Y0, int Width, int Height) {
    ViewportX0 = X0;
    ViewportY0 = Y0,
    ViewportWidth = Width;
    ViewportWidth = Height;
}

#endif