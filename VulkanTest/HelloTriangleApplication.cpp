#include "HelloTriangleApplication.h"

#include <vector>


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
	CreateInstance();
}

void HelloTriangleApplication::MainLoop() {
	while (!glfwWindowShouldClose(window_)) {
		glfwPollEvents();
	}
}

void HelloTriangleApplication::Cleanup() {
	vkDestroyInstance(instance_, nullptr);
	glfwDestroyWindow(window_);
	glfwTerminate();
}


void HelloTriangleApplication::CreateInstance() {
	VkApplicationInfo appInfo = { };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	VkInstanceCreateInfo createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
	createInfo.enabledLayerCount = 0;

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
	printf("vkCreateInstance result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create instance!");

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	puts("Available extensions:");

	for (const auto& extension : extensions) {
		printf("\t%s\n", extension.extensionName);
	}

	puts("Checking required extensions:");

	for (int i = 0; i < glfwExtensionCount; ++i) {
		printf("\t%s", glfwExtensions[i]);

		int r = 0;
		for (const auto& extension : extensions) {
			r = strcmp(glfwExtensions[i], extension.extensionName);
			if (!r) {
				puts(" -> Found!");
				break;
			}
		}

		if (r) {
			puts(" -> Not supported!");
			throw std::runtime_error("Required extension not supported!");
		}
	}
}
