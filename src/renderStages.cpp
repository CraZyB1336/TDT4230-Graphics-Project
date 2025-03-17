#include "renderStages.hpp"

void main3DStage(Gloom::Shader& shader, glm::mat4& VP, glm::vec3& cameraPosition) {
    GLint VPLocation = shader.getUniformFromName("VP");
    glUniformMatrix4fv(VPLocation, 1, GL_FALSE, glm::value_ptr(VP));

    GLint CamPositionLocation = shader.getUniformFromName("cameraPosition");
    glUniform3f(CamPositionLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);
}

void main2DStage(Gloom::Shader& shader, glm::mat4& OrthoVP) {
    GLint OrthoVPLocation = shader.getUniformFromName("OrthoVP");
    glUniformMatrix4fv(OrthoVPLocation, 1, GL_FALSE, glm::value_ptr(OrthoVP));
} 