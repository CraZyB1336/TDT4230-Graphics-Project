#pragma once

#include "utilities/imageLoader.hpp"
#include <glm/vec3.hpp>

unsigned int getTextureID(PNGImage &image);
unsigned int getEmptyFrameBufferTextureID(int width, int height);
std::vector<int> generateFramebuffer(int width, int height, bool hasDepth);
std::vector<int> generateThicknessBuffer(int width, int height);
std::vector<int> generateGBuffer(int width, int height);
std::vector<glm::vec3> generateSSAOKernel(int kernelSize);
unsigned int generateNoiseTexture();
unsigned int createSSAOTexture(int width, int height);