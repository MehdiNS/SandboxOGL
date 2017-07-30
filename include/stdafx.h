#pragma once

// Because windows.h ...
#define WIN32_LEAN_AND_MEAN

// GLAD
#ifdef _WIN32
#define APIENTRY __stdcall
#endif
#include <glad\glad.h>


// GLFW
#include <glfw/glfw3.h>
#ifdef _WIN32
#undef APIENTRY
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#include <glfw/glfw3native.h>
#endif

// Standard library
#ifdef _DEBUG
#include <iostream>
#endif
#include <fstream>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <array>	
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

// Assimp
#include <assimp/Importer.hpp> 
#include <assimp/scene.h>     
#include <assimp/postprocess.h>
#include <assimp/cimport.h>

// GLM
#define GLM_FORCE_SWIZZLE 
#define GLM_FORCE_SSE2
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/matrix_interpolation.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/norm.hpp>

// GLI
#include <gli/gli.hpp>

// Imgui
#include <imgui/imgui.h>
#include "imgui_impl_glfw_gl3.h"

// Useful files
#include "Types.h"
#include "Uncopiable.h"