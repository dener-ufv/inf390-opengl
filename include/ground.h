#include <vector>
#include <string>
#include <map>
#include <sstream>

#include "object.h"
using namespace std;

class Ground : public vector<object*> {
   public:
   Ground();
   Ground(string path);

   private:
   void load_ground(string path);
   void save_obj(object *obj, vector<GLfloat> vert, vector<GLuint> faces_indexes);
};

Ground::Ground() {
    // empty
}

Ground::Ground(string path) {
    load_ground(path);
}

void Ground::load_ground(string path) {
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

void Ground::save_obj(object *obj, vector<GLfloat> vert, vector<GLuint> faces_indexes) {
    obj->set_faces_indexes(faces_indexes);
    obj->set_vertices(vert);
    obj->set_color(0.5, 0.5, 0.5);
    // obj->load_texture("../imgs/pista.jpg");
    this->push_back(obj);
}