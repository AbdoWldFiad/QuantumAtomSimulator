#include "Simulation.h"
#include <random>
#include <cmath>
#define _USE_MATH_DEFINES
#include <cmath>
#include <algorithm>
#include <random>
#include <complex>
#include "Common.h"

using namespace std;
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

vector<Particle> particles;

int N = 100;
float n = 3, l = 1, m = 1;

static random_device rd;
static mt19937 gen(rd());
static uniform_real_distribution<float> dis(0.0f, 1.0f);

vec3 sphericalToCartesian(float r, float theta, float phi){
    return vec3(
        r * sin(theta) * cos(phi),
        r * cos(theta),
        r * sin(theta) * sin(phi)
    );
}

// --- sample R ---
double sampleR(int n, int l, mt19937& gen) {
    const int N = 4096;
    //const double a0 = 1.0;
    const double rMax = 10.0 * n * n * a0;

    static vector<double> cdf;
    static bool built = false;

    if (!built) {
        cdf.resize(N);
        double dr = rMax / (N - 1);
        double sum = 0.0;

        for (int i = 0; i < N; ++i) {
            double r = i * dr;
            double rho = 2.0 * r / (n * a0);

            // Associated Laguerre L_{n-l-1}^{2l+1}(rho)
            int k = n - l - 1;
            int alpha = 2 * l + 1;

            double L = 1.0, Lm1 = 1.0 + alpha - rho;
            if (k == 1) L = Lm1;
            else if (k > 1) {
                double Lm2 = 1.0;
                for (int j = 2; j <= k; ++j) {
                    L = ((2*j - 1 + alpha - rho) * Lm1 -
                         (j - 1 + alpha) * Lm2) / j;
                    Lm2 = Lm1;
                    Lm1 = L;
                }
            }

            double norm = pow(2.0 / (n * a0), 3) * tgamma(n - l) / (2.0 * n * tgamma(n + l + 1));
            double R = sqrt(norm) * exp(-rho / 2.0) * pow(rho, l) * L;

            double pdf = r * r * R * R;  //This ensures particles appear where the electron is likely to be.
            sum += pdf;
            cdf[i] = sum;
        }

        for (double& v : cdf) v /= sum;
        built = true;
    }

    uniform_real_distribution<double> dis(0.0, 1.0);
    double u = dis(gen);

    int idx = lower_bound(cdf.begin(), cdf.end(), u) - cdf.begin();
    return idx * (rMax / (N - 1));
}
// --- sample Theta ---
double sampleTheta(int l, int m, mt19937& gen) {
    const int N = 2048;
    static vector<double> cdf;
    static bool built = false;

    if (!built) {
        cdf.resize(N);
        double dtheta = M_PI / (N - 1);
        double sum = 0.0;

        for (int i = 0; i < N; ++i) {
            double theta = i * dtheta;
            double x = cos(theta);

            // Associated Legendre P_l^m(x)
            double Pmm = 1.0;
            if (m > 0) {
                double somx2 = sqrt((1.0 - x) * (1.0 + x));
                double fact = 1.0;
                for (int j = 1; j <= m; ++j) {
                    Pmm *= -fact * somx2;
                    fact += 2.0;
                }
            }

            double Plm;
            if (l == m) {
                Plm = Pmm;
            } else {
                double Pm1m = x * (2 * m + 1) * Pmm;
                if (l == m + 1) {
                    Plm = Pm1m;
                } else {
                    double Pll;
                    for (int ll = m + 2; ll <= l; ++ll) {
                        Pll = ((2 * ll - 1) * x * Pm1m -
                               (ll + m - 1) * Pmm) / (ll - m);
                        Pmm = Pm1m;
                        Pm1m = Pll;
                    }
                    Plm = Pm1m;
                }
            }

            double pdf = sin(theta) * Plm * Plm;
            sum += pdf;
            cdf[i] = sum;
        }

        for (double& v : cdf) v /= sum;
        built = true;
    }

    uniform_real_distribution<double> dis(0.0, 1.0);
    double u = dis(gen);

    int idx = lower_bound(cdf.begin(), cdf.end(), u) - cdf.begin();
    return idx * (M_PI / (N - 1));
}
// --- sample Phi (uniform) ---
float samplePhi(float n, float l, float m) {
    return 2.0f * M_PI * dis(gen);
}
// --- calculate prob current ---
    //This simulates probability current : Particles rotate around the z-axis & Speed depends on 'm'
