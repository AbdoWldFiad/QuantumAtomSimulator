#include "Engine.h"
#include "Simulation.h"
#include "Common.h"


int main() {
    Engine engine;

    generateParticles();

    while (!glfwWindowShouldClose(engine.window)) {
        engine.beginFrame();

        updateParticles(0.5f);
        auto spheres = buildSpheres();

        engine.render(spheres);

        engine.endFrame();
    }

    return 0;
}