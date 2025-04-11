#include "renderStages.hpp"
#include "utilities/window.hpp"
#include <iostream>

void passInAllLights(LightSource* lightSources, int lightSourcesLen, Gloom::Shader &shader)
{
    for (int i = 0; i < lightSourcesLen; i++)
    {
        GLint lightPosLoc = shader.getUniformFromName("lights[" + std::to_string(i) + "].position");
        glUniform3f(lightPosLoc, lightSources[i].position.x, lightSources[i].position.y, lightSources[i].position.z);

        GLint lightColLoc = shader.getUniformFromName("lights[" + std::to_string(i) + "].color");
        glUniform3f(lightColLoc, lightSources[i].color.x, lightSources[i].color.y, lightSources[i].color.z);

        GLint lightIntLoc = shader.getUniformFromName("lights[" + std::to_string(i) + "].intensity");
        glUniform1f(lightIntLoc, lightSources[i].intensity);

        GLint lightTypeLoc = shader.getUniformFromName("lights[" + std::to_string(i) + "].type");
        int type = lightSources[i].lightType == P_LIGHT ? 0 : lightSources[i].lightType == D_LIGHT ? 1 : 2;
        glUniform1i(lightTypeLoc, type);
    }
}

void passInMaterial(Material* material, Gloom::Shader &shader){
    GLint albedoLocation = shader.getUniformFromName("albedo");
    glUniform3f(albedoLocation, material->albedo.r, material->albedo.g, material->albedo.b);

    GLint specularLocation = shader.getUniformFromName("specularFactor");
    glUniform1f(specularLocation, material->specularFactor);

    GLint roughnessLocation = shader.getUniformFromName("roughnessFactor");
    glUniform1f(roughnessLocation, material->roughnessFactor);
}

void checkTextureValid(unsigned int textureID, const char* name) {
    if (textureID == 0) {
        std::cout << "WARNING: Invalid texture ID for " << name << std::endl;
    }
}

