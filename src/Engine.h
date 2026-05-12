#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "Simulation.h"
#include "Common.h"
#include <glm/gtc/constants.hpp>
using namespace std;


struct Camera {
    glm::vec3 target = glm::vec3(0.0f);
    float radius = 50.0f;
    float azimuth = 0.0f;
    float elevation = glm::pi<float>() / 2.0f;

    float orbitSpeed = 0.01f;
    float zoomSpeed = 10.0f;

    bool dragging = false;
    double lastX = 0, lastY = 0;

    glm::vec3 position() const {
        float e = CLAMP(elevation, 0.01f, (float)M_PI - 0.01f);        return glm::vec3(
            radius * sin(e) * cos(azimuth),
            radius * cos(e),
            radius * sin(e) * sin(azimuth)
        );
    }
    void update() {
    target = vec3(0.0f, 0.0f, 0.0f);
}

    void processMouseMove(double x, double y) {
        float dx = float(x - lastX);
        float dy = float(y - lastY);
        if (dragging) {
            azimuth += dx * orbitSpeed;
            elevation -= dy * orbitSpeed;
            elevation = CLAMP(elevation, 0.01f, (float)M_PI - 0.01f);
        }
        lastX = x;
        lastY = y;
        update();
    }
    void processMouseButton(int button, int action, int mods, GLFWwindow* win) {
        if (button == GLFW_MOUSE_BUTTON_LEFT || button == GLFW_MOUSE_BUTTON_MIDDLE) {
            if (action == GLFW_PRESS) {
                dragging = true;
                glfwGetCursorPos(win, &lastX, &lastY);
            } else if (action == GLFW_RELEASE) {
                dragging = false;
            }
        }
    }
    void processScroll(double xoffset, double yoffset) {
        radius -= yoffset * zoomSpeed;
        if (radius < 1.0f) radius = 1.0f;
        update();
    };
};

class Engine {
    Camera camera;

void setupCameraCallbacks();
public:
    GLFWwindow* window;

    Engine();
    ~Engine();

    void render(std::vector<Sphere>& spheres);
    void beginFrame();
    void endFrame();

private:
    GLuint shader;
    GLuint VAO, VBO;
    GLuint ssbo;

    void initGL();
    void initImGui();
    void setupQuad();
    GLuint createShader();

    void drawGUI();
};