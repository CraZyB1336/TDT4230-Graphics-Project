#pragma once

#include <glm/gtc/matrix_transform.hpp>

struct LightSource {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};