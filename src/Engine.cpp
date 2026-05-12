#include "Engine.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include "Common.h"

// ImGui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

using namespace std;
using namespace glm;


Engine::Engine() {
    initGL();
    initImGui();
    setupQuad();
    shader = createShader();
    setupCameraCallbacks();
    glGenBuffers(1, &ssbo);
}

Engine::~Engine() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}

void Engine::initGL() {
    glfwInit();
    window = glfwCreateWindow(1000, 800, "Quantum Atom Simulation", NULL, NULL);
    glfwMakeContextCurrent(window);
    glClearColor(0,0,0,1);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glewInit();
    glViewport(0, 0, 1000, 800);
}

void Engine::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");
}

void Engine::setupQuad() {
    float quad[] = {
        -1,1, -1,-1, 1,-1,
        -1,1, 1,-1, 1,1
    };

    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(quad),quad,GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,2*sizeof(float),0);
}

GLuint Engine::createShader() {
    const char* vs = R"(#version 430 core
    layout(location = 0) in vec2 aPos;
    out vec2 ScreenPos;

    void main() {
        ScreenPos = aPos;
        gl_Position = vec4(aPos, 0.0, 1.0);
    }
    )";

    const char* fs = R"(#version 430 core
        out vec4 FragColor;
        in vec2 ScreenPos;

        uniform vec3 camera_pos;
        uniform mat4 inv_view_proj;
        uniform vec3 light_pos;
        uniform float light_intensity;
        uniform vec3 ambient_light;

        struct Sphere {
            vec4 center_radius;
            vec4 color;
        };

        layout(std430, binding = 0) buffer SphereBuffer {
            Sphere spheres[];
        };

        float intersect_sphere(vec3 ro, vec3 rd, vec3 c, float r) {
            vec3 oc = ro - c;
            float b = dot(oc, rd);
            float c2 = dot(oc, oc) - r*r;
            float h = b*b - c2;
            if (h < 0.0) return -1.0;
            return -b - sqrt(h);
        }

        bool any_hit(vec3 ro, vec3 rd, float max_d) {
            for (uint i = 0; i < spheres.length(); i++) {
                float t = intersect_sphere(ro, rd,
                    spheres[i].center_radius.xyz,
                    spheres[i].center_radius.w);
                if (t > 0.0 && t < max_d) return true;
            }
            return false;
        }

        void main() {
            vec4 target = inv_view_proj * vec4(ScreenPos, 1.0, 1.0);
            vec3 rd = normalize(vec3(target / target.w) - camera_pos);

            float tmin = 1e20;
            int hit = -1;

            for (uint i = 0; i < spheres.length(); i++) {
                float t = intersect_sphere(camera_pos, rd,
                    spheres[i].center_radius.xyz,
                    spheres[i].center_radius.w);
                if (t > 0.0 && t < tmin) {
                    tmin = t;
                    hit = int(i);
                }
            }

            if (hit != -1) {
                vec3 hp = camera_pos + rd * tmin;
                vec3 n = normalize(hp - spheres[hit].center_radius.xyz);

                vec3 ldir = normalize(light_pos - hp);
                float ldist = length(light_pos - hp);

                float shadow = any_hit(hp + n * 0.001, ldir, ldist) ? 0.0 : 1.0;

                float diff = max(dot(n, ldir), 0.0);

                vec3 col = spheres[hit].color.rgb;

                vec3 lighting =
                    ambient_light * col +
                    diff * col * light_intensity * shadow;

                FragColor = vec4(lighting, spheres[hit].color.a);
            } else {
                FragColor = vec4(0,0,0,1);
            }
        }
        )";
    GLuint v=glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v,1,&vs,NULL);
    glCompileShader(v);

    GLuint f=glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f,1,&fs,NULL);
    glCompileShader(f);

    GLuint p=glCreateProgram();
    glAttachShader(p,v);
    glAttachShader(p,f);
    glLinkProgram(p);

    return p;
}

