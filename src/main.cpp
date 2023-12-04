#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <fstream>
#include <iostream>

#include "object.h"
#include "scene.h"

#include <car.h>
#include <ground.h>


using namespace std;

bool ortho_per = true;
GLint width = 800, height = 800;

bool camera_fixed = false;

glm::vec3 cam(0.0, 5.0, -7.0);
glm::vec3 center(0.0, 0.0, 0.0);

// car info
Car carro;
float curSpeed = 0.0;
float curTurn = 0.0;

// terrain info
Ground terreno;

static void error_callback(int error, const char *description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (key == GLFW_KEY_P && action == GLFW_PRESS)
        ortho_per = true;
    if (key == GLFW_KEY_O && action == GLFW_PRESS)
        ortho_per = false;

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_A:
                cout << "left" << endl;
                curTurn = min(32.0f, curTurn + 8.0f);
                carro.set_turn(curTurn);
                break;

            case GLFW_KEY_D:
                cout << "right" << endl;
                curTurn = max(-32.0f, curTurn - 8.0f);
                carro.set_turn(curTurn);
                break;

            case GLFW_KEY_W:
                cout << "up" << endl;
                curSpeed = min(0.75f, curSpeed + 0.25f);
                carro.set_speed(curSpeed);
                break;

            case GLFW_KEY_S:
                cout << "down" << endl;
                curSpeed = max(-0.25f, curSpeed - 0.25f);
                carro.set_speed(curSpeed);
                break;
            
            case GLFW_KEY_F:
                if(mods == GLFW_MOD_SHIFT) camera_fixed = true;
                else camera_fixed = false;
                break;

            default:
                break;
        }
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        // color[(int)(xpos/(width/4))][3 - (int)(ypos/(height/4))] = !(color[(int)(xpos/(width/4))][3 - (int)(ypos/(height/4))]);
    }
}

int main(void) {
    GLFWwindow *window;
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
    carro = Car("../models/carro.obj");
    terreno = Ground("../models/terreno.obj");

    my_scene.push_back_objects(carro);
    my_scene.push_back_objects(terreno);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glViewport(0, 0, width, height);
    

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (ortho_per) {
            my_scene.perspective(0.75, 1.0, 0.1, 200.0);
        } else {
            my_scene.Ortho3D(-2.0, 2.0, -2.0, 2.0, 0.0, 200.0);
        }

        carro.update();

        // update center
        center = glm::vec3(carro.get_model() * glm::vec4(0.0, 0.0, 0.0, 1.0));

        // low pass filter in cam
        glm::vec3 goal_cam = glm::vec3(carro.get_model() * glm::vec4(0.0, 5.0, -7.0, 1.0));
        cam = goal_cam * 0.001f + cam * 0.999f;

        if(camera_fixed)
            my_scene.LookAt(0.0, 5.0, -10.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
        else
            my_scene.LookAt(cam[0], cam[1], cam[2], center[0], center[1], center[2], 0.0, 1.0, 0.0);
        
        my_scene.render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}