#include "utilities/shader.hpp"
#include <glad/glad.h>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void main3DStage(Gloom::Shader &shader, glm::mat4 &VP, glm::vec3 &cameraPosition);
void main2DStage(Gloom::Shader &shader, glm::mat4 &OrthoVP);