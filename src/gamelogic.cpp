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
Gloom::Shader* thicknessShader;
Gloom::Shader* thicknessBiasShader;
Gloom::Shader* gbufferShader;
Gloom::Shader* ssaoShader;
sf::Sound* sound;

CommandLineOptions options;

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
glm::mat4 projection;
glm::mat4 cameraTransform;
glm::vec3 cameraPosition = {0.0, 0.0, 10.0};
float cameraAngle = 35.0f;

// Uniforms 2D
glm::mat4 OrthoVP;


// 3D
SceneNode* rootNode;
SceneNode* squareNode;
SceneNode* light1;
// SceneNode* skyBoxNode;

// 3D Rendering Pipeline
unsigned int skyboxTextureID;
unsigned int diffuseSubTextureID;
unsigned int subsurfacedHorizontalTextureID;
unsigned int subsurfacedFinalTextureID;
unsigned int gbufferPostionTextureID;
unsigned int gbufferNormalTextureID;
unsigned int noiseTextureID;
unsigned int ssaoResultTextureID;

std::vector<glm::vec3> ssaoKernel;

unsigned int skyboxFBO;
unsigned int diffuseFBO;
unsigned int gbufferFBO;

float lightAnimationTime = 0.0f;
float lightAnimationSpeed = 0.5f;
bool lightIsMoving = true;

// 2D
SceneNode* root2DNode;
SceneNode* text1;
SceneNode* text2;
SceneNode* text3;
SceneNode* text4;
bool hideUI = false;

bool pauseHand = false;

void initSSAOBuffers() {
    std::vector<int> gbufferIDS = generateGBuffer(windowWidth, windowHeight);
    gbufferFBO = gbufferIDS[0];
    gbufferPostionTextureID = gbufferIDS[1];
    gbufferNormalTextureID = gbufferIDS[2];
    
    ssaoKernel = generateSSAOKernel(64);
    noiseTextureID = generateNoiseTexture();
    ssaoResultTextureID = createSSAOTexture(windowWidth, windowHeight);
}

void initSubsurfaceBuffers(SceneNode* node, int windowWidth, int windowHeight) {
    std::vector<int> diffuseBufferIDS = generateFramebuffer(windowWidth, windowHeight, true);
    node->diffuseFBO = diffuseBufferIDS[0];
    node->diffuseTextureID = diffuseBufferIDS[1];

    glBindFramebuffer(GL_FRAMEBUFFER, node->diffuseFBO);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "Error: Diffuse framebuffer is not complete!" << std::endl;
    }

    std::vector<int> thicknessBufferIDS = generateThicknessBuffer(windowWidth, windowHeight);
    node->thicknessFBO = thicknessBufferIDS[0];
    node->thicknessTextureID = thicknessBufferIDS[1];
    node->thicknessBiasAlbedoTextureID = getEmptyFrameBufferTextureID(windowWidth, windowHeight);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    node->subsurfaceHorizontalTextureID = getEmptyFrameBufferTextureID(windowWidth, windowHeight);
    node->subsurfaceFinalTextureID = getEmptyFrameBufferTextureID(windowWidth, windowHeight);
}

void initSkyboxBuffer() {
    std::vector<int> skyboxIDS = generateFramebuffer(windowWidth, windowHeight, true);

    skyboxFBO       = skyboxIDS[0];
    skyboxTextureID = skyboxIDS[1];
}

void initLights() {
    light1 = createSceneNode();

    light1->lightSourceID       = 0;
    light1->nodeType            = DIR_LIGHT;
    light1->position            = {0.0, 0.0, 0.0};
    lightSources[0].color       = glm::vec3(1, 0.59, 0.3);
    lightSources[0].intensity   = 1.0;
    lightSources[0].position    = {-0.8, -0.2, -1.0};
    lightSources[0].lightType   = D_LIGHT;

    lightSources[0].position = glm::normalize(lightSources[0].position);
    light1->rotation            = lightSources[0].position;

    rootNode->children.push_back(light1);
}

