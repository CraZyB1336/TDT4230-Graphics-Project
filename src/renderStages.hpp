#include "utilities/shader.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sceneGraph.hpp"
#include "shaderStructs.hpp"

void skyboxStage(SceneNode* skyboxNode, Gloom::Shader &shader, LightSource* lightSources, glm::mat4 &VP, unsigned int skyboxFBO);
void subsurfaceStage(SceneNode* rootNode, Gloom::Shader &diffuseShader, Gloom::Shader &thicknessShader, Gloom::Shader &thicknessBiasShader, Gloom::Shader &horizontalShader, Gloom::Shader &verticalShader, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition, unsigned int skyboxFBO);
void main2DStage(Gloom::Shader &shader, SceneNode* rootNode, glm::mat4 &OrthoVP);
void main3DStage(SceneNode* rootNode, Gloom::Shader &shader, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition, unsigned int skyboxFBO);
void gbufferStage(SceneNode* rootNode, Gloom::Shader &shader, glm::mat4 &VP, glm::mat4 &V, unsigned int gbufferFBO);
void ssaoStage(Gloom::Shader &shader, std::vector<glm::vec3> &kernel, unsigned int noiseTexture, unsigned int resultTexture, unsigned int positionTexture, unsigned int normalTexture, glm::mat4 &projection);