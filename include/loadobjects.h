#ifndef LOAD_OBJECTS_H
#define LOAD_OBJECTS_H

#include <object.h>
#include <mtl.h>

#include <sstream>
#include <string>
#include <vector>
using namespace std;

vector<object*> load_objects(string path, string filename) {
    ifstream file(path + "/" + filename);
    if (!file) {
        cerr << "Error opening file: " << path + "/" + filename << endl;
        throw ios_base::failure("Error opening file: " + path + "/" + filename);
    }
    // vertices, normals and texture from obj file
    vector<vector<GLfloat>> vertices;
    vector<vector<GLfloat>> normals;
    vector<vector<GLfloat>> texture;

    // each face has 3 triples (ver, nor, tex)
    map<string, int> vertices_ids;
    vector<GLfloat> obj_vertices;
    vector<GLuint> obj_faces_indexes;

    vector<object*> objs;

    // current material for
    string material = "";
    mtl *mtllib = nullptr;

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
            if (!objs.empty()) {
                objs.back()->set_vertices(obj_vertices);
                objs.back()->add_faces_indexes_material(obj_faces_indexes, material);
                objs.back()->set_mtl(mtllib);
            }

            // clear values
            obj_vertices.clear();
            obj_faces_indexes.clear();

            // create new object
            objs.push_back(new object());
            string name;
            iss >> name;
            objs.back()->set_name(name);
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
                    obj_vertices.push_back(texture[index_u - 1][0]);
                    obj_vertices.push_back(texture[index_u - 1][1]);
                }

                obj_faces_indexes.push_back(vertices_ids[input_vertex]);
            }
            // cout << endl;
        } else if (token == "mtllib") {
            string s;
            iss >> s;
            mtllib = new mtl(path, s);
        } else if(token == "usemtl") {
            // save current faces before next material
            objs.back()->add_faces_indexes_material(obj_faces_indexes, material);
            obj_faces_indexes.clear();

            if(iss >> material) {
                cout << "material: " << material << endl;
            } else {
                cout << "no material" << endl;
                material = "";
            }
        } else {
            // ignore others
        }
    }
    file.close();

    // save last
    if (objs.size()) {
        objs.back()->set_vertices(obj_vertices);
        objs.back()->add_faces_indexes_material(obj_faces_indexes, material);
        objs.back()->set_mtl(mtllib);
    }

    return objs;
}

#endif