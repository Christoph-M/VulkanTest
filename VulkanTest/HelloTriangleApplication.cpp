#include "HelloTriangleApplication.h"


void HelloTriangleApplication::Run() {
	this->InitWindow();
	this->InitVulkan();
	this->MainLoop();
	this->Cleanup();
}

void HelloTriangleApplication::InitWindow() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	window_ = glfwCreateWindow(WIDTH, HEIGHT, "VulkanTest", nullptr, nullptr);
}


void HelloTriangleApplication::InitVulkan() {
	
}

void HelloTriangleApplication::MainLoop() {
	while (!glfwWindowShouldClose(window_)) {
		glfwPollEvents();
	}
}

void HelloTriangleApplication::Cleanup() {
	glfwDestroyWindow(window_);
	glfwTerminate();
}
