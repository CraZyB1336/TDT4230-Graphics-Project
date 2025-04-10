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
#include "renderStages.hpp"
#include "shaderStructs.hpp"
#include "utilities/meshFileReader.hpp"

enum KeyFrameAction {
    BOTTOM, TOP
};

#include <timestamps.h>

// These are heap allocated, because they should not be initialised at the start of the program
sf::SoundBuffer* buffer;
Gloom::Shader* shader;
Gloom::Shader* shader_2D;
Gloom::Shader* diffusePassShader;
Gloom::Shader* subsurfaceHorizontalShader;
Gloom::Shader* subsurfaceVerticalShader;
Gloom::Shader* skyboxShader;
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

LightSource lightSources[1];

glm::mat4 identityMat = {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0}
};

// Uniforms 3D
glm::mat4 VP;
glm::vec3 cameraPosition = {0.0, 0.0, 10.0};
float cameraAngle = 35.0f;

// Uniforms 2D
glm::mat4 OrthoVP;


// 3D
SceneNode* rootNode;
SceneNode* squareNode;
// SceneNode* skyBoxNode;

// 3D Rendering Pipeline
unsigned int skyboxTextureID;
unsigned int diffuseSubTextureID;
unsigned int subsurfacedHorizontalTextureID;
unsigned int subsurfacedFinalTextureID;
unsigned int skyboxFBO;
unsigned int diffuseFBO;

// 2D
SceneNode* root2DNode;

void initSubsurfaceBuffers(SceneNode* node, int windowWidth, int windowHeight) {
    std::vector<int> diffuseBufferIDS = generateFramebuffer(windowWidth, windowHeight, true);
    node->diffuseFBO = diffuseBufferIDS[0];
    node->diffuseTextureID = diffuseBufferIDS[1];

    node->subsurfaceHorizontalTextureID = getEmptyFrameBufferTextureID(windowWidth, windowHeight);
    node->subsurfaceFinalTextureID = getEmptyFrameBufferTextureID(windowWidth, windowHeight);

    glBindFramebuffer(GL_FRAMEBUFFER, node->diffuseFBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error: Diffuse framebuffer is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSkyboxBuffer() {
    std::vector<int> skyboxIDS = generateFramebuffer(windowWidth, windowHeight, true);

    skyboxFBO       = skyboxIDS[0];
    skyboxTextureID = skyboxIDS[1];
}

void initLights() {
    SceneNode* light1 = createSceneNode();

    light1->lightSourceID       = 0;
    light1->nodeType            = DIR_LIGHT;
    light1->position            = {0.0, 0.0, 0.0};
    lightSources[0].color       = glm::vec3(1, 0.59, 0.3);
    lightSources[0].intensity   = 1.0;
    lightSources[0].position    = {-1.0, -1.0, 1.0};
    lightSources[0].lightType   = D_LIGHT;

    glm::normalize(lightSources[0].position);
    light1->rotation            = lightSources[0].position;

    rootNode->children.push_back(light1);
}

void init3DNodes() {
    rootNode = createSceneNode();

    // Generate Textures
    // PNGImage brickTexture = loadPNGFile("../res/textures/Brick03_col.png");
    // PNGImage brickTextureNRM = loadPNGFile("../res/textures/Brick03_nrm.png");
    // PNGImage brickTextureRGH = loadPNGFile("../res/textures/Brick03_rgh.png");

    // int brickTextureID = getTextureID(brickTexture);
    // int brickTextureNRMID = getTextureID(brickTextureNRM);
    // int brickTextureRGHID = getTextureID(brickTextureRGH);

    // Mesh squareMesh = cube({20.0, 20.0, 20.0}, {15.0, 15.0}, true, false, {1.0, 1.0, 1.0});
    // Mesh squareMesh = generateSphere(15.0, 40, 40, {2.0, 2.0});

    Mesh squareMesh = loadOBJFile("../res/models/hand.obj");
    std::vector<unsigned int> squareVAOIBO = generateBuffer(squareMesh);
    squareNode = createSceneNode();
    squareNode->nodeType            = GEOMETRY;
    squareNode->vertexArrayObjectID = squareVAOIBO[0];
    squareNode->indexArrayObjectID  = squareVAOIBO[1];
    squareNode->VAOIndexCount       = squareMesh.indices.size();

    squareNode->isSubsurface                = true;
    squareNode->material->albedo            = {0.81, 0.72, 0.58};
    squareNode->material->specularFactor    = 0.001;
    squareNode->material->roughnessFactor   = 0.5;
    squareNode->material->subsurfaceTint    = {0.93, 0.4, 0.11};
    squareNode->material->subsurfaceThickness = 0.75;

    initSubsurfaceBuffers(squareNode, windowWidth, windowHeight);

    rootNode->children.push_back(squareNode);

    Mesh skyBox = cube({450.0, 450.0, 450.0}, {1.0, 1.0}, true, true, {1.0, 1.0, 1.0});
    std::vector<unsigned int> skyboxVAOIBO = generateBuffer(skyBox);
    rootNode->nodeType              = SKYBOX;
    rootNode->vertexArrayObjectID   = skyboxVAOIBO[0];
    rootNode->indexArrayObjectID    = skyboxVAOIBO[1];
    rootNode->VAOIndexCount         = skyBox.indices.size();

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

    // Shaders
    skyboxShader = new Gloom::Shader();
    skyboxShader->makeBasicShader("../res/shaders/skybox.vert", "../res/shaders/skybox.frag");

    diffusePassShader = new Gloom::Shader();
    diffusePassShader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/diffusePass.frag");

    subsurfaceHorizontalShader  = new Gloom::Shader();
    subsurfaceHorizontalShader->attach("../res/shaders/subsurfaceHorizontal.comp");
    subsurfaceHorizontalShader->link();

    subsurfaceVerticalShader    = new Gloom::Shader();
    subsurfaceVerticalShader->attach("../res/shaders/subsurfaceVertical.comp");
    subsurfaceVerticalShader->link();

    shader = new Gloom::Shader();
    shader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/simple.frag");

    shader_2D = new Gloom::Shader();
    shader_2D->makeBasicShader("../res/shaders/simple_2D.vert", "../res/shaders/simple_2D.frag");
    
    shader->activate();

    initSkyboxBuffer();
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
        case DIR_LIGHT: {
            if (node->lightSourceID != -1) {
                glm::mat3 rotationMatrix = glm::mat3(node->currentTransformationMatrix);
                lightSources[node->lightSourceID].position = rotationMatrix * glm::vec3(0.0, 0.0, -1.0);
            }
            break;
        }
        default: break;
    }

    for(SceneNode* child : node->children) {
        updateNodeTransformations(child, node->currentTransformationMatrix);
    }
}

void renderFrame(GLFWwindow* window) {
    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    skyboxStage(rootNode, *skyboxShader, lightSources, VP, skyboxFBO);
    subsurfaceStage(rootNode, *diffusePassShader, *subsurfaceHorizontalShader, *subsurfaceVerticalShader, lightSources, VP, cameraPosition, skyboxFBO);
    main3DStage(rootNode, *shader, lightSources, VP, cameraPosition, skyboxFBO);
    // main2DStage(*shader_2D, root2DNode, OrthoVP);
}
