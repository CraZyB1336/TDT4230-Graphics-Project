#include <iostream>
#include <glad/glad.h>
#include "glfont.h"

int fullAtlasWidth = 128 * 29;

Mesh generateTextGeometryBuffer(std::string text, float characterHeightOverWidth, float totalTextWidth) {
    float characterWidth = totalTextWidth / float(text.length());
    float characterHeight = characterHeightOverWidth * characterWidth;

    unsigned int vertexCount = 4 * text.length();
    unsigned int indexCount = 6 * text.length();

    Mesh mesh;

    mesh.vertices.resize(vertexCount);
    mesh.indices.resize(indexCount);
    mesh.textureCoordinates.resize(vertexCount);

    for(unsigned int i = 0; i < text.length(); i++)
    {
        float baseXCoordinate = float(i) * characterWidth;

        // Each letter is mapped to 4 vertices.
        mesh.vertices.at(4 * i + 0) = {baseXCoordinate, 0, 0};
        mesh.vertices.at(4 * i + 1) = {baseXCoordinate + characterWidth, 0, 0};
        mesh.vertices.at(4 * i + 2) = {baseXCoordinate + characterWidth, characterHeight, 0};
        mesh.vertices.at(4 * i + 3) = {baseXCoordinate, characterHeight, 0};


        mesh.indices.at(6 * i + 0) = 4 * i + 0;
        mesh.indices.at(6 * i + 1) = 4 * i + 1;
        mesh.indices.at(6 * i + 2) = 4 * i + 2;
        mesh.indices.at(6 * i + 3) = 4 * i + 0;
        mesh.indices.at(6 * i + 4) = 4 * i + 2;
        mesh.indices.at(6 * i + 5) = 4 * i + 3;

        // "For myself":
        // Tex coords represents the pixel-range inside a texture.
        // So imagine this as the textures full normal pixel range.
        // And then you have to normalise it to a 0-1 range value.
        int asciiVal = text[i];
        float uLeft = (asciiVal * characterWidth) / fullAtlasWidth;
        float uRight = ((asciiVal + 1) * characterWidth) / fullAtlasWidth;

        // // I am assuming this is bottomleft, bottomright, topright, topleft?
        mesh.textureCoordinates.at(4 * i + 0) = {uLeft, 0.0};
        mesh.textureCoordinates.at(4 * i + 1) = {uRight, 0.0};
        mesh.textureCoordinates.at(4 * i + 2) = {uRight, 1.0};
        mesh.textureCoordinates.at(4 * i + 3) = {uLeft, 1.0};
    }

    return mesh;
}

Mesh changeTextTextureCoords(std::string text, float characterHeightOverWidth, float totalTextWidth, int VAOID, int IBOID) {
    Mesh newMeshData = generateTextGeometryBuffer(text, characterHeightOverWidth, totalTextWidth);
    if (VAOID == -1 || IBOID == -1) {
        return newMeshData;
    }

    // Bind the VAO so we can target correct VBO
    glBindVertexArray(VAOID);

    // Must also redefine verticy amount, and indicy amount, since each letter is mapped to 4 vertices.
    int verticesVBO;
    int textureCoordsVBO;
    glGetVertexAttribiv(0, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &textureCoordsVBO);
    glGetVertexAttribiv(2, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &textureCoordsVBO);

    // Bind IndexBuffer and give new data.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBOID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, newMeshData.indices.size() * sizeof(unsigned int), newMeshData.indices.data(), GL_DYNAMIC_DRAW);

    // https://learnopengl.com/Advanced-OpenGL/Advanced-Data
    // Bind the vertices VBO.
    glBindBuffer(GL_ARRAY_BUFFER, verticesVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, newMeshData.vertices.size() * sizeof(glm::vec3), newMeshData.vertices.data());

    // Bind the texCoords VBO.
    glBindBuffer(GL_ARRAY_BUFFER, textureCoordsVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, newMeshData.textureCoordinates.size() * sizeof(glm::vec2), newMeshData.textureCoordinates.data());

    return newMeshData;
}