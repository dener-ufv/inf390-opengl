#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include "object.h"

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <iostream>

using namespace std;

#define MAX_TURN_ANGLE      32.0f
#define MIN_TURN_ANGLE      -32.0f
#define WEEL_DIAMETER       0.396

class Car : public vector<object*> {
    object *rodaFD, *rodaFE, *rodaTD, *rodaTE;
    vector<object *> fixed;  // chassi, vidro, portas, etc...
    
    float turnAngle;    // how the front weel is turned
    float weelAngle;  // how is the weels are rotated

    float speed;

    glm::vec3 backCenter;
    glm::vec3 frontCenter;

    glm::mat4 carModel;

public:
    Car();
    Car(string path);
    void set_speed(float _speed);
    void set_turn(float _angle);
    void update();
    glm::mat4 get_model() const;

private:
    void load_car(string path);
    void save_obj(object *obj, vector<GLfloat> vert, vector<GLuint> faces_indexes);
    void update_model();
    void update_centers();
};

Car::Car() {
    // empty
}

Car::Car(string path) {
    carModel = glm::mat4(1.0);

    load_car(path);
    turnAngle = 0.0;
    weelAngle = 0.0;
    speed = 0.0;

    update_model();
    update_centers();
}

/*
    set_speed: sets the car speed using weel rotations in degrees per tick, 
        where tick is the amount of time between car updates
    @param _speed: the car speed
    @return nothing
*/
void Car::set_speed(float _speed) {
    speed = _speed;
}

void Car::set_turn(float _angle) {
    turnAngle = min(MAX_TURN_ANGLE, max(MIN_TURN_ANGLE, _angle));
}

void Car::update() {
    weelAngle += speed;
    if(weelAngle >= 360) weelAngle -= 360.0; 
    if(weelAngle < 0) weelAngle += 360.0;

    float alpha = glm::radians(turnAngle);
    float h = glm::distance(frontCenter, backCenter);
    float d = (WEEL_DIAMETER / 2.0) * glm::radians(speed);
    float theta = alpha - glm::asin(h * glm::sin(alpha) / (h + d));

    glm::vec3 curFrontCenterPos = glm::vec3(carModel * glm::vec4(frontCenter, 1.0));
    glm::vec3 curBackCenterPos = glm::vec3(carModel * glm::vec4(backCenter, 1.0));

    glm::vec3 carDir = curFrontCenterPos - curBackCenterPos;
    carDir[1] = 0.0; // make sure that there is no vertical component

    carModel = 
        glm::translate(
            glm::translate(
                glm::rotate(
                    glm::translate(
                        glm::mat4(1.0),
                        curBackCenterPos
                    ),
                    theta,
                    glm::vec3(0.0, 1.0, 0.0)
                ),
                d * glm::normalize(carDir)
            ),
            -curBackCenterPos
        ) * carModel;

    update_model();
}

glm::mat4 Car::get_model() const {
    return carModel;
}

void Car::load_car(string path) {
    ifstream file(path);
    if (!file) {
        cerr << "Error opening file: " << path << endl;
        throw ios_base::failure("Error opening file: " + path);
    }
    // vertices, normals and texture from obj file
    vector<vector<GLfloat>> vertices;
    vector<vector<GLfloat>> normals;
    vector<vector<GLfloat>> texture;

    // each face has 3 triples (ver, nor, tex)
    map<string, int> vertices_ids;
    vector<GLfloat> obj_vertices;
    vector<GLuint> obj_faces_indexes;

    object *cur_obj = nullptr;

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        if (line[0] == '#') {
            // cout << "comment: " << line << endl;
            continue;  // comment
        }
        string token;
        iss >> token;
        if (token == "o") {
            // save current
            if (cur_obj != nullptr) save_obj(cur_obj, obj_vertices, obj_faces_indexes);
            
            // clear values
            obj_vertices.clear();
            obj_faces_indexes.clear();
            
            // create new object
            cur_obj = new object();
            string name;
            iss >> name;
            cur_obj->set_name(name);
        } else if (token == "v") {
            float x, y, z;
            iss >> x >> y >> z;
            vertices.push_back({x, y, z});
            // cout << "v " << x << " " << y << z << endl;
        } else if (token == "vn") {
            float nx, ny, nz;
            iss >> nx >> ny >> nz;
            normals.push_back({nx, ny, nz});
            // cout << "vn " << nx << " " << ny << nz << endl;
        } else if (token == "vt") {
            float u, v;
            iss >> u >> v;
            texture.push_back({u, v});
            // cout << "vt " << u << " " << v << endl;
        } else if (token == "f") {
            // cout << "f ";
            for (int i = 0; i < 3; ++i) {
                string input_vertex;
                iss >> input_vertex;
                // cout << input_vertex << " ";
                istringstream vss(input_vertex);
                uint32_t index_v, index_n, index_u;
                vss >> index_v;
                if (vss.peek() == '/') {
                    vss.ignore();
                    if (vss.peek() != '/') {
                        vss >> index_u;
                    }
                    if (vss.peek() == '/') {
                        vss.ignore();
                        vss >> index_n;
                    }
                }

                if (!vertices_ids.count(input_vertex)) {
                    vertices_ids[input_vertex] = obj_vertices.size() / 8;
                    obj_vertices.push_back(vertices[index_v - 1][0]);
                    obj_vertices.push_back(vertices[index_v - 1][1]);
                    obj_vertices.push_back(vertices[index_v - 1][2]);
                    obj_vertices.push_back(normals[index_n - 1][0]);
                    obj_vertices.push_back(normals[index_n - 1][1]);
                    obj_vertices.push_back(normals[index_n - 1][2]);
                    obj_vertices.push_back(texture[index_u-1][0]);
                    obj_vertices.push_back(texture[index_u-1][1]);
                }

                obj_faces_indexes.push_back(vertices_ids[input_vertex]);
            }
            // cout << endl;
        } else if (token == "l") {
            uint32_t a, b;
            iss >> a >> b;
            // cout << "l " << a << " " << b << endl;
        } else if (token == "s") {
            int a;
            iss >> a;
            // cout << "s " << a << endl;
        }
    }
    file.close();
    // save last
    if (cur_obj != nullptr) save_obj(cur_obj, obj_vertices, obj_faces_indexes);
}

