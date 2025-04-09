#include "utilities/shader.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "sceneGraph.hpp"
#include "shaderStructs.hpp"

void passInAllLights(LightSource* lightSources, int lightSourcesLen, Gloom::Shader &shader);
void diffuseBufferStage(Gloom::Shader &shader, SceneNode* rootNode, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition, unsigned int &diffuseFBO);
void subsurfaceHorizontalStage(Gloom::Shader &shader, unsigned int &diffuseSubTextureID, unsigned int &horizontalTextureID, int windowWidth, int windowHeight);
void subsurfaceVerticalStage(Gloom::Shader &shader, unsigned int &horizontalTextureID, unsigned int &subsurfacedTextureID, int windowWidth, int windowHeight);
void main3DStage(Gloom::Shader &shader, SceneNode* rootNode, unsigned int &diffuseTextureID, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition);
void main2DStage(Gloom::Shader &shader, SceneNode* rootNode, glm::mat4 &OrthoVP);