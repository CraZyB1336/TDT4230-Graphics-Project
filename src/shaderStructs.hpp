#pragma once

#include <glm/gtc/matrix_transform.hpp>

enum LightType {
	P_LIGHT, S_LIGHT, D_LIGHT
};

struct LightSource {
    // For Directional Light, this is it's direction.
    // When using it as direction light. 
        // +X is light pointing Up
        // +Y is light pointing Left
        // +Z is light pointing forward
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    LightType lightType;
};

struct Material {
    Material() {
        albedo          = {0.6, 0.6, 0.6};
        specularFactor  = 1.0;
        roughnessFactor = 0.0; // Constrained between 0-1
        
        subsurfaceTint = {1.0, 1.0, 1.0};
        baseThickness = 0.5;
        materialType = 1; 
    }

    glm::vec3 albedo;
    float specularFactor;
    float roughnessFactor;

    // Subsurface material things
    glm::vec3 subsurfaceTint;
    float baseThickness;
    int materialType; // 0 for skin, 1 for leaves.
};