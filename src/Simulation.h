#pragma once
#include <glm/glm.hpp>
#include <vector>

using namespace glm;

struct Particle {
    vec3 pos;                    //3D location
    vec3 vel;       //motion (from quantum probability flow)
    vec4 color;                  //based on probability density

    Particle(vec3 p, vec4 c) : pos(p), vel(0.0f), color(c) {}
};

struct Sphere {
    vec4 center_radius;
    vec4 color;
};

extern std::vector<Particle> particles;

// global quantum params
extern int N;
extern float n, l, m;

void generateParticles();
void updateParticles(float dt);
std::vector<Sphere> buildSpheres();