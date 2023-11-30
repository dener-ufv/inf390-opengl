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
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <vector>

class scene;
class object {
   public:
    friend class scene;
    object();
    // object(int v_number, GLfloat *Vertices);
    // object(int v_number, int i_number, GLfloat *Vertices, GLushort *index);
    ~object();
    void Model(glm::mat4 model_matrix);
    void push_right_matrix(glm::mat4 matrix);
    void push_left_matrix(glm::mat4 matrix);
    glm::mat4 get_Model_matrix();
    friend class scene;

    void set_wireframe(bool on_wireframe) { wireframe = on_wireframe; };
    void set_color(float r, float g, float b);
    void set_name(const string &objname);
    string get_name() const;
    void set_vertices(vector<GLfloat> v);
    void set_faces_indexes(vector<GLuint> v);
    void set_center(float x, float y, float z);

    float get_center_x() const;
    float get_center_y() const;
    float get_center_z() const;

   private:
    void render(GLint position, GLint normal, GLint texture);
    bool indexed;
    bool wireframe;
    float color[3];
    glm::mat4 Model_matrix;
    GLuint VBO;
    GLuint EBO;
    int v_number;
    int i_number;
    string name;
    GLfloat center[3];
};

object::~object() {
    glDeleteBuffers(1, &VBO);
    if (indexed)
        glDeleteBuffers(1, &EBO);
}

object::object() {
    indexed = false;
    wireframe = false;
    color[0] = 1.0;
    color[1] = color[2] = 0.0;
    Model_matrix = glm::mat4(1.0);
    center[0] = center[1] = center[2] = 0.0;
};

// object::object(int v_number, GLfloat *Vertices) : v_number(v_number) {
//     indexed = false;
//     wireframe = false;
//     glGenBuffers(1, &VBO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v_number * 6, Vertices, GL_STATIC_DRAW);
//     color[0] = 1.0;
//     color[1] = color[2] = 0.0;
//     Model_matrix = glm::mat4(1.0);
// };

// object::object(int v_number, int i_number, GLfloat *Vertices, GLushort *index) : v_number(v_number), i_number(i_number) {
//     indexed = true;
//     wireframe = false;
//     glGenBuffers(1, &VBO);
//     glBindBuffer(GL_ARRAY_BUFFER, VBO);
//     glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * v_number * 6, Vertices, GL_STATIC_DRAW);
//     glGenBuffers(1, &EBO);
//     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
//     glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * i_number, index, GL_STATIC_DRAW);
//     color[0] = 1.0;
//     color[1] = color[2] = 0.0;
//     Model_matrix = glm::mat4(1.0);
// }

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

void object::render(GLint position, GLint normal, GLint texture) {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(position, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
    glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(texture, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));

    if (!wireframe) {
        if (!(indexed)) {
            glDrawArrays(GL_TRIANGLES, 0, v_number);
        } else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glDrawElements(GL_TRIANGLES, i_number, GL_UNSIGNED_INT, NULL);
        }
    } else {
        glPolygonMode(GL_FRONT, GL_LINE);
        glPolygonMode(GL_BACK, GL_LINE);
        if (!(indexed)) {
            glDrawArrays(GL_TRIANGLES, 0, v_number);
        } else {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glDrawElements(GL_TRIANGLES, i_number, GL_UNSIGNED_INT, NULL);
        }
        glPolygonMode(GL_FRONT, GL_FILL);
        glPolygonMode(GL_BACK, GL_FILL);
    }
}

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
    for(int i=0; i<v.size(); i += 8) for(int j=0; j<3; ++j) center[j] += v[i+j] / v_number;
}


/*
    set_faces_indexes: Create a list with triangular faces
    @param f: Array of vertices indexes to form faces
    @param len: Size
    @returns Nothing
*/
void object::set_faces_indexes(vector<GLuint> f) {
    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * f.size(), f.data(), GL_STATIC_DRAW);
    indexed = true;
    i_number = f.size();
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

float object::get_center_x() const {return center[0];}
float object::get_center_y() const {return center[1];}
float object::get_center_z() const {return center[2];}

#endif