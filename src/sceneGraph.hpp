#pragma once

#include <glm/glm.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <stack>
#include <vector>
#include <cstdio>
#include <stdbool.h>
#include <cstdlib> 
#include <ctime> 
#include <chrono>
#include <fstream>

#include "shaderStructs.hpp"

enum SceneNodeType {
	GEOMETRY, POINT_LIGHT, SPOT_LIGHT, DIR_LIGHT, GEOMETRY_2D, GEOMETRY_TEXTURE, SKYBOX
};

struct SceneNode {
	SceneNode() {
		position = glm::vec3(0, 0, 0);
		rotation = glm::vec3(0, 0, 0);
		scale = glm::vec3(1, 1, 1);

        referencePoint 		= glm::vec3(0, 0, 0);
        vertexArrayObjectID = -1;
		indexArrayObjectID 	= -1;
		lightSourceID 		= -1;
		textureID 			= -1;
		normalTextureID		= -1;
		roughnessTextureID	= -1;
        VAOIndexCount 		= 0;

		isSubsurface		= false;
		material 			= new Material();

		diffuseFBO			= -1;
		thicknessFBO		= -1;

		subsurfaceHorizontalTextureID 	= -1;
		subsurfaceFinalTextureID 		= -1;
		diffuseTextureID				= -1;
		thicknessTextureID				= -1;

        nodeType = GEOMETRY;
	}

	// A list of all children that belong to this node.
	// For instance, in case of the scene graph of a human body shown in the assignment text, the "Upper Torso" node would contain the "Left Arm", "Right Arm", "Head" and "Lower Torso" nodes in its list of children.
	std::vector<SceneNode*> children;
	
	// The node's position and rotation relative to its parent
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;

	// A transformation matrix representing the transformation of the node's location relative to its parent. This matrix is updated every frame.
	glm::mat4 currentTransformationMatrix;

	// The location of the node's reference point
	glm::vec3 referencePoint;

	// The ID of the VAO containing the "appearance" of this SceneNode.
	int vertexArrayObjectID;
	int indexArrayObjectID;
	int lightSourceID;
	int textureID;
	int normalTextureID;
	int roughnessTextureID;
	unsigned int VAOIndexCount;

	// Subsurface options
	bool isSubsurface;
	Material* material;

	unsigned int diffuseFBO;
	unsigned int thicknessFBO;

	unsigned int subsurfaceHorizontalTextureID;
	unsigned int subsurfaceFinalTextureID;
	unsigned int diffuseTextureID;
	unsigned int thicknessTextureID;

	// Node type is used to determine how to handle the contents of a node
	SceneNodeType nodeType;
};

SceneNode* createSceneNode();
void addChild(SceneNode* parent, SceneNode* child);
void printNode(SceneNode* node);
int totalChildren(SceneNode* parent);

// For more details, see SceneGraph.cpp.