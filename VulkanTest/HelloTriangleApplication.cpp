#include "HelloTriangleApplication.h"

#include <set>
#include <string>
#include <algorithm>


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
	this->CreateSurface();
	this->PickPhysicalDevice();
	this->CreateLogicalDevice();
	this->CreateSwapChain();
	this->CreateImageViews();
}

void HelloTriangleApplication::MainLoop() {
	while (!glfwWindowShouldClose(window_)) {
		glfwPollEvents();
	}
}

void HelloTriangleApplication::Cleanup() {
	for (size_t i = 0; i < swapChainImageViews_.size(); ++i) vkDestroyImageView(device_, swapChainImageViews_[i], nullptr);
	vkDestroySwapchainKHR(device_, swapchain_, nullptr);
	vkDestroyDevice(device_, nullptr);
	DestroyDebugReportCallbackEXT(instance_, callback_, nullptr);
	vkDestroySurfaceKHR(instance_, surface_, nullptr);
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
	bool exensionsSupported = this->CheckDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (exensionsSupported) {
		SwapChainSupportDetails swapChainSupport = this->QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}

	return indices.IsComplete() && exensionsSupported && swapChainAdequate;
}

HelloTriangleApplication::QueueFamilyIndices HelloTriangleApplication::FindQueueFamilies(VkPhysicalDevice device) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (int i = 0; i < queueFamilies.size(); ++i) {
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
		if (queueFamilies[i].queueCount > 0 && presentSupport) indices.presentFamily = i;

		if (indices.IsComplete()) break;
	}

	return indices;
}

bool HelloTriangleApplication::CheckDeviceExtensionSupport(VkPhysicalDevice device) {
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

	puts("\tSupported device extensions:");

	for (const auto& extension : availableExtensions) printf("\t\t%s\n", extension.extensionName);

	puts("\tChecking required device extension:");
	for (const char* requiredExtension : deviceExtensions) {
		printf("\t\t%s", requiredExtension);

		int r = 0;
		for (const auto& extensionProperty : availableExtensions) {
			r = strcmp(requiredExtension, extensionProperty.extensionName);
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

HelloTriangleApplication::SwapChainSupportDetails HelloTriangleApplication::QuerySwapChainSupport(VkPhysicalDevice device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);

	uint32_t formatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR HelloTriangleApplication::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED) return { VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR };

	for (const auto& format : availableFormats) if (format.format == VK_FORMAT_B8G8R8A8_UNORM && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return format;

	return availableFormats[0];
}

VkPresentModeKHR HelloTriangleApplication::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes) {
	VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

	for (const auto& presentMode : availablePresentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR) return presentMode;
		if (presentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) bestMode = presentMode;
	}

	return bestMode;
}

VkExtent2D HelloTriangleApplication::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;

	VkExtent2D actualExtent = { WIDTH, HEIGHT };

	actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
	actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

	return actualExtent;
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

void HelloTriangleApplication::CreateSurface() {
	VkResult result = glfwCreateWindowSurface(instance_, window_, nullptr, &surface_);
	printf("glfwCreateWindowSurface result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create window surface!");
}

void HelloTriangleApplication::GetRequiredExtensions(std::vector<const char*>& extensions) {
	unsigned int glfwExtensionCount = 0;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	for (unsigned int i = 0; i < glfwExtensionCount; ++i) extensions.push_back(glfwExtensions[i]);

	if (enableValidationLayers) extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
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
	this->GetRequiredExtensions(requiredExtensions);

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

	for (uint32_t i = 0; i < deviceCount; ++i) {
		printf("Checking suitability of device %d:\n", i);
		if (this->IsDeviceSuitable(devices[i])) {
			physicalDevice_ = devices[i];
			break;
		}
	}

	if (physicalDevice_ == VK_NULL_HANDLE) throw std::runtime_error("Failed to find suitable GPU!");
}

void HelloTriangleApplication::CreateLogicalDevice() {
	QueueFamilyIndices indices = this->FindQueueFamilies(physicalDevice_);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<int> uniqueQueueFamilies = { indices.graphicsFamily, indices.presentFamily };

	float queuePriority = 1.0f;
	for (int queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = { };
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures = { };

	VkDeviceCreateInfo createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	} else {
		createInfo.enabledLayerCount = 0;
	}

	VkResult result = vkCreateDevice(physicalDevice_, &createInfo, nullptr, &device_);
	printf("vkCreateDevice result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create logical device!");

	vkGetDeviceQueue(device_, indices.graphicsFamily, 0, &graphicsQueue_);
	vkGetDeviceQueue(device_, indices.presentFamily, 0, &presentQueue_);
}

void HelloTriangleApplication::CreateSwapChain() {
	SwapChainSupportDetails swapChainSupport = this->QuerySwapChainSupport(physicalDevice_);

	VkSurfaceFormatKHR surfaceFormat = this->ChooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = this->ChooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = this->ChooseSwapExtent(swapChainSupport.capabilities);

	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) imageCount = swapChainSupport.capabilities.maxImageCount;

	QueueFamilyIndices indices = this->FindQueueFamilies(physicalDevice_);
	uint32_t queueFamilyIndices[] = { (uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily };

	VkSwapchainCreateInfoKHR createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface_;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	VkResult result = vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapchain_);
	printf("vkCreateSwapchainKHR result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create swap chain!");

	vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr);
	swapChainImages_.resize(imageCount);
	vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, swapChainImages_.data());
	swapChainImageFormat_ = surfaceFormat.format;
	swapChainExtent_ = extent;
}

void HelloTriangleApplication::CreateImageViews() {
	swapChainImageViews_.resize(swapChainImages_.size());

	for (size_t i = 0; i < swapChainImages_.size(); ++i) {
		VkImageViewCreateInfo createInfo = { };
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages_[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat_;
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		VkResult result = vkCreateImageView(device_, &createInfo, nullptr, &swapChainImageViews_[i]);
		printf("vkCreateImageView %d result: %d\n", (int)i, result);
		if (result != VK_SUCCESS) throw std::runtime_error("Failed to create image views!");
	}
}
