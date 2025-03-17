#include "utilities/shader.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sceneGraph.hpp"
#include "shaderStructs.hpp"

void main3DStage(Gloom::Shader &shader, SceneNode* rootNode, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition);
void main2DStage(Gloom::Shader &shader, SceneNode* rootNode, glm::mat4 &OrthoVP);