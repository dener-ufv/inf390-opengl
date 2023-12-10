#ifndef GROUND_H
#define GROUND_H

#include <vector>
#include <string>
#include <map>
#include <sstream>

#include "object.h"
#include <loadobjects.h>
using namespace std;

class Ground : public vector<object*> {
   public:
   Ground();
   Ground(string path, string filename);
};

Ground::Ground() {
    // empty
}

Ground::Ground(string path, string filename) {
    vector<object*> objs = load_objects(path, filename);
    this->insert(this->end(), objs.begin(), objs.end());
}

#endif