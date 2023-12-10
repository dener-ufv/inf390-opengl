#ifndef MTL_H
#define MTL_H


#include <fstream>
#include <sstream>
#include <map>
#include <utility>
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <texture.h>
using namespace std;

const glm::vec3 defaultColor(1.0, 0.0, 0.0);

class mtl {
    map<string, Texture*> mp_texture;
    map<string, glm::vec3> mp_color;

    public:
    mtl(string path, string filename);
    Texture* get_texture(string material);
    glm::vec3 get_color(string material);
};

mtl::mtl(string path, string filename) {
    ifstream file(path + "/" + filename);
    if (!file) {
        cerr << "Error opening file: " << path + "/" + filename << endl;
        throw ios_base::failure("Error opening file: " + path + "/" + filename);
    }

    string line;
    string name;
    while(getline(file, line)) {
        istringstream iss(line);

        string token;
        iss >> token;
        if(token == "newmtl") {
            iss >> name;
            mp_texture[name] = nullptr;
            mp_color[name] = defaultColor;
        } else if(token == "Kd") {
            float r, g, b;
            iss >> r >> g >> b;
            mp_color[name] = glm::vec3(r, g, b);
        } else if(token == "map_Kd") {
            string s;
            iss >> s;
            mp_texture[name] = new Texture(GL_TEXTURE_2D, path + "/" + s);
            mp_texture[name]->load_image();
        } else {
            // ignore others
        }
    }
}

Texture *mtl::get_texture(string material) {
    return mp_texture[material];
}

glm::vec3 mtl::get_color(string material) {
    if(mp_color.count(material)) return mp_color[material];
    return defaultColor;
}

#endif