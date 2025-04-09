#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct LightSource {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    SceneNodeType lightType = POINT_LIGHT;
};

struct Material {
    Material() {
        albedo          = glm::vec3(0.6, 0.6, 0.6);
        specularFactor  = 1.0;
        roughnessFactor = 0.0; // Constrained between 0-1
    }

    glm::vec3 albedo;
    float specularFactor;
    float roughnessFactor;
};