vec3 calculateProbabilityFlow(Particle& p, int n, int l, int m) {  
    double r = length(p.pos);   if (r < 1e-6) return vec3(0.0f);
    double theta = acos(p.pos.y / r); 
    double phi = atan2(p.pos.z, p.pos.x); 


    //Compute magnitude
    double sinTheta = sin(theta);  if (abs(sinTheta) < 1e-4) sinTheta = 1e-4;
    double v_mag = hbar * m / (m_e * r * sinTheta);

    //Convert to Cartesian
    double vx = -v_mag * sin(phi);
    double vy = 0.0; 
    double vz =  v_mag * cos(phi);

    return vec3((float)vx, (float)vy, (float)vz);
}

vec4 inferno2(double r, double theta, double phi, int n, int l, int m) {
    // --- radial part |R(r)|^2 ---
    double rho = 2.0 * r / (n * a0);

    int k = n - l - 1;
    int alpha = 2 * l + 1;

    double L = 1.0;
    if (k == 1) {
        L = 1.0 + alpha - rho;
    } else if (k > 1) {
        double Lm2 = 1.0;
        double Lm1 = 1.0 + alpha - rho;
        for (int j = 2; j <= k; ++j) {
            L = ((2*j - 1 + alpha - rho) * Lm1 -
                 (j - 1 + alpha) * Lm2) / j;
            Lm2 = Lm1;
            Lm1 = L;
        }
    }

    double norm = pow(2.0 / (n * a0), 3)
                * tgamma(n - l)
                / (2.0 * n * tgamma(n + l + 1));

    double R = sqrt(norm) * exp(-rho / 2.0) * pow(rho, l) * L;
    double radial = R * R;

    // --- angular part |P_l^m(cosθ)|^2 ---
    double x = cos(theta);

    double Pmm = 1.0;
    if (m > 0) {
        double somx2 = sqrt((1.0 - x) * (1.0 + x));
        double fact = 1.0;
        for (int j = 1; j <= m; ++j) {
            Pmm *= -fact * somx2;
            fact += 2.0;
        }
    }

    double Plm;
    if (l == m) {
        Plm = Pmm;
    } else {
        double Pm1m = x * (2*m + 1) * Pmm;
        if (l == m + 1) {
            Plm = Pm1m;
        } else {
            for (int ll = m + 2; ll <= l; ++ll) {
                double Pll = ((2*ll - 1) * x * Pm1m -
                              (ll + m - 1) * Pmm) / (ll - m);
                Pmm = Pm1m;
                Pm1m = Pll;
            }
            Plm = Pm1m;
        }
    }

    double angular = Plm * Plm;

    double intensity = radial * angular;

    // log compression
    double t = log10(intensity + 1e-12) + 12.0;
    t /= 12.0;

    t = std::clamp(t, 0.0, 1.0);

    // --- inferno-style ramp ---
    float rC = smoothstep(0.15f, 1.0f, static_cast<float>(t));
    float gC = smoothstep(0.45f, 1.0f, static_cast<float>(t));
    float bC = smoothstep(0.85f, 1.0f, static_cast<float>(t)) * 0.2f;

    return vec4(rC, gC * 0.8f, bC, 1.0f);
}