void init3DNodes() {
    rootNode = createSceneNode();

    // Generate Textures
    // PNGImage brickTexture = loadPNGFile("../res/textures/skinColS.png");
    // PNGImage brickTextureNRM = loadPNGFile("../res/textures/skinNormS.png");
    // PNGImage brickTextureRGH = loadPNGFile("../res/textures/skinRghS.png");

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

    // squareNode->textureID           = brickTextureID;
    // squareNode->normalTextureID     = brickTextureNRMID;
    // squareNode->roughnessTextureID  = brickTextureRGHID;

    squareNode->isSubsurface                = true;
    squareNode->material->albedo            = {0.81, 0.72, 0.58};
    squareNode->material->specularFactor    = 0.001;
    squareNode->material->roughnessFactor   = 0.5;
    squareNode->material->subsurfaceTint    = {0.93, 0.4, 0.11};
    squareNode->material->baseThickness     = 0.25;
    squareNode->material->materialType      = 0;

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

    PNGImage charmap = loadPNGFile("../res/textures/charmap.png");

    std::string text1String = "Press \"Space\" to pause sun.";
    std::string text2String = "Press \"S\" to toggle subsurface scattering.";
    std::string text3String = "Press \"H\" to toggle hiding UI.";
    std::string text4String = "Press \"X\" to pause hand.";
    Mesh text1Mesh = generateTextGeometryBuffer(text1String, 39.0f / 29, 29 * text1String.length());
    Mesh text2Mesh = generateTextGeometryBuffer(text2String, 39.0f / 29, 29 * text2String.length());
    Mesh text3Mesh = generateTextGeometryBuffer(text3String, 39.0f / 29, 29 * text3String.length());
    Mesh text4Mesh = generateTextGeometryBuffer(text4String, 39.0f / 29, 29 * text4String.length());
    std::vector<unsigned int> text1VAOIBO = generateBuffer(text1Mesh);
    std::vector<unsigned int> text2VAOIBO = generateBuffer(text2Mesh);
    std::vector<unsigned int> text3VAOIBO = generateBuffer(text3Mesh);
    std::vector<unsigned int> text4VAOIBO = generateBuffer(text4Mesh);

    text1 = createSceneNode();
    text2 = createSceneNode();
    text3 = createSceneNode();
    text4 = createSceneNode();

    text1->vertexArrayObjectID = text1VAOIBO[0];
    text1->indexArrayObjectID = text1VAOIBO[1];
    text1->VAOIndexCount = text1Mesh.indices.size();
    text1->nodeType = GEOMETRY_2D;
    text1->textureID = getTextureID(charmap);
    text1->position = {50, 180, 0};
    text1->scale = {0.7, 0.7, 1};

    text2->vertexArrayObjectID = text2VAOIBO[0];
    text2->indexArrayObjectID = text2VAOIBO[1];
    text2->VAOIndexCount = text2Mesh.indices.size();
    text2->nodeType = GEOMETRY_2D;
    text2->textureID = text1->textureID;
    text2->position = {50, 150, 0};
    text2->scale = {0.7, 0.7, 1};

    text3->vertexArrayObjectID = text3VAOIBO[0];
    text3->indexArrayObjectID = text3VAOIBO[1];
    text3->VAOIndexCount = text3Mesh.indices.size();
    text3->nodeType = GEOMETRY_2D;
    text3->textureID = text1->textureID;
    text3->position = {50, 120, 0};
    text3->scale = {0.7, 0.7, 1};

    text4->vertexArrayObjectID = text4VAOIBO[0];
    text4->indexArrayObjectID = text4VAOIBO[1];
    text4->VAOIndexCount = text4Mesh.indices.size();
    text4->nodeType = GEOMETRY_2D;
    text4->textureID = text1->textureID;
    text4->position = {50, 210, 0};
    text4->scale = {0.7, 0.7, 1};

    root2DNode->children.push_back(text1);
    root2DNode->children.push_back(text2);
    root2DNode->children.push_back(text3);
    root2DNode->children.push_back(text4);

    OrthoVP = glm::ortho(0.0f, (float) windowWidth, 0.0f, (float) windowHeight, -1.0f, 1.0f);
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
        lightIsMoving = !lightIsMoving;
    }

    if (key == GLFW_KEY_S && action == GLFW_PRESS) {
        squareNode->isSubsurface = !squareNode->isSubsurface;
    }

    if (key == GLFW_KEY_H && action == GLFW_PRESS) {
        hideUI = !hideUI;
    }

    if (key == GLFW_KEY_X && action == GLFW_PRESS) {
        pauseHand = !pauseHand;
    }
}

