#include "HelloTriangleApplication.h"

#include <set>
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
	this->CreateRenderPass();
	this->CreateGraphicsPipeline();
	this->CreateFramebuffers();
	this->CreateCommandPool();
	this->CreateCommandBuffers();
}

void HelloTriangleApplication::MainLoop() {
	while (!glfwWindowShouldClose(window_)) {
		glfwPollEvents();
	}
}

void HelloTriangleApplication::Cleanup() {
	vkDestroyCommandPool(device_, commandPool_, nullptr);
	for (size_t i = 0; i < swapChainFramebuffers_.size(); ++i) vkDestroyFramebuffer(device_, swapChainFramebuffers_[i], nullptr);
	vkDestroyPipeline(device_, graphicsPipeline_, nullptr);
	vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr);
	vkDestroyRenderPass(device_, renderPass_, nullptr);
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

VkShaderModule HelloTriangleApplication::CreateShaderModule(const std::vector<char>& code) {
	std::vector<uint32_t> codeAligned(code.size() / sizeof(uint32_t) + 1);
	memcpy(codeAligned.data(), code.data(), code.size());

	VkShaderModuleCreateInfo createInfo = { };
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = codeAligned.data();

	VkShaderModule shaderModule;
	VkResult result = vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule);
	printf("vkCreateShaderModule result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create shader module!");

	return shaderModule;
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
	uint32_t queueFamilyIndices[] = { static_cast<uint32_t>(indices.graphicsFamily), static_cast<uint32_t>(indices.presentFamily) };

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
		printf("vkCreateImageView %d result: %d\n", static_cast<int>(i), result);
		if (result != VK_SUCCESS) throw std::runtime_error("Failed to create image views!");
	}
}

void HelloTriangleApplication::CreateRenderPass() {
	VkAttachmentDescription colorAttachment = { };
	colorAttachment.format = swapChainImageFormat_;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = { };
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = { };
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkRenderPassCreateInfo renderPassInfo = { };
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 1;
	renderPassInfo.pAttachments = &colorAttachment;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;

	VkResult result = vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass_);
	printf("vkCreateRenderPass result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to cerate render pass!");
}

void HelloTriangleApplication::CreateGraphicsPipeline() {
	std::vector<char> vertShaderCode, fragShaderCode;
	this->ReadFile("CompiledShaders/vert.spv", vertShaderCode);
	this->ReadFile("CompiledShaders/frag.spv", fragShaderCode);
	printf("vertShaderCode size: %d\n", static_cast<int>(vertShaderCode.size()));
	printf("fragShaderCode size: %d\n", static_cast<int>(fragShaderCode.size()));

	VkShaderModule vertShaderModule = this->CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = this->CreateShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo = { };
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo = { };
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 0;
	vertexInputInfo.pVertexBindingDescriptions = nullptr;
	vertexInputInfo.vertexAttributeDescriptionCount = 0;
	vertexInputInfo.pVertexAttributeDescriptions = nullptr;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = { };
	inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport viewport = { };
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent_.width);
	viewport.height = static_cast<float>(swapChainExtent_.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = { };
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent_;

	VkPipelineViewportStateCreateInfo viewportStateInfo = { };
	viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.pViewports = &viewport;
	viewportStateInfo.scissorCount = 1;
	viewportStateInfo.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizerInfo = { };
	rasterizerInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizerInfo.depthClampEnable = VK_FALSE;
	rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizerInfo.lineWidth = 1.0f;
	rasterizerInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizerInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizerInfo.depthBiasClamp = VK_FALSE;
	rasterizerInfo.depthBiasConstantFactor = 0.0f;
	rasterizerInfo.depthBiasClamp = 0.0f;
	rasterizerInfo.depthBiasSlopeFactor = 0.0f;

	VkPipelineMultisampleStateCreateInfo multisamplingInfo = { };
	multisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisamplingInfo.sampleShadingEnable = VK_FALSE;
	multisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisamplingInfo.minSampleShading = 1.0f;
	multisamplingInfo.pSampleMask = nullptr;
	multisamplingInfo.alphaToCoverageEnable = VK_FALSE;
	multisamplingInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineColorBlendAttachmentState colorBlendAttachment = { };
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlendingInfo = { };
	colorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendingInfo.logicOpEnable = VK_FALSE;
	colorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
	colorBlendingInfo.attachmentCount = 1;
	colorBlendingInfo.pAttachments = &colorBlendAttachment;
	colorBlendingInfo.blendConstants[0] = 0.0f;
	colorBlendingInfo.blendConstants[1] = 0.0f;
	colorBlendingInfo.blendConstants[2] = 0.0f;
	colorBlendingInfo.blendConstants[3] = 0.0f;

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_LINE_WIDTH
	};

	VkPipelineDynamicStateCreateInfo dynamicStateInfo = { };
	dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicStateInfo.dynamicStateCount = 2;
	dynamicStateInfo.pDynamicStates = dynamicStates;

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = { };
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0;
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;

	VkResult result = vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_);
	printf("vkCreatePipelineLayout result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create pipeline layout!");

	VkGraphicsPipelineCreateInfo pipelineInfo = { };
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssemblyInfo;
	pipelineInfo.pViewportState = &viewportStateInfo;
	pipelineInfo.pRasterizationState = &rasterizerInfo;
	pipelineInfo.pMultisampleState = &multisamplingInfo;
	pipelineInfo.pDepthStencilState = nullptr;
	pipelineInfo.pColorBlendState = &colorBlendingInfo;
	pipelineInfo.pDynamicState = nullptr;
	pipelineInfo.layout = pipelineLayout_;
	pipelineInfo.renderPass = renderPass_;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;

	result = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_);
	printf("vkCreateGraphicsPipelines result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create graphics pipeline!");

	vkDestroyShaderModule(device_, fragShaderModule, nullptr);
	vkDestroyShaderModule(device_, vertShaderModule, nullptr);
}

