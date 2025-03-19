#include "renderStages.hpp"

void renderNode(SceneNode* node, Gloom::Shader &shader, LightSource* lightSources) {
    GLint MLocation = shader.getUniformFromName("M");
    glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

    GLint normalMatLoc = shader.getUniformFromName("NM");

    glm::mat3 NM3 = glm::mat3(node->currentTransformationMatrix);
    glm::mat3 NM = glm::transpose(glm::inverse(NM3));
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, glm::value_ptr(NM));

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {
                GLint hasTextureLocation = shader.getUniformFromName("hasTexture");
                glUniform1i(hasTextureLocation, 0);
                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case POINT_LIGHT: {
            int i = node->lightSourceID;
            if (i != -1) {
                GLint lightPosLoc = shader.getUniformFromName("lights[" + std::to_string(i) + "].position");
                glUniform3f(lightPosLoc, lightSources[i].position.x, lightSources[i].position.y, lightSources[i].position.z);

                GLint lightColLoc = shader.getUniformFromName("lights[" + std::to_string(i) + "].color");
                glUniform3f(lightColLoc, lightSources[i].color.x, lightSources[i].color.y, lightSources[i].color.z);

                GLint lightIntLoc = shader.getUniformFromName("lights[" + std::to_string(i) + "].intensity");
                glUniform1f(lightIntLoc, lightSources[i].intensity);
            }
            break;
        }
        case SPOT_LIGHT: break;
        case GEOMETRY_TEXTURE: {
            GLint hasTextureLocation = shader.getUniformFromName("hasTexture");
            glUniform1i(hasTextureLocation, 1);

            glBindTextureUnit(1, node->textureID);
            glBindTextureUnit(2, node->normalTextureID);
            glBindTextureUnit(3, node->roughnessTextureID);

            glBindVertexArray(node->vertexArrayObjectID);
            glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            break;
        }
        default: break;
    }

    for(SceneNode* child : node->children) {
        renderNode(child, shader, lightSources);
    }
}

void render2DNode(SceneNode* node, Gloom::Shader &shader) {
    GLint MLocation = shader.getUniformFromName("M");
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
        render2DNode(child, shader);
    }
}

void diffuseBufferStage(Gloom::Shader &shader, SceneNode* rootNode, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition, unsigned int &diffuseFBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, diffuseFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader.activate();

    GLint VPLocation = shader.getUniformFromName("VP");
    glUniformMatrix4fv(VPLocation, 1, GL_FALSE, glm::value_ptr(VP));

    GLint CamPositionLocation = shader.getUniformFromName("cameraPosition");
    glUniform3f(CamPositionLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    // Render
    renderNode(rootNode, shader, lightSources);
}

void subsurfaceHorizontalStage(Gloom::Shader &shader, unsigned int &diffuseSubTextureID, unsigned int &horizontalTextureID, int windowWidth, int windowHeight) {
    shader.activate();

    glBindImageTexture(0, diffuseSubTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, horizontalTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Send out the compute shader! Angarde!
    glDispatchCompute(windowWidth / 16, windowHeight / 16, 1);

    // Ensure the second pass waits for this pass to finish.
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void subsurfaceVerticalStage(Gloom::Shader &shader, unsigned int &horizontalTextureID, unsigned int &subsurfacedTextureID, int windowWidth, int windowHeight) {
    shader.activate();

    glBindImageTexture(0, horizontalTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, subsurfacedTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Send out the compute shader! Angarde!
    glDispatchCompute(windowWidth / 16, windowHeight / 16, 1);

    // Ensure the second pass waits for this pass to finish.
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void main3DStage(Gloom::Shader &shader, SceneNode* rootNode, unsigned int &diffuseTextureID, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition) {
    // Activate Depth test (?)
    glEnable(GL_DEPTH_TEST);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    shader.activate();

    glBindImageTexture(0, diffuseTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    GLint VPLocation = shader.getUniformFromName("VP");
    glUniformMatrix4fv(VPLocation, 1, GL_FALSE, glm::value_ptr(VP));

    GLint CamPositionLocation = shader.getUniformFromName("cameraPosition");
    glUniform3f(CamPositionLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    // Render
    renderNode(rootNode, shader, lightSources);
}

void main2DStage(Gloom::Shader &shader, SceneNode* rootNode, glm::mat4 &OrthoVP) {
    // Disable Depth test.
    glDisable(GL_DEPTH_TEST);
    shader.activate();

    GLint OrthoVPLocation = shader.getUniformFromName("OrthoVP");
    glUniformMatrix4fv(OrthoVPLocation, 1, GL_FALSE, glm::value_ptr(OrthoVP));

    // Render
    render2DNode(rootNode, shader);
} 