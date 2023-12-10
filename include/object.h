#ifndef OBJECT_H__
#define OBJECT_H__

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include "utils.h"
using namespace std;
#include <mtl.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vector>

#include "texture.h"

class scene;
class object {
   public:
    friend class scene;
    object();
    ~object();
    void set_color(float r, float g, float b);
    void set_shininess(float shininess) { Shininess = shininess; };
    void set_strength(float strength) { Strength = strength; };
    // void set_use_texture(bool Use_texture) { use_texture = use_texture; };
    void set_name(const string &objname);
    string get_name() const;
    void set_vertices(vector<GLfloat> v);
    // void set_faces_indexes(vector<GLuint> v);
    void add_faces_indexes_material(vector<GLuint> f, string material);
    void Model(glm::mat4 model_matrix);
    void push_right_matrix(glm::mat4 matrix);
    void push_left_matrix(glm::mat4 matrix);
    glm::mat4 get_Model_matrix();
    void set_wireframe(bool on_wireframe) { wireframe = on_wireframe; };

    void set_center(float x, float y, float z);
    float get_center_x() const;
    float get_center_y() const;
    float get_center_z() const;
    void set_mtl(mtl *mtllib);

   private:
    void render(GLint position, GLint normal, GLint texcoord, GLint color_u, GLint tex_comb_u);
    bool indexed;
    bool wireframe;
    bool no_normal;
    bool no_texture;
    float color[3];
    float Shininess;
    float Strength;
    float center[3];
    string name;
    glm::mat4 Model_matrix;

    vector<tuple<GLuint, string, int>> EBO_material_inumber;
    GLuint VBO;
    int v_number;

    mtl *material_lib;
};

object::~object() {
    glDeleteBuffers(1, &VBO);
    if (indexed) {
        for (auto [ebo, mat, inum] : EBO_material_inumber) {
            glDeleteBuffers(1, &ebo);
        }
    }
}

object::object() {
    indexed = false;
    wireframe = false;
    color[0] = 1.0;
    color[1] = color[2] = 0.0;
    Model_matrix = glm::mat4(1.0);
    center[0] = center[1] = center[2] = 0.0;
    no_texture = false;
    no_normal = false;
    Shininess = 50.0;
    Strength = 1.0;
};

void object::set_color(float r, float g, float b) {
    color[0] = r;
    color[1] = g;
    color[2] = b;
    return;
}

void object::Model(glm::mat4 model_matrix) {
    Model_matrix = model_matrix;
    return;
}

void object::push_right_matrix(glm::mat4 matrix) {
    Model_matrix = Model_matrix * matrix;
}
void object::push_left_matrix(glm::mat4 matrix) {
    Model_matrix = matrix * Model_matrix;
}

glm::mat4 object::get_Model_matrix() {
    return Model_matrix;
}

void object::render(GLint position, GLint normal, GLint texcoord, GLint color_u, GLint tex_comb_u) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glVertexAttribPointer(texcoord, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));

    for (auto [ebo, mat, inum] : EBO_material_inumber) {
        Texture *t = material_lib->get_texture(mat);
        if (t) {
            t->Bind(GL_TEXTURE0);
            glUniform1i(tex_comb_u, 2);
        } else {
            glUniform1i(tex_comb_u, 0);
        }

        color[0] = material_lib->get_color(mat)[0];
        color[1] = material_lib->get_color(mat)[1];
        color[2] = material_lib->get_color(mat)[2];

        // cout << "render " << name << endl;
        // cout << "material " << mat << endl;
        // cout << "color " << color[0] << " " << color[1] << " " << color[2] << endl;
        // cout << endl;

        glUniform3fv(color_u, 1, color);


        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glDrawElements(GL_TRIANGLES, inum, GL_UNSIGNED_INT, NULL);
    }
}

void object::set_mtl(mtl *mtllib) {
    material_lib = mtllib;
}

/*
    set_center: Define the object center
    @param x: center X coordinate
    @param y: center Y coordinate
    @param z: center Z coordinate
*/
void object::set_center(float x, float y, float z) {
    center[0] = x;
    center[1] = y;
    center[2] = z;
}

float object::get_center_x() const { return center[0]; }
float object::get_center_y() const { return center[1]; }
float object::get_center_z() const { return center[2]; }

void object::set_name(const string &objname) {
    name = objname;
}

string object::get_name() const {
    return name;
}

/*
    Create vertices buffer with vertexes
    @param v: Array of vertices. Each vertex is defined by 6 float point numbers.
        [Vx, Vy, Vz, VNx, VNy, VNz]
    @returns Nothing
*/
void object::set_vertices(vector<GLfloat> v) {
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v.size(), v.data(), GL_STATIC_DRAW);
    v_number = v.size() / 8;

    center[0] = center[1] = center[2] = 0.0;
    for (int i = 0; i < v.size(); i += 8)
        for (int j = 0; j < 3; ++j) center[j] += v[i + j] / v_number;
}

// /*
//     set_faces_indexes: Create a list with triangular faces
//     @param f: Array of vertices indexes to form faces
//     @param len: Size
//     @returns Nothing
// */
// void object::set_faces_indexes(vector<GLuint> f) {
//     glGenBuffers(1, &EBO);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * f.size(), f.data(), GL_STATIC_DRAW);
//     indexed = true;
//     i_number = f.size();
// }

/*
    set_faces_indexes: Create a list with triangular faces
    @param f: Array of vertices indexes to form faces
    @param material: material name
    @returns Nothing
*/
void object::add_faces_indexes_material(vector<GLuint> f, string material) {
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * f.size(), f.data(), GL_STATIC_DRAW);
    indexed = true;
    int inum = f.size();
    EBO_material_inumber.emplace_back(ebo, material, inum);
}

#endif