void renderNode(SceneNode* node, Gloom::Shader &shader, LightSource* lightSources) {
    GLint subsurfaceLocation = shader.getUniformFromName("isSubsurface");
    glUniform1i(subsurfaceLocation, node->isSubsurface);

    GLint MLocation = shader.getUniformFromName("M");
    glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

    GLint normalMatLoc = shader.getUniformFromName("NM");

    glm::mat3 NM3 = glm::mat3(node->currentTransformationMatrix);
    glm::mat3 NM = glm::transpose(glm::inverse(NM3));
    glUniformMatrix3fv(normalMatLoc, 1, GL_FALSE, glm::value_ptr(NM));

    passInMaterial(node->material, shader);

    if (node->isSubsurface) {
        glBindImageTexture(0, node->subsurfaceFinalTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    }

    switch(node->nodeType) {
        case GEOMETRY:
            if(node->vertexArrayObjectID != -1) {    
                GLint hasTextureLocation = shader.getUniformFromName("hasTexture");
                glUniform1i(hasTextureLocation, 0);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        case GEOMETRY_TEXTURE: {
            if (node->vertexArrayObjectID != -1) {
                GLint hasTextureLocation = shader.getUniformFromName("hasTexture");
                glUniform1i(hasTextureLocation, 1);

                passInMaterial(node->material, shader);

                glBindTextureUnit(1, node->textureID);
                glBindTextureUnit(2, node->normalTextureID);
                glBindTextureUnit(3, node->roughnessTextureID);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
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

void renderSkyboxPass(SceneNode* skyboxNode, Gloom::Shader &shader, LightSource* lightSources, glm::mat4 &VP, unsigned int skyboxFBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, skyboxFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.activate();

    GLint VPLocation = shader.getUniformFromName("VP");
    glUniformMatrix4fv(VPLocation, 1, GL_FALSE, glm::value_ptr(VP));

    passInAllLights(lightSources, 1, shader);

    if (skyboxNode->nodeType == SKYBOX) {
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);

        glBindVertexArray(skyboxNode->vertexArrayObjectID);
        glDrawElements(GL_TRIANGLES, skyboxNode->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
}

void renderSubsurface(SceneNode* node,
                    Gloom::Shader &diffuseShader,
                    Gloom::Shader &thicknessShader,
                    Gloom::Shader &thicknessBiasShader,
                    Gloom::Shader &horizontalShader,
                    Gloom::Shader &verticalShader,
                    LightSource* lightSources,
                    glm::mat4 &VP,
                    glm::vec3 &cameraPosition,
                    unsigned int skyboxFBO)
{
    if (node->isSubsurface == false) return;

    checkTextureValid(node->diffuseTextureID, "diffuseTexture");
    checkTextureValid(node->subsurfaceHorizontalTextureID, "horiTexture");
    checkTextureValid(node->subsurfaceFinalTextureID, "finalTexture");

    // First we do the diffuse pass.
    // We have to copy the skyboxFBO to the diffuseFBO as the initial color.
    // Necessary for compute shaders image processing.
    glBindFramebuffer(GL_READ_FRAMEBUFFER, skyboxFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, node->diffuseFBO);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

    glBindFramebuffer(GL_FRAMEBUFFER, node->diffuseFBO);

    glClear(GL_DEPTH_BUFFER_BIT);

    diffuseShader.activate();

    // Set common uniforms
    GLint VPLocation = diffuseShader.getUniformFromName("VP");
    glUniformMatrix4fv(VPLocation, 1, GL_FALSE, glm::value_ptr(VP));
    
    GLint MLocation = diffuseShader.getUniformFromName("M");
    glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));
    
    // Normal matrix
    glm::mat3 NM = glm::transpose(glm::inverse(glm::mat3(node->currentTransformationMatrix)));
    GLint NMLocation = diffuseShader.getUniformFromName("NM");
    glUniformMatrix3fv(NMLocation, 1, GL_FALSE, glm::value_ptr(NM));
    
    // Camera position for specular
    GLint camPosLocation = diffuseShader.getUniformFromName("cameraPosition");
    glUniform3f(camPosLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    // Pass material properties
    passInMaterial(node->material, diffuseShader);
    
    // Pass light information
    passInAllLights(lightSources, 1, diffuseShader);

    switch (node->nodeType) {
        case GEOMETRY: {
            if(node->vertexArrayObjectID != -1) {
                GLint hasTextureLocation = diffuseShader.getUniformFromName("hasTexture");
                glUniform1i(hasTextureLocation, 0);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        }
        case GEOMETRY_TEXTURE: {
            if(node->vertexArrayObjectID != -1) {
                GLint hasTextureLocation = diffuseShader.getUniformFromName("hasTexture");
                glUniform1i(hasTextureLocation, 1);

                glBindTextureUnit(1, node->textureID);
                glBindTextureUnit(2, node->normalTextureID);
                glBindTextureUnit(3, node->roughnessTextureID);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        }
        default: {
            return;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, node->thicknessFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Thickness pass stage
    thicknessShader.activate();

    // Set common uniforms
    VPLocation = thicknessShader.getUniformFromName("VP");
    glUniformMatrix4fv(VPLocation, 1, GL_FALSE, glm::value_ptr(VP));
    
    MLocation = thicknessShader.getUniformFromName("M");
    glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));
    
    // Normal matrix
    NMLocation = thicknessShader.getUniformFromName("NM");
    glUniformMatrix3fv(NMLocation, 1, GL_FALSE, glm::value_ptr(NM));
    
    // Camera position for specular
    camPosLocation = thicknessShader.getUniformFromName("cameraPosition");
    glUniform3f(camPosLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint hasTextureLocation = diffuseShader.getUniformFromName("hasTexture");
    if (node->nodeType == GEOMETRY_TEXTURE) {
        glUniform1i(hasTextureLocation, 1);

        glBindTextureUnit(2, node->normalTextureID);
    } else {
        glUniform1i(hasTextureLocation, 0);
    }

    // Pass light information
    passInAllLights(lightSources, 1, diffuseShader);

    GLint baseThickLoc = thicknessShader.getUniformFromName("baseThickness");
    glUniform1f(baseThickLoc, node->material->baseThickness);

    GLint materialTypeLoc = thicknessShader.getUniformFromName("materialType");
    glUniform1i(materialTypeLoc, node->material->materialType);

    glBindVertexArray(node->vertexArrayObjectID);
    glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);

    thicknessBiasShader.activate();

    GLint tintLoc = thicknessBiasShader.getUniformFromName("ssTint");
    glUniform3f(tintLoc, node->material->subsurfaceTint.r, node->material->subsurfaceTint.g, node->material->subsurfaceTint.b);

    glBindImageTexture(0, node->diffuseTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, node->thicknessTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);
    glBindImageTexture(2, node->thicknessBiasAlbedoTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    glDispatchCompute((windowWidth + 15) / 16, (windowHeight + 15) / 16, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    // Horizontal Pass stage
    horizontalShader.activate();

    glBindImageTexture(0, node->thicknessBiasAlbedoTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, node->subsurfaceHorizontalTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute((windowWidth + 15) / 16, (windowHeight + 15) / 16, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


    // Vertical pass stage
    verticalShader.activate();

    glBindImageTexture(0, node->subsurfaceHorizontalTextureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, node->subsurfaceFinalTextureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    glDispatchCompute((windowWidth + 15) / 16, (windowHeight + 15) / 16, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_TEXTURE_FETCH_BARRIER_BIT);
}

void renderGbuffer(SceneNode* node, Gloom::Shader &shader, glm::mat4 &V) {
    GLint MLocation = shader.getUniformFromName("M");
    glUniformMatrix4fv(MLocation, 1, GL_FALSE, glm::value_ptr(node->currentTransformationMatrix));

    GLint normalMatLoc = shader.getUniformFromName("NM");

    glm::mat3 NM3 = glm::mat3(V * node->currentTransformationMatrix);
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
        case GEOMETRY_TEXTURE: {
            if (node->vertexArrayObjectID != -1) {
                GLint hasTextureLocation = shader.getUniformFromName("hasTexture");
                glUniform1i(hasTextureLocation, 1);

                glBindTextureUnit(2, node->normalTextureID);

                glBindVertexArray(node->vertexArrayObjectID);
                glDrawElements(GL_TRIANGLES, node->VAOIndexCount, GL_UNSIGNED_INT, nullptr);
            }
            break;
        }
        default: break;
    }

    for(SceneNode* child : node->children) {
        renderGbuffer(child, shader, V);
    }
}

void skyboxStage(SceneNode* skyboxNode, Gloom::Shader &shader, LightSource* lightSources, glm::mat4 &VP, unsigned int skyboxFBO) {
    renderSkyboxPass(skyboxNode, shader, lightSources, VP, skyboxFBO);
}

void subsurfaceStage(SceneNode* rootNode,
    Gloom::Shader &diffuseShader,
    Gloom::Shader &thicknessShader,
    Gloom::Shader &thicknessBiasShader,
    Gloom::Shader &horizontalShader,
    Gloom::Shader &verticalShader,
    LightSource* lightSources,
    glm::mat4 &VP,
    glm::vec3 &cameraPosition,
    unsigned int skyboxFBO)
{
    if (rootNode->isSubsurface) {
        renderSubsurface(rootNode, diffuseShader, thicknessShader, thicknessBiasShader, horizontalShader, verticalShader, lightSources, VP, cameraPosition, skyboxFBO);
    }

    for (SceneNode* child : rootNode->children) {
        subsurfaceStage(child, diffuseShader, thicknessShader, thicknessBiasShader, horizontalShader, verticalShader, lightSources, VP, cameraPosition, skyboxFBO);
    }
}

void main3DStage(SceneNode* rootNode, Gloom::Shader &shader, LightSource* lightSources, glm::mat4 &VP, glm::vec3 &cameraPosition, unsigned int skyboxFBO) {

    // Some processing before activating main pipeline.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, skyboxFBO);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

    shader.activate();

    GLint VPLocation = shader.getUniformFromName("VP");
    glUniformMatrix4fv(VPLocation, 1, GL_FALSE, glm::value_ptr(VP));

    GLint CamPositionLocation = shader.getUniformFromName("cameraPosition");
    glUniform3f(CamPositionLocation, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    passInAllLights(lightSources, 1, shader);

    // Render
    renderNode(rootNode, shader, lightSources);
}

void gbufferStage(SceneNode* rootNode, Gloom::Shader &shader, glm::mat4 &VP, glm::mat4 &V, unsigned int gbufferFBO) {
    glBindFramebuffer(GL_FRAMEBUFFER, gbufferFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.activate();

    GLint VPLocation = shader.getUniformFromName("VP");
    glUniformMatrix4fv(VPLocation, 1, GL_FALSE, glm::value_ptr(VP));

    GLint VLocation = shader.getUniformFromName("V");
    glUniformMatrix4fv(VLocation, 1, GL_FALSE, glm::value_ptr(V));

    renderGbuffer(rootNode, shader, V);
}

void ssaoStage(Gloom::Shader &shader, 
               std::vector<glm::vec3> &kernel, 
               unsigned int noiseTexture, 
               unsigned int resultTexture, 
               unsigned int positionTexture, 
               unsigned int normalTexture,
               glm::mat4 &projection) 
{
    shader.activate();

    glBindImageTexture(0, positionTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(1, normalTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    // glBindImageTexture(2, noiseTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGB32F);
    // glBindTextureUnit(0, positionTexture);
    // glBindTextureUnit(1, normalTexture);
    glBindTextureUnit(2, noiseTexture);

    glBindImageTexture(3, resultTexture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32F);

    GLint projLoc = shader.getUniformFromName("projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    GLint kernelSizeLoc = shader.getUniformFromName("kernelSize");
    glUniform1i(kernelSizeLoc, kernel.size());

    for (unsigned int i = 0; i < kernel.size(); i++) {
        GLint sampleLoc = shader.getUniformFromName("samples[" + std::to_string(i) + "]");
        glUniform3fv(sampleLoc, 1, glm::value_ptr(kernel[i]));
    }

    glDispatchCompute((windowWidth + 15) / 16, (windowHeight + 15) / 16, 1);

    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void main2DStage(Gloom::Shader &shader, SceneNode* rootNode, glm::mat4 &OrthoVP) {
    // Disable Depth test.
    glDisable(GL_DEPTH_TEST);
    shader.activate();

    GLint OrthoVPLocation = shader.getUniformFromName("OrthoVP");
    glUniformMatrix4fv(OrthoVPLocation, 1, GL_FALSE, glm::value_ptr(OrthoVP));

    // Render
    render2DNode(rootNode, shader);

    glEnable(GL_DEPTH_TEST);
} 