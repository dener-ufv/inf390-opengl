#ifndef WATCH_TOWER_H
#define WATCH_TOWER_H

#include <vector>
#include <string>
#include <map>
#include <sstream>

#include "object.h"
#include <loadobjects.h>
#include <utils.h>
using namespace std;

class WatchTower : public vector<object*> {
   public:
   WatchTower();
   WatchTower(string path, string filename);
};

WatchTower::WatchTower() {
    // empty
}

WatchTower::WatchTower(string path, string filename) {
    vector<object*> objs = load_objects(path, filename);

    float r_tx = randomFloat();
    float r_tz = randomFloat();

    float tx = r_tx * 140 - 70;
    float tz = r_tz * 140 - 70;

    glm::mat4 model = glm::scale(
        glm::translate(
            glm::mat4(1.0),
            glm::vec3(tx, 0.0, tz)
        ),
        glm::vec3(0.3, 0.3, 0.3)
    );

    for(object *o : objs) {
        o->Model(model);
    }

    this->insert(this->end(), objs.begin(), objs.end());
}

#endif