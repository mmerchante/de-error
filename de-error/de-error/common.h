#pragma once

#define SFML_STATIC
#include <SFML/Graphics.hpp>
#include <SFML/Main.hpp>

#define GLM_FORCE_RADIANS

#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <glm/trigonometric.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/constants.hpp>

#include "pcg32.h"
#include <thread>
#include <chrono>
#include <iostream>
#include <iomanip>

#ifdef DOUBLE_PRECISION
	typedef glm::float64 Float;
	typedef glm::dvec2 Vector2;
	typedef glm::dvec3 Vector3;
#else
	typedef glm::float32 Float;
	typedef glm::vec2 Vector2;
	typedef glm::vec3 Vector3;
#endif

	typedef glm::vec3 Color;