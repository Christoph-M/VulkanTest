#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>

#include <cstdio>
#include <stdexcept>
#include <functional>


const int WIDTH = 800;
const int HEIGHT = 600;

class HelloTriangleApplication {
public:
	void Run();

private:
	void InitWindow();
	void InitVulkan();
	void MainLoop();
	void Cleanup();

private:
	GLFWwindow* window_;
};