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
	this->CreateInstance();
	this->SetupDebugCallback();
	this->PickPhysicalDevice();
}

void HelloTriangleApplication::MainLoop() {
	while (!glfwWindowShouldClose(window_)) {
		glfwPollEvents();
	}
}

void HelloTriangleApplication::Cleanup() {
	DestroyDebugReportCallbackEXT(instance_, callback_, nullptr);
	vkDestroyInstance(instance_, nullptr);
	glfwDestroyWindow(window_);
	glfwTerminate();
}


bool HelloTriangleApplication::CheckValidationLayerSupport() {
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	puts("Supported validation layers:");

	for (const auto& layerProperties : availableLayers) printf("\t%s\n", layerProperties.layerName);

	puts("Checking required validation layers:");
	for (const char* layerName : validationLayers) {
		printf("\t%s", layerName);

		int r = 0;
		for (const auto& layerProperties : availableLayers) {
			r = strcmp(layerName, layerProperties.layerName);
			if (!r) {
				puts(" -> Found!");
				break;
			}
		}

		if (r) {
			puts(" -> Not supported!");
			return false;
		}
	}

	return true;
}

bool HelloTriangleApplication::IsDeviceSuitable(VkPhysicalDevice device) {
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;
	vkGetPhysicalDeviceProperties(device, &deviceProperties);
	vkGetPhysicalDeviceFeatures(device, &deviceFeatures);
	QueueFamilyIndices indices = this->FindQueueFamilies(device);

	return indices.IsComplete();
}

HelloTriangleApplication::QueueFamilyIndices HelloTriangleApplication::FindQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (int i = 0; i < queueFamilies.size(); ++i) {
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
		if (indices.IsComplete()) break;
	}

	return indices;
}


void HelloTriangleApplication::SetupDebugCallback() {
	if (!enableValidationLayers) return;

	VkDebugReportCallbackCreateInfoEXT createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	createInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	createInfo.pfnCallback = this->DebugCallback;
	createInfo.pUserData = this;

	VkResult result = CreateDebugReportCallbackEXT(instance_, &createInfo, nullptr, &callback_);
	printf("CreateDebugReportCallbackEXT result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to set up debug callback!");
}


void HelloTriangleApplication::GetRequiredExtensions(std::vector<const char*>* extensions) {
	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; ++i) extensions->push_back(glfwExtensions[i]);

	if (enableValidationLayers) extensions->push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
}


void HelloTriangleApplication::CreateInstance() {
	if (enableValidationLayers && !this->CheckValidationLayerSupport()) throw std::runtime_error("Required validation layer not supported!");

	VkApplicationInfo appInfo = { };
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	std::vector<const char*> requiredExtensions;
	this->GetRequiredExtensions(&requiredExtensions);

	VkInstanceCreateInfo createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
	createInfo.ppEnabledExtensionNames = requiredExtensions.data();
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
	printf("vkCreateInstance result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create instance!");

	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

	puts("Supported extensions:");
	for (const auto& extension : availableExtensions) printf("\t%s\n", extension.extensionName);

	puts("Checking required extensions:");
	for (const auto& extensionName : requiredExtensions) {
		printf("\t%s", extensionName);

		int r = 0;
		for (const auto& extension : availableExtensions) {
			r = strcmp(extensionName, extension.extensionName);
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

void HelloTriangleApplication::PickPhysicalDevice() {
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
	if (deviceCount == 0) throw std::runtime_error("Failed to find GPUs with Vulkan support!");
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

	for (const auto& device : devices) {
		if (this->IsDeviceSuitable(device)) {
			physicalDevice_ = device;
			break;
		}
	}

	if (physicalDevice_ == VK_NULL_HANDLE) throw std::runtime_error("Failed to find suitable GPU!");
}
