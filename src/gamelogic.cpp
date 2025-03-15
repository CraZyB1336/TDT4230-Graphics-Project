#include <chrono>
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <SFML/Audio/SoundBuffer.hpp>
#include <utilities/shader.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <utilities/timeutils.h>
#include <utilities/mesh.h>
#include <utilities/shapes.h>
#include <utilities/glutils.h>
#include <SFML/Audio/Sound.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fmt/format.h>
#include "gamelogic.h"
#include "sceneGraph.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "utilities/imageLoader.hpp"
#include "utilities/glfont.h"
#include "textureHandler.hpp"

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
Gloom::Shader* shader_2D;
sf::Sound* sound;

CommandLineOptions options;

bool mouseLeftPressed   = false;
bool mouseLeftReleased  = false;
bool mouseRightPressed  = false;
bool mouseRightReleased = false;

// Modify if you want the music to start further on in the track. Measured in seconds.
const float debug_startTime = 0;
double totalElapsedTime = debug_startTime;
double gameElapsedTime = debug_startTime;

double mouseSensitivity = 1.0;
double lastMouseX = windowWidth / 2;
double lastMouseY = windowHeight / 2;
void mouseCallback(GLFWwindow* window, double x, double y) {
    // Stuff to do when mouse is called.
}

struct LightSource {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
};

LightSource lightSources[1];

glm::mat4 identityMat = {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0}
};

// Uniforms 3D
glm::mat4 VP;
glm::vec3 cameraPosition = {0.0, 0.0, 30.0};
float cameraAngle = 35.0f;

// Uniforms 2D
glm::mat4 OrthoVP;


// 3D
SceneNode* rootNode;
SceneNode* squareNode;

// 2D
SceneNode* root2DNode;

void initLights() {
    SceneNode* light1 = createSceneNode();

    light1->lightSourceID       = 0;
    light1->nodeType            = POINT_LIGHT;
    light1->position            = {0.0, 0.0, 60.0};
    lightSources[0].color       = glm::vec3(1, 0.59, 0.3);
    lightSources[0].intensity   = 2.0;

    rootNode->children.push_back(light1);
}

void init3DNodes() {
    rootNode = createSceneNode();

    // Generate Textures
    PNGImage brickTexture = loadPNGFile("../res/textures/Brick03_col.png");
    PNGImage brickTextureNRM = loadPNGFile("../res/textures/Brick03_nrm.png");
    PNGImage brickTextureRGH = loadPNGFile("../res/textures/Brick03_rgh.png");

    int brickTextureID = getTextureID(brickTexture);
    int brickTextureNRMID = getTextureID(brickTextureNRM);
    int brickTextureRGHID = getTextureID(brickTextureRGH);

    Mesh squareMesh = cube({20.0, 20.0, 20.0}, {15.0, 15.0}, true, false, {1.0, 1.0, 1.0});
    // Mesh squareMesh = generateSphere(15.0, 40, 40, {2.0, 2.0});
    std::vector<unsigned int> squareVAOIBO = generateBuffer(squareMesh);
    squareNode = createSceneNode();
    squareNode->nodeType            = GEOMETRY_TEXTURE;
    squareNode->vertexArrayObjectID = squareVAOIBO[0];
    squareNode->indexArrayObjectID  = squareVAOIBO[1];
    squareNode->VAOIndexCount       = squareMesh.indices.size();
    squareNode->textureID           = brickTextureID;
    squareNode->normalTextureID     = brickTextureNRMID;
    squareNode->roughnessTextureID  = brickTextureRGHID;

    rootNode->children.push_back(squareNode);

    initLights();
}

void init2DNodes() {
    root2DNode = createSceneNode();
}

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {

    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");

    shader_2D = new Gloom::Shader();
    shader_2D->makeBasicShader("../res/shaders/simple_2D.vert", "../res/shaders/simple_2D.frag");
    
    shader->activate();

    init3DNodes();
    init2DNodes();

    getTimeDeltaSeconds();
}

void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
        mouseLeftPressed = true;
        mouseLeftReleased = false;
    } else {
        mouseLeftReleased = mouseLeftPressed;
        mouseLeftPressed = false;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
        mouseRightPressed = true;
        mouseRightReleased = false;
    } else {
        mouseRightReleased = mouseRightPressed;
        mouseRightPressed = false;
    }

    glm::mat4 projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 350.f);
    // Rotation Order: Y, X, Z
    glm::mat4 cameraTransform = glm::translate(-cameraPosition);

    VP = projection * cameraTransform;

    squareNode->rotation.y = squareNode->rotation.y >= 360.0 || squareNode->rotation.y <= -360.0 ? 0.0 : squareNode->rotation.y + timeDelta * 0.2;

    updateNodeTransformations(rootNode, identityMat);
    updateNodeTransformations(root2DNode, identityMat);
}

