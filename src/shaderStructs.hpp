#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct LightSource {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

struct Material {
    Material() {
        albedo          = glm::vec3(0.6, 0.6, 0.6);
        specularFactor  = 1.0;
        roughnessFactor = 1.0;
    }

    glm::vec3 albedo;
    float specularFactor;
    float roughnessFactor;
};