#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>

#include <cstdio>
#include <stdexcept>
#include <functional>
#include <vector>


const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = {
	"VK_LAYER_LUNARG_standard_validation"
};

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif


class HelloTriangleApplication {
public:
	void Run();

private:
	struct QueueFamilyIndices {
		int graphicsFamily = -1;
		int presentFamily = -1;
		bool IsComplete() { return graphicsFamily >= 0 && presentFamily >= 0; }
	};

private:
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugReportFlagsEXT flags,
		VkDebugReportObjectTypeEXT objType,
		uint64_t obj,
		size_t location,
		int32_t code,
		const char* layerPrefix,
		const char* msg,
		void* userData) {
		
		printf("Validation layer: %s\n", msg);
		return VK_FALSE;
	}

	static VkResult CreateDebugReportCallbackEXT(VkInstance instance, const VkDebugReportCallbackCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugReportCallbackEXT* pCallback) {
		auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT");
		if (func) return func(instance, pCreateInfo, pAllocator, pCallback);
		else return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	static void DestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback, const VkAllocationCallbacks* pAllocator) {
		auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT");
		if (func) func(instance, callback, pAllocator);
	}

private:
	void InitWindow();
	void InitVulkan();
	void MainLoop();
	void Cleanup();

	bool CheckValidationLayerSupport();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

	void SetupDebugCallback();
	void CreateSurface();
	void GetRequiredExtensions(std::vector<const char*>* extensions);
	void CreateInstance();
	void PickPhysicalDevice();
	void CreateLogicalDevice();

private:
	GLFWwindow* window_;

	VkInstance instance_;
	VkDebugReportCallbackEXT callback_;
	VkSurfaceKHR surface_;
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDevice device_;
	VkQueue graphicsQueue;
	VkQueue presentQueue;
};