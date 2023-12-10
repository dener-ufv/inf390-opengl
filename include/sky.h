#ifndef SKY_H
#define SKY_H

#include <vector>
#include <string>
#include <map>
#include <sstream>

#include "object.h"
#include <loadobjects.h>
using namespace std;

class Sky : public vector<object*> {
   public:
   Sky();
   Sky(string path, string filename);
};

Sky::Sky() {
    // empty
}

Sky::Sky(string path, string filename) {
    vector<object*> objs = load_objects(path, filename);
    this->insert(this->end(), objs.begin(), objs.end());
}

#endif