#pragma once

#include "config.h"
#include "util/Logger.h"

#include <glm/glm.hpp>

#define SAFE_DELETE(X) if((X)) delete (X)

#ifdef DEBUG
#define ASSERT(X) if(!(X)) __debugbreak()
#define ASSERT_RUN(X) ASSERT(X)
#define NOT_IMPLEMENTED ASSERT(0)
#else
#define ASSERT(X)
#define ASSERT_RUN(X) X
#define NOT_IMPLEMENTED LOG("Warning, feature not implemented!")
#endif // DEBUG

#define MIN(X,Y) ((X) < (Y)) ? (X) : (Y)
#define MAX(X,Y) ((X) > (Y)) ? (X) : (Y)

using Vec2 = glm::vec2;
using Vec3 = glm::vec3;
using Vec4 = glm::vec4;
using Mat3 = glm::mat3;
using Mat4 = glm::mat4;