void Engine::drawGUI() {
    ImGuiIO& io = ImGui::GetIO();

    // Position: top-right corner
    ImVec2 window_pos = ImVec2(io.DisplaySize.x - 10.0f, 10.0f);
    ImVec2 window_pivot = ImVec2(1.0f, 0.0f); // anchor top-right

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pivot);
    ImGui::SetNextWindowBgAlpha(0.9f); // optional transparency

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("Quantum Controls", nullptr, flags)) {

        // Collapsible section
        if (ImGui::CollapsingHeader("Quantum Numbers", ImGuiTreeNodeFlags_DefaultOpen)) {

            bool changed = false;

            int n = 1;
            int l = 0;
            int m = 0;

            changed |= ImGui::SliderInt("n", &n, 1, 10);
            changed |= ImGui::SliderInt("l", &l, 0, n - 1);
            changed |= ImGui::SliderInt("m", &m, -l, l);

            if (changed)
                generateParticles();
        }

        if (ImGui::CollapsingHeader("Simulation")) {
            if (ImGui::SliderInt("Particles", &N, 100, 50000)) {
                N = std::max(100, N);
                generateParticles();
            }
        }

        // --- Controls help ---
        if (ImGui::CollapsingHeader("Controls")) {
            ImGui::Text("W/S : n +/-");
            ImGui::Text("E/D : l +/-");
            ImGui::Text("R/F : m +/-");
            ImGui::Text("T/G : Particles +/-");
        }

        // --- Reset button ---
        ImGui::Separator();
        if (ImGui::Button("Reset Simulation")) {
            n = 1; l = 0; m = 0; N = 100;
            generateParticles();
        }

        ImGui::Separator();
        ImGui::Text("FPS: %.1f", io.Framerate);
    }

    ImGui::End();
}

void Engine::beginFrame() {
    glfwPollEvents();
    glClear(GL_COLOR_BUFFER_BIT);

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    drawGUI();
}


void Engine::render(vector<Sphere>& spheres) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
        spheres.size()*sizeof(Sphere),
        spheres.data(),
        GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    glUseProgram(shader);

    // Camera matrices
    glm::mat4 view = glm::lookAt(
        camera.position(),
        camera.target,
        glm::vec3(0,1,0)
    );

    glm::mat4 proj = glm::perspective(
        glm::radians(45.0f),
        1000.0f/800.0f,
        0.1f,
        10000.0f
    );

    glm::mat4 invVP = glm::inverse(proj * view);

    glUniform3fv(glGetUniformLocation(shader,"camera_pos"),1,
        glm::value_ptr(camera.position()));
    glUniformMatrix4fv(glGetUniformLocation(shader,"inv_view_proj"),
        1,GL_FALSE,glm::value_ptr(invVP));

    glm::vec3 light_pos = glm::vec3(0,50,50);
    glm::vec3 ambient = glm::vec3(0.2f);
    float intensity = 3.0f;

    glUniform3fv(glGetUniformLocation(shader,"light_pos"),1,
        glm::value_ptr(light_pos));
    glUniform3fv(glGetUniformLocation(shader,"ambient_light"),1,
        glm::value_ptr(ambient));
    glUniform1f(glGetUniformLocation(shader,"light_intensity"),intensity);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES,0,6);
}

void Engine::endFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}
void Engine::setupCameraCallbacks() {
    glfwSetWindowUserPointer(window, &camera);

    glfwSetCursorPosCallback(window, [](GLFWwindow* win, double x, double y) {
        ((Camera*)glfwGetWindowUserPointer(win))->processMouseMove(x, y);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow* win, int button, int action, int mods) {
        ((Camera*)glfwGetWindowUserPointer(win))->processMouseButton(button, action, mods, win);
    });

    glfwSetScrollCallback(window, [](GLFWwindow* win, double xoffset, double yoffset) {
        ((Camera*)glfwGetWindowUserPointer(win))->processScroll(xoffset, yoffset);
    });
}