void initGame(GLFWwindow* window, CommandLineOptions gameOptions) {

    options = gameOptions;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetKeyCallback(window, keyCallback);

    int windowWidth, windowHeight;
    glfwGetWindowSize(window, &windowWidth, &windowHeight);
    glViewport(0, 0, windowWidth, windowHeight);

    // Shaders
    skyboxShader = new Gloom::Shader();
    skyboxShader->makeBasicShader("../res/shaders/skybox.vert", "../res/shaders/skybox.frag");

    diffusePassShader = new Gloom::Shader();
    diffusePassShader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/diffusePass.frag");

    thicknessShader = new Gloom::Shader();
    thicknessShader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/thickness.frag");

    thicknessBiasShader = new Gloom::Shader();
    thicknessBiasShader->attach("../res/shaders/thicknessBias.comp");
    thicknessBiasShader->link();

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

    gbufferShader = new Gloom::Shader();
    gbufferShader->makeBasicShader("../res/shaders/simple.vert", "../res/shaders/gbuffer.frag");

    ssaoShader = new Gloom::Shader();
    ssaoShader->attach("../res/shaders/ssao.comp");
    ssaoShader->link();
    
    shader->activate();

    initSSAOBuffers();
    initSkyboxBuffer();
    init3DNodes();
    init2DNodes();

    getTimeDeltaSeconds();
}

void updateFrame(GLFWwindow* window) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    double timeDelta = getTimeDeltaSeconds();

    lightAnimationTime += lightIsMoving ? timeDelta * lightAnimationSpeed : 0.0;

    text1->nodeType = hideUI ? GEOMETRY : GEOMETRY_2D;
    text2->nodeType = hideUI ? GEOMETRY : GEOMETRY_2D;
    text3->nodeType = hideUI ? GEOMETRY : GEOMETRY_2D;
    text4->nodeType = hideUI ? GEOMETRY : GEOMETRY_2D;

    float xAngle = sin(lightAnimationTime) * 0.8f;
    float yAngle = cos(lightAnimationTime) * 0.8f;

    light1->rotation.x = xAngle;
    light1->rotation.y = yAngle;

    projection = glm::perspective(glm::radians(80.0f), float(windowWidth) / float(windowHeight), 0.1f, 1000.f);
    // Rotation Order: Y, X, Z
    cameraTransform = glm::lookAt(cameraPosition, cameraPosition * glm::vec3(0, 0, -1), glm::vec3(0, 1, 0));

    VP = projection * cameraTransform;

    if (!pauseHand) {
        squareNode->rotation.y = squareNode->rotation.y >= 360.0 || squareNode->rotation.y <= -360.0 ? 0.0 : squareNode->rotation.y + timeDelta * 0.2;
    }

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

    // gbufferStage(rootNode, *gbufferShader, VP, cameraTransform, gbufferFBO);
    // ssaoStage(*ssaoShader, ssaoKernel, noiseTextureID, ssaoResultTextureID, gbufferPostionTextureID, gbufferNormalTextureID, projection);
    skyboxStage(rootNode, *skyboxShader, lightSources, VP, skyboxFBO);
    subsurfaceStage(rootNode, *diffusePassShader, *thicknessShader, *thicknessBiasShader, *subsurfaceHorizontalShader, *subsurfaceVerticalShader, lightSources, VP, cameraPosition, skyboxFBO);
    main3DStage(rootNode, *shader, lightSources, VP, cameraPosition, skyboxFBO);
    main2DStage(*shader_2D, root2DNode, OrthoVP);
}
