#ifndef CAR_H
#define CAR_H 

#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

#include "object.h"
#include <loadobjects.h>

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
    Car(string path, string filename);
    void set_speed(float _speed);
    void set_turn(float _angle);
    void update();
    glm::mat4 get_model() const;

private:
    void update_model();
    void update_centers();
};

Car::Car() {
    // empty
}

Car::Car(string path, string filename) {
    carModel = glm::mat4(1.0);

    vector<object*> objs = load_objects(path, filename);

    this->insert(this->end(), objs.begin(), objs.end());

    for(object *o : objs) {
        string n = o->get_name();
        if(n == "roda_FD") {
            rodaFD = o;
        } else if(n == "roda_FE") {
            rodaFE = o;
        } else if(n == "roda_TD") {
            rodaTD = o;
        } else if(n == "roda_TE") {
            rodaTE = o;
        } else {
            fixed.push_back(o);
        }
    }


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

#endif