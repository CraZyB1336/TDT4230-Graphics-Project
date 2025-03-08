#pragma once

#include <string>
#include "mesh.h"

Mesh generateTextGeometryBuffer(std::string text, float characterHeightOverWidth, float totalTextWidth);

Mesh changeTextTextureCoords(std::string text, float characterHeightOverWidth, float totalTextWidth, int VAOID, int IBOID);