void HelloTriangleApplication::CreateFramebuffers() {
	swapChainFramebuffers_.resize(swapChainImageViews_.size());

	for (size_t i = 0; i < swapChainImageViews_.size(); ++i) {
		VkImageView attachments[] = { swapChainImageViews_[i] };

		VkFramebufferCreateInfo framebufferInfo = { };
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass_;
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapChainExtent_.width;
		framebufferInfo.height = swapChainExtent_.height;
		framebufferInfo.layers = 1;

		VkResult result = vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &swapChainFramebuffers_[i]);
		printf("vkCreateFramebuffer %d result: %d\n", static_cast<int>(i), result);
		if (result != VK_SUCCESS) throw std::runtime_error("Failed to create framebuffer!");
	}
}

void HelloTriangleApplication::CreateCommandPool() {
	QueueFamilyIndices queueFamilyIndices = this->FindQueueFamilies(physicalDevice_);

	VkCommandPoolCreateInfo poolInfo = { };
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
	poolInfo.flags = 0;

	VkResult result = vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_);
	printf("vkCreateCommandPool result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create command pool!");
}

void HelloTriangleApplication::CreateCommandBuffers() {
	commandBuffers_.resize(swapChainFramebuffers_.size());

	VkCommandBufferAllocateInfo allocateInfo = { };
	allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInfo.commandPool = commandPool_;
	allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

	VkResult result = vkAllocateCommandBuffers(device_, &allocateInfo, commandBuffers_.data());
	printf("vkAllocateCommandBuffers result: %d\n", result);
	if (result != VK_SUCCESS) throw std::runtime_error("Failed to create command buffers!");

	for (size_t i = 0; i < commandBuffers_.size(); ++i) {
		VkCommandBufferBeginInfo beginInfo = { };
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		vkBeginCommandBuffer(commandBuffers_[i], &beginInfo);

		VkClearValue clearColor = { 0.0f, 0.0f, 0.0f, 1.0f };
		VkRenderPassBeginInfo renderPassInfo = { };
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = renderPass_;
		renderPassInfo.framebuffer = swapChainFramebuffers_[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = swapChainExtent_;
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(commandBuffers_[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdBindPipeline(commandBuffers_[i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_);
		vkCmdDraw(commandBuffers_[i], 3, 1, 0, 0);
		vkCmdEndRenderPass(commandBuffers_[i]);

		result = vkEndCommandBuffer(commandBuffers_[i]);
		printf("vkEndCommandBuffer %d result: %d\n", static_cast<int>(i), result);
		if (result != VK_SUCCESS) throw std::runtime_error("Failed to record command buffer!");
	}
}