vec4 heatmap_fire(float value) {
    // Ensure value is clamped between 0 and 1
    value = std::max(0.0f, std::min(1.0f, value));

    // Define color stops for the "Heat/Fire" pattern
    // Order: Black -> Dark Purple -> Red -> Orange -> Yellow -> White
    const int num_stops = 6;
    vec4 colors[num_stops] = {
        {0.0f, 0.0f, 0.0f, 1.0f}, // 0.0: Black
        {0.3f, 0.0f, 0.6f, 1.0f}, // 0.2: Dark Purple
        {0.8f, 0.0f, 0.0f, 1.0f}, // 0.4: Deep Red
        {1.0f, 0.5f, 0.0f, 1.0f}, // 0.6: Orange
        {1.0f, 1.0f, 0.0f, 1.0f}, // 0.8: Yellow
        {1.0f, 1.0f, 1.0f, 1.0f}  // 1.0: White
    };

    // Find which segment the value falls into
    float scaled_v = value * (num_stops - 1);
    int i = static_cast<int>(scaled_v);
    int next_i = std::min(i + 1, num_stops - 1);
    
    // Calculate how far we are between stop 'i' and 'next_i'
    float local_t = scaled_v - i;

    // Linearly interpolate between the two colors
    vec4 result;
    result.r = colors[i].r + local_t * (colors[next_i].r - colors[i].r);
    result.g = colors[i].g + local_t * (colors[next_i].g - colors[i].g);
    result.b = colors[i].b + local_t * (colors[next_i].b - colors[i].b);
    result.a = 1.0f; // Solid opacity
    // result = vec4(0.2, 0.9, 0.05, 1.0);

    return result;
}
vec4 inferno(double r, double theta, double phi, int n, int l, int m) {
    // --- radial part |R(r)|^2 ---
    double rho = 2.0 * r / (n * a0);

    int k = n - l - 1;
    int alpha = 2 * l + 1;

    double L = 1.0;
    if (k == 1) {
        L = 1.0 + alpha - rho;
    } else if (k > 1) {
        double Lm2 = 1.0;
        double Lm1 = 1.0 + alpha - rho;
        for (int j = 2; j <= k; ++j) {
            L = ((2*j - 1 + alpha - rho) * Lm1 -
                 (j - 1 + alpha) * Lm2) / j;
            Lm2 = Lm1;
            Lm1 = L;
        }
    }

    double norm = pow(2.0 / (n * a0), 3)
                * tgamma(n - l)
                / (2.0 * n * tgamma(n + l + 1));

    double R = sqrt(norm) * exp(-rho / 2.0) * pow(rho, l) * L;
    double radial = R * R;

    // --- angular part |P_l^m(cosθ)|^2 ---
    double x = cos(theta);

    double Pmm = 1.0;
    if (m > 0) {
        double somx2 = sqrt((1.0 - x) * (1.0 + x));
        double fact = 1.0;
        for (int j = 1; j <= m; ++j) {
            Pmm *= -fact * somx2;
            fact += 2.0;
        }
    }

    double Plm;
    if (l == m) {
        Plm = Pmm;
    } else {
        double Pm1m = x * (2*m + 1) * Pmm;
        if (l == m + 1) {
            Plm = Pm1m;
        } else {
            for (int ll = m + 2; ll <= l; ++ll) {
                double Pll = ((2*ll - 1) * x * Pm1m -
                              (ll + m - 1) * Pmm) / (ll - m);
                Pmm = Pm1m;
                Pm1m = Pll;
            }
            Plm = Pm1m;
        }
    }

    double angular = Plm * Plm;

    double intensity = radial * angular;

    //cout << "intensity: " << intensity << endl;
    // return vec4(1.0f);
    return heatmap_fire(intensity * LightingScaler); // Scale for better color mapping
}

void generateParticles() {
    particles.clear();

    for (int i = 0; i < N; ++i) {
        vec3 pos = sphericalToCartesian(
            sampleR(n, l, gen), //Builds a CDF (cumulative distribution function) once, Then samples using inverse transform sampling
            sampleTheta(l, m, gen), // P(θ)=sinθ⋅∣Plm​(cosθ)∣2 , plm = associated Legendre polynomial
            samplePhi(n, l, m) // Uniform because probability is symmetric in φ for magnitude.
        );

        float r = length(pos);
        double theta = acos(pos.y / r);
        double phi = atan2(pos.z, pos.x);

        vec4 col = inferno(r, theta, phi, n, l, m);
        particles.emplace_back(pos, col);
    }
}

void updateParticles(float dt) {
    for (auto& p : particles) {
        p.vel = calculateProbabilityFlow(p, n, l, m);

        // FULL Cartesian integration
        p.pos += p.vel * dt;
    }
}

vector<Sphere> buildSpheres() {
    vector<Sphere> out;
    for (auto& p : particles) {
        out.push_back({vec4(p.pos, 0.25f), p.color});
    }
    return out;
}