void Car::save_obj(object *obj, vector<GLfloat> vert, vector<GLuint> faces_indexes) {
    obj->set_faces_indexes(faces_indexes);
    obj->set_vertices(vert);
                
    string n = obj->get_name();
    if(n == "roda_FD") {
        rodaFD = obj;
    } else if(n == "roda_FE") {
        rodaFE = obj;
    } else if(n == "roda_TD") {
        rodaTD = obj;
    } else if(n == "roda_TE") {
        rodaTE = obj;
    } else {
        fixed.push_back(obj);
    }

    this->push_back(obj);
}

void Car::update_model() {
    // model roda Frente Direita
    glm::mat4 modelRodaFD = 
        glm::translate(
            glm::rotate(
                glm::rotate(
                    glm::translate(
                        glm::mat4(1.0),
                        glm::vec3(rodaFD->get_center_x(), rodaFD->get_center_y(), rodaFD->get_center_z())
                    ),
                    glm::radians(turnAngle),
                    glm::vec3(0.0, 1.0, 0.0)
                ),
                glm::radians(weelAngle),
                glm::vec3(1.0, 0.0, 0.0)
            ),
            glm::vec3(-rodaFD->get_center_x(), -rodaFD->get_center_y(), -rodaFD->get_center_z())
        );
    rodaFD->Model(carModel * modelRodaFD);

    // model roda Frente Esquerda
    glm::mat4 modelRodaFE = 
        glm::translate(
            glm::rotate(
                glm::rotate(
                    glm::translate(
                        glm::mat4(1.0),
                        glm::vec3(rodaFE->get_center_x(), rodaFE->get_center_y(), rodaFE->get_center_z())
                    ),
                    glm::radians(turnAngle),
                    glm::vec3(0.0, 1.0, 0.0)
                ),
                glm::radians(weelAngle),
                glm::vec3(1.0, 0.0, 0.0)
            ),
            glm::vec3(-rodaFE->get_center_x(), -rodaFE->get_center_y(), -rodaFE->get_center_z())
        );
    rodaFE->Model(carModel * modelRodaFE);

    // model roda Tras Direita
    glm::mat4 modelRodaTD = 
        glm::translate(
            glm::rotate(
                glm::translate(
                    glm::mat4(1.0),
                    glm::vec3(rodaTD->get_center_x(), rodaTD->get_center_y(), rodaTD->get_center_z())
                ),
                glm::radians(weelAngle),
                glm::vec3(1.0, 0.0, 0.0)
            ),
            glm::vec3(-rodaTD->get_center_x(), -rodaTD->get_center_y(), -rodaTD->get_center_z())
        );
    rodaTD->Model(carModel * modelRodaTD);

    // model roda Tras Esquerda
    glm::mat4 modelRodaTE = 
        glm::translate(
            glm::rotate(
                glm::translate(
                    glm::mat4(1.0),
                    glm::vec3(rodaTE->get_center_x(), rodaTE->get_center_y(), rodaTE->get_center_z())
                ),
                glm::radians(weelAngle),
                glm::vec3(1.0, 0.0, 0.0)
            ),
            glm::vec3(-rodaTE->get_center_x(), -rodaTE->get_center_y(), -rodaTE->get_center_z())
        );
    rodaTE->Model(carModel * modelRodaTE);

    // update fixed parts
    for(object *obj : fixed) obj->Model(carModel);
}

void Car::update_centers() {
    backCenter[0] = ( rodaTE->get_center_x() + rodaTD->get_center_x() ) / 2.0; 
    backCenter[1] = ( rodaTE->get_center_y() + rodaTD->get_center_y() ) / 2.0; 
    backCenter[2] = ( rodaTE->get_center_z() + rodaTD->get_center_z() ) / 2.0; 

    frontCenter[0] = ( rodaFE->get_center_x() + rodaFD->get_center_x() ) / 2.0; 
    frontCenter[1] = ( rodaFE->get_center_y() + rodaFD->get_center_y() ) / 2.0; 
    frontCenter[2] = ( rodaFE->get_center_z() + rodaFD->get_center_z() ) / 2.0;
}