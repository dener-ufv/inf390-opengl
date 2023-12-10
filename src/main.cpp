#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <car.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include "object.h"
#include "scene.h"

#include <ground.h>
#include <sky.h>
#include <loadobjects.h>
#include <tree.h>

using namespace std;

bool ortho_per = true;
GLint width = 800, height = 800;

int worldx = 0, worldy = 0, worldz = 0;

glm::vec3 base_focus(0.0, 0.0, 0.0);
glm::vec3 base_camera(0.0, 5.0, -7.0);

glm::vec3 camera(0.0, 5.0, -7.0);

Car carro;
float cspeed = 0;
float cturn = 0;

Ground pista;
Sky ceu;
vector<Tree> arvores(10);

void set_light_default(LightProperties& light) {
    light.is_camera_coordinate = true;
    light.isEnabled = true;
    light.isLocal = true;
    light.isSpot = false;

    light.ambient[0] = light.ambient[1] = light.ambient[2] = 0.3;
    light.color[0] = light.color[1] = light.color[2] = 1.0;
    light.position[0] = 0.0;
    light.position[1] = 10.0;
    light.position[2] = 0.0;

    light.constantAttenuation = 1.0;
    light.linearAttenuation = 0.0;
    light.quadraticAttenuation = 0.0;

    light.is_camera_coordinate = false;
}

static void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        ortho_per = true;
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        ortho_per = false;
    if (action != GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_W:
                cspeed = min(1.5f, cspeed + 0.5f);
                carro.set_speed(cspeed);
                cout << "forward" << endl;
                break;

            case GLFW_KEY_S:
                cspeed = max(-0.5f, cspeed - 0.5f);
                carro.set_speed(cspeed);
                cout << "backward" << endl;
                break;

            case GLFW_KEY_A:
                cturn = min(32.0f, cturn + 8.0f);
                carro.set_turn(cturn);
                cout << "left" << endl;
                break;

            case GLFW_KEY_D:
                cturn = max(-32.0f, cturn - 8.0f);
                carro.set_turn(cturn);
                cout << "right" << endl;
                break;

            case GLFW_KEY_Z:
                if(mods == GLFW_MOD_SHIFT) {
                    base_camera *= 0.99f;
                } else {
                    base_camera *= 1.01f;
                }
                break;

            case GLFW_KEY_UP:
                base_focus[1] += 0.05;
                break;

            case GLFW_KEY_DOWN:
                base_focus[1] -= 0.05;
                break;

            default:
                break;
        }
    }
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        // color[(int)(xpos/(width/4))][3 - (int)(ypos/(height/4))] = !(color[(int)(xpos/(width/4))][3 - (int)(ypos/(height/4))]);
    }
}

int main(void) {
    GLFWwindow* window;
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(width, height, "Simple example", NULL, NULL);
    if (!window) {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    scene my_scene;

    LightProperties light0, light1;
    set_light_default(light0);

    my_scene.set_light(0, light0);

    carro = Car("../models/carro", "carro.obj");
    pista = Ground("../models/pista", "terreno.obj");
    ceu = Sky("../models/ceu", "esfera.obj");
    
    map<string, object*> trees = load_trees_obj("../models/tree", "trees9.obj");

    for(Tree &t : arvores) {
        t = Tree(trees["Mossy_Tr"]);
        my_scene.push_back_objects(t);
    }

    my_scene.push_back_objects(carro);
    my_scene.push_back_objects(pista);
    my_scene.push_back_objects(ceu);


    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    my_scene.set_Viewport(0, 0, width, height);

    while (!glfwWindowShouldClose(window)) {
        if (ortho_per) {
            my_scene.perspective(0.75, 1.0, 0.1, 200.0);
        } else {
            my_scene.Ortho3D(-2.0, 2.0, -2.0, 2.0, 0.0, 200.0);
        }


        carro.update();

        // camera low pass filter
        glm::vec3 goal_focus = glm::vec3(carro.get_model() * glm::vec4(base_focus, 1.0));
        glm::vec3 goal_camera = glm::vec3(carro.get_model() * glm::vec4(base_camera, 1.0));
        camera = camera * 0.999f + goal_camera * 0.001f;

        my_scene.LookAt(camera[0], camera[1], camera[2], goal_focus[0], goal_focus[1], goal_focus[2], 0.0, 1.0, 0.0);
        
        my_scene.render();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}