void updateNodeTransformations(SceneNode* node, glm::mat4 transformationThusFar) {
    glm::mat4 transformationMatrix =
              glm::translate(node->position)
            * glm::translate(node->referencePoint)
            * glm::rotate(node->rotation.y, glm::vec3(0,1,0))
            * glm::rotate(node->rotation.x, glm::vec3(1,0,0))
            * glm::rotate(node->rotation.z, glm::vec3(0,0,1))
            * glm::scale(node->scale)
            * glm::translate(-node->referencePoint);

    node->currentTransformationMatrix = transformationThusFar * transformationMatrix;

    switch(node->nodeType) {
        case GEOMETRY: break;
        case POINT_LIGHT: {
            glm::vec4 lightPosition = node->currentTransformationMatrix * glm::vec4(0, 0, 0, 1);
            if (node->lightSourceID != -1) {
                lightSources[node->lightSourceID].position = glm::vec3(lightPosition.x, lightPosition.y, lightPosition.z);
            }
            break;
        }
        case SPOT_LIGHT: break;
        default: break;
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void passInNormalMatrix(SceneNode* node) {
    GLint normalMatLoc = shader->getUniformFromName("NM");

    glm::mat3 NM3 = glm::mat3(node->currentTransformationMatrix);
    glm::mat3 NM = glm::transpose(glm::inverse(NM3));
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, glm::value_ptr(NM));
}

void renderNode(SceneNode* node) {
    GLint MLocation = shader->getUniformFromName("M");
    glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

    passInNormalMatrix(node);

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                GLint hasTextureLocation = shader->getUniformFromName("hasTexture");
                glUniform1i(hasTextureLocation, 0);
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case POINT_LIGHT: {
            int i = node->lightSourceID;
            if (i != -1) {
                GLint lightPosLoc = shader->getUniformFromName("lights[" + std::to_string(i) + "].position");
                glUniform3f(lightPosLoc, lightSources[i].position.x, lightSources[i].position.y, lightSources[i].position.z);

                GLint lightColLoc = shader->getUniformFromName("lights[" + std::to_string(i) + "].color");
                glUniform3f(lightColLoc, lightSources[i].color.x, lightSources[i].color.y, lightSources[i].color.z);

                GLint lightIntLoc = shader->getUniformFromName("lights[" + std::to_string(i) + "].intensity");
                glUniform1f(lightIntLoc, lightSources[i].intensity);
            }
            break;
        }
        case SPOT_LIGHT: break;
        case GEOMETRY_TEXTURE: {
            GLint hasTextureLocation = shader->getUniformFromName("hasTexture");
            glUniform1i(hasTextureLocation, 1);

            glBindTextureUnit(0, node->textureID);
            glBindTextureUnit(1, node->normalTextureID);
            glBindTextureUnit(2, node->roughnessTextureID);

            glBindVertexArray(node->vertexArrayObjectID);
            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            break;
        }
        default: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child);
    }
}

void render2DNode(SceneNode* node) {
    GLint MLocation = shader->getUniformFromName("M");
    glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

    switch(node->nodeType) {
        case GEOMETRY_2D: {
            if (node->textureID != -1) {
                glBindTextureUnit(0, node->textureID);
            }

            glBindVertexArray(node->vertexArrayObjectID);
            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            break;
        }
        default: break;
    }

    for(SceneNode* child : node->children) {
        render2DNode(child);
    }
}

void setup3Duniforms() {
    // Pass the VP to the shader
    GLint VPLocation = shader->getUniformFromName("VP");
    glUniformMatrix4fv(VPLocation, 1, GL_FALSE, glm::value_ptr(VP));

    GLint CamPositionLocation = shader->getUniformFromName("cameraPosition");
    glUniform3f(CamPositionLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);
}

void setup2Duniforms() {
    GLint OrthoVPLocation = shader_2D->getUniformFromName("OrthoVP");
    glUniformMatrix4fv(OrthoVPLocation, 1, GL_FALSE, glm::value_ptr(OrthoVP));
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    glEnable(GL_DEPTH_TEST);
    shader->activate();
    setup3Duniforms();
    renderNode(rootNode);


    glDisable(GL_DEPTH_TEST);
    shader_2D->activate();
    setup2Duniforms();
    render2DNode(root2DNode);
}
