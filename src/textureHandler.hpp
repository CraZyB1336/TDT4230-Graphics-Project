#pragma once

#include "utilities/imageLoader.hpp"
#include <glm/vec3.hpp>

unsigned int getTextureID(PNGImage &image);
unsigned int getEmptyFrameBufferTextureID(int width, int height);
std::vector<int> generateFramebuffer(int width, int height, bool hasDepth);