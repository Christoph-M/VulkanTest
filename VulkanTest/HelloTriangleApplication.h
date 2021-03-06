#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW\glfw3.h>

#include <cstdio>
#include <stdexcept>
#include <functional>
#include <vector>
#include <fstream>


const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

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

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
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

	static void ReadFile(const std::string& filename, std::vector<char>& buffer) {
		std::ifstream file(filename, std::ios::ate | std::ios::binary);
		if (!file.is_open()) throw std::runtime_error("Failed to open file!");

		size_t fileSize = static_cast<size_t>(file.tellg());
		buffer.resize(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
	}

	static void OnWindowResized(GLFWwindow* window, int width, int height) {
		if (width == 0 || height == 0) return;

		HelloTriangleApplication* app = reinterpret_cast<HelloTriangleApplication*>(glfwGetWindowUserPointer(window));
		app->RecreateSwapChain();
	}

private:
	void InitWindow();
	void InitVulkan();
	void MainLoop();
	void CleanupSwapChain();
	void Cleanup();

	void DrawFrame();
	void RecreateSwapChain();

	bool CheckValidationLayerSupport();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	VkShaderModule CreateShaderModule(const std::vector<char>& code);

	void SetupDebugCallback();
	void CreateSurface();
	void GetRequiredExtensions(std::vector<const char*>& extensions);
	void CreateInstance();
	void PickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateRenderPass();
	void CreateGraphicsPipeline();
	void CreateFramebuffers();
	void CreateCommandPool();
	void CreateCommandBuffers();
	void CreateSemaphores();

private:
	GLFWwindow* window_;

	VkInstance instance_;
	VkDebugReportCallbackEXT callback_;
	VkSurfaceKHR surface_;
	VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
	VkDevice device_;
	VkQueue graphicsQueue_;
	VkQueue presentQueue_;
	VkSwapchainKHR swapchain_;
	std::vector<VkImage> swapChainImages_;
	VkFormat swapChainImageFormat_;
	VkExtent2D swapChainExtent_;
	std::vector<VkImageView> swapChainImageViews_;
	VkRenderPass renderPass_;
	VkPipelineLayout pipelineLayout_;
	VkPipeline graphicsPipeline_;
	std::vector<VkFramebuffer> swapChainFramebuffers_;
	VkCommandPool commandPool_;
	std::vector<VkCommandBuffer> commandBuffers_;
	
	VkSemaphore imageAvailableSemaphore_;
	VkSemaphore renderFinishedSemaphore_;
};