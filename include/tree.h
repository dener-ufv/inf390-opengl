#ifndef TREE_H
#define TREE_H

#include <loadobjects.h>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <iostream>

#include <random>
#include <chrono>

#include <map>

#include "object.h"
using namespace std;

mt19937 rng(chrono::steady_clock::now().time_since_epoch().count());

class Tree : public vector<object*> {
   public:
    Tree();
    Tree(object* o);

   private:
    glm::mat4 model;
};

Tree::Tree() {
    // empty
}

Tree::Tree(object *o) {

    object *my_obj = new object(*o);

    float r_angle = rng() / (float) rng.max();
    float angle = r_angle * 2 * glm::pi<float>();

    float r_tx = rng() / (float) rng.max();
    float r_tz = rng() / (float) rng.max();

    float tx = r_tx * 60 - 30 - o->get_center_x();
    float tz = r_tz * 60 - 30 - o->get_center_z();

    float r_scale = rng() / (float) rng.max();
    float scale = r_scale;

    cout << tx << " " << tz << " " << scale << endl;

    model = glm::scale(
        glm::rotate(
            glm::translate(
                glm::mat4(1.0),
                glm::vec3(tx, 0.0, tz)
            ),
            angle,
            glm::vec3(0.0, 1.0, 0.0)
        ),
        glm::vec3(scale, scale, scale)
    );

    my_obj->Model(model);
    this->push_back(my_obj);
}


map<string, object*> load_trees_obj(string dir, string filename) {
    map<string, object*> trs;
    vector<object*> objs = load_objects(dir, filename);

    for(object *o : objs) {
        trs[o->get_name()] = o;
    }

    return trs;
}

#endif