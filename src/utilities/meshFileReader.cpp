#include "meshFileReader.hpp"
#include <iostream>
#include <cstring>

// https://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/#loading-the-obj
Mesh loadOBJFile(const char * path) {
    Mesh mesh;

    std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
    
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;

    FILE* file = fopen(path, "r");

    if (file == NULL) {
        std::cout << "Cannot open file. File is NULL" << std::endl;
        return mesh;
    }

    while (1) {
        //Assuming each lineheader is 128 long.
        char lineHeader[128];

        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break;
        
        if(strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            normals.push_back(normal);
        } else if(strcmp(lineHeader, "vt") == 0) {
            glm::vec2 uv;
            fscanf(file, "%f %f\n", &uv.x, &uv.y);
            uvs.push_back(uv);
        } else if (strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            vertices.push_back(vertex);
        } else if(strcmp(lineHeader, "f") == 0) {
            unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
            fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", 
                &vertexIndex[0], &uvIndex[0], &normalIndex[0], 
                &vertexIndex[1], &uvIndex[1], &normalIndex[1], 
                &vertexIndex[2], &uvIndex[2], &normalIndex[2]);

            // -1 because OBJ file is 1 indexed.
            vertexIndices.push_back(vertexIndex[0] - 1);
            vertexIndices.push_back(vertexIndex[1] - 1);
            vertexIndices.push_back(vertexIndex[2] - 1);
            uvIndices.push_back(uvIndex[0] - 1);
            uvIndices.push_back(uvIndex[1] - 1);
            uvIndices.push_back(uvIndex[2] - 1);
            normalIndices.push_back(normalIndex[0] - 1);
            normalIndices.push_back(normalIndex[1] - 1);
            normalIndices.push_back(normalIndex[2] - 1);
        }
    }

    for (int i = 0; i < vertexIndices.size(); i++) {
        glm::vec3 vertex = vertices[vertexIndices[i]];
        mesh.vertices.push_back(vertex);

        glm::vec2 uv = uvs[uvIndices[i]];
        mesh.textureCoordinates.push_back(uv);

        glm::vec3 normal = normals[normalIndices[i]];
        mesh.normals.push_back(normal);

        mesh.indices.push_back(i);
    }

    return mesh;
}