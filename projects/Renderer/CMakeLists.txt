CMAKE_MINIMUM_REQUIRED(VERSION 3.12)

if( UNIX )
	#set(CMAKE_CXX_COMPILER "/usr/bin/c++")
	#set(CMAKE_C_COMPILER "gcc-9")
	#set(CMAKE_CXX_COMPILER "/usr/bin/g++-9")
endif()

PROJECT(Vk_renderer
	VERSION 1.0
	DESCRIPTION "Low level Vulkan renderer"
	LANGUAGES CXX
	)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_GENERATOR_PLATFORM x64)
FIND_PACKAGE(Vulkan REQUIRED)

MESSAGE(STATUS "Project: " ${PROJECT_NAME})

ADD_DEFINITIONS(
	-std=c++17
	-O2		# Optimization incompatible with Release mode
	#-O3
)
#ADD_COMPILE_DEFINITIONS( IMGUI_IMPL_OPENGL_LOADER_GLEW=1 )
#ADD_COMPILE_DEFINITIONS( IMGUI_IMPL_OPENGL_LOADER_GLAD=1 )

ADD_LIBRARY(${PROJECT_NAME} STATIC
	src/main.cpp

	src/environment.cpp
	src/renderer.cpp
	src/models.cpp
	src/vertex.cpp
	src/importer.cpp
	src/ubo.cpp
	src/input.cpp
	src/timer.cpp
	src/toolkit.cpp
	src/physics.cpp
	src/commons.cpp
	src/ECSarch.cpp

	include/renderer.hpp
	include/environment.hpp
	include/models.hpp
	include/vertex.hpp
	include/importer.hpp
	include/ubo.hpp
	include/input.hpp
	include/timer.hpp
	include/toolkit.hpp
	include/physics.hpp
	include/commons.hpp
	include/ECSarch.hpp

	../../Readme.md
	TODO.txt
	CMakeLists.txt
)

TARGET_INCLUDE_DIRECTORIES( ${PROJECT_NAME} PUBLIC
	include

	C:/VulkanSDK/1.3.211.0/Include
	../../extern/glfw/glfw-3.3.2/include
	../../extern/glm/glm-0.9.9.5
	../../extern/stb
	../../extern/tinyobjloader
	../../extern/assimp/include
	../../_BUILD/extern/assimp/include/
	../../extern/BulletPhysics/bullet3/src
	#../../extern/vulkansdk-linux-x86_64-1.2.170.0/1.2.170.0/x86_64/include
	#../../extern/imgui/imgui-1.72b
	#../../extern/eigen-3.3.8
)








