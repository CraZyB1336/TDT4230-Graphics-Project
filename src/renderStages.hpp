#include "utilities/shader.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sceneGraph.hpp"
#include "shaderStructs.hpp"

void skyboxStage(SceneNode* skyboxNode, Gloom::Shader &shader, LightSource* lightSources, glm::mat4 &VP, unsigned int skyboxFBO);
void subsurfaceStage(SceneNode* rootNode, Gloom::Shader &diffuseShader, Gloom::Shader &thicknessShader, Gloom::Shader &horizontalShader, Gloom::Shader &verticalShader, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition, unsigned int skyboxFBO);
void main2DStage(Gloom::Shader &shader, SceneNode* rootNode, glm::mat4 &OrthoVP);
void main3DStage(SceneNode* rootNode, Gloom::Shader &shader, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition, unsigned int skyboxFBO);
// void diffuseStage(Gloom::Shader &diffuseShader, Gloom::Shader &ssHorizontal, Gloom::Shader &ssVertical, Gloom::Shader &main3Dshader, SceneNode *rootNode, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition, unsigned int &skyboxFBO);

// void renderSkyboxPass(SceneNode* skyboxNode, Gloom::Shader &shader, LightSource* lightSources, glm::mat4 &VP, unsigned int skyboxFBO);
// void renderSubsurface(SceneNode* node, Gloom::Shader &diffuseShader, Gloom::Shader &horizontalShader, Gloom::Shader &verticalShader, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition, unsigned int skyboxFBO);