#include "program.hpp"
#include <cstdint>
#include <stdexcept>
#include <cstring>
#include <vulkan/vulkan_core.h>
#include <fstream>

struct	SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR		capabilities{};
	std::vector<VkSurfaceFormatKHR>	formats;
	std::vector<VkPresentModeKHR>	presentModes;
};

static SwapChainSupportDetails	querySwapChainSupport(VkPhysicalDevice const & device, VkSurfaceKHR const & surface) {
	SwapChainSupportDetails	swapCHainSupportDetails{};
	uint32_t				surfaceFormatCount = 0;
	uint32_t				presentModeCount = 0;

	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface,
		&swapCHainSupportDetails.capabilities) != VK_SUCCESS)
		throw std::runtime_error("Failed to query physical device surface capabilities");

	if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
		&surfaceFormatCount, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to query physical device surface formats");
	swapCHainSupportDetails.formats.resize(surfaceFormatCount);
	if (vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface,
		&surfaceFormatCount, swapCHainSupportDetails.formats.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to query physical device surface formats");

	if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
		&presentModeCount, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to query physical device surface present modes");
	swapCHainSupportDetails.presentModes.resize(presentModeCount);
	if (vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,
		&presentModeCount, swapCHainSupportDetails.presentModes.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to query physical device surface present modes");

	return (swapCHainSupportDetails);
}

static VkSurfaceFormatKHR	chooseSwapChainSurfaceFormat(std::vector<VkSurfaceFormatKHR> const & availableFormats) {
	for (VkSurfaceFormatKHR const & surfaceFormat : availableFormats) {
		if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_SRGB
			&& surfaceFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return (surfaceFormat);
	}
	return (availableFormats[0]);
}

static VkPresentModeKHR	chooseSwapChainPresentMode(std::vector<VkPresentModeKHR> const & availablePresentModes) {
	for (VkPresentModeKHR const & presentMode : availablePresentModes) {
		if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return (presentMode);
	}

	return (VK_PRESENT_MODE_FIFO_KHR);
}

static VkExtent2D	chooseSwapChainExtent(VkSurfaceCapabilitiesKHR const & capabilities, GLFWwindow * window) {
	if (capabilities.currentExtent.width != UINT32_MAX)
		return (capabilities.currentExtent);
	
	VkExtent2D	extent;
	int			width;
	int			height;
	glfwGetFramebufferSize(window, &width, &height);
	extent.width = static_cast<uint32_t>(width);
	extent.height = static_cast<uint32_t>(height);

	if (extent.width > capabilities.maxImageExtent.width)
		extent.width = capabilities.maxImageExtent.width;
	else if (extent.width < capabilities.minImageExtent.width)
		extent.width = capabilities.minImageExtent.width;

	if (extent.height > capabilities.maxImageExtent.height)
		extent.height = capabilities.maxImageExtent.height;
	else if (extent.height < capabilities.minImageExtent.height)
		extent.height = capabilities.minImageExtent.height;

	return (extent);
}

static bool	checkDeviceExtensionSupport(VkPhysicalDevice const & device, char const * extension) {
	uint32_t	extensionCount;

	if (vkEnumerateDeviceExtensionProperties(device, nullptr,
		&extensionCount, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to query extension properties");
	if (extensionCount == 0)
		return (false);
	std::vector<VkExtensionProperties> extensionProperties(extensionCount);
	if (vkEnumerateDeviceExtensionProperties(device, nullptr,
		&extensionCount, extensionProperties.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to query extension properties");

	for (VkExtensionProperties const & extensionProperty : extensionProperties)
		if (strcmp(extension, extensionProperty.extensionName) == 0)
			return (true);

	return (false);
}

static bool	checkValidationLayerSupport(char const * validationLayerName) {
	uint32_t	layerCount;

	if (vkEnumerateInstanceLayerProperties(&layerCount, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to query instance layer properties");
	if (layerCount == 0)
		return (false);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	if (vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to query instance layer properties");

	for (VkLayerProperties const & layerProperties : availableLayers)
		if (strcmp(validationLayerName, layerProperties.layerName) == 0)
			return (true);

	return (false);
}

static bool	isDeviceSuitable(VkPhysicalDevice const & device, VkSurfaceKHR const & surface, Program::QueueFamilyIndices const & indices) {
	if (!indices.isComplete()) {
		return (false);
	}

	if (!checkDeviceExtensionSupport(device, VK_KHR_SWAPCHAIN_EXTENSION_NAME)) {
		return (false);
	}

	SwapChainSupportDetails	swapChainSupportDetails
		= querySwapChainSupport(device, surface);
	if (!(swapChainSupportDetails.capabilities.supportedUsageFlags
		& VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)) {
		return (false);
	}
	if (!(swapChainSupportDetails.capabilities.supportedCompositeAlpha
		& VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)) {
		return (false);
	}
	if (swapChainSupportDetails.formats.empty()
		|| swapChainSupportDetails.presentModes.empty()) {
		return (false);
	}

	VkFormatProperties	depthFormatProperties{};
	vkGetPhysicalDeviceFormatProperties(device, VK_FORMAT_D32_SFLOAT, &depthFormatProperties);
	if (!(depthFormatProperties.optimalTilingFeatures
		& VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)) {
		return (false);
	}

	return (true);
}

static Program::QueueFamilyIndices	findQueueFamilies(VkPhysicalDevice const & physicalDevice, VkSurfaceKHR const & surface) {
	Program::QueueFamilyIndices	queueFamilyIndices{};
	uint32_t					queueFamilyCount = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice,
		&queueFamilyCount, queueFamilies.data());

	for (size_t i = 0; i < queueFamilyCount; ++i) {
		VkBool32 presentSupport = VK_FALSE;
		if (vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport)
			!= VK_SUCCESS)
			throw std::runtime_error("Failed to query surface support");

		if (presentSupport && !queueFamilyIndices.presentFound) {
			queueFamilyIndices.presentFamily = i;
			queueFamilyIndices.presentFound = true;
		}

		if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT
			&& !queueFamilyIndices.graphicsFound) {
			queueFamilyIndices.graphicsFamily = i;
			queueFamilyIndices.graphicsFound = true;
		}

		if (queueFamilyIndices.isComplete())
			break;
	}
	return (queueFamilyIndices);
}

static std::vector<uint32_t>	readSpirvFile(std::string const & filename) {
	std::ifstream			file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
		throw std::runtime_error("Failed to open shader file");

	std::streamsize const	fileSize = static_cast<std::streamsize>(file.tellg());

	if (fileSize <= 0 || fileSize % sizeof(uint32_t) != 0)
		throw std::runtime_error("Invalid SPIR-V file size");
	
	std::vector<uint32_t>	code(static_cast<size_t>(fileSize) / sizeof(uint32_t));

	file.seekg(0);

	if (!file.read(reinterpret_cast<char *>(code.data()), fileSize))
		throw std::runtime_error("Failed to read shader file");

	return (code);
}

Program::Program() {
	window = nullptr;
	instance = VK_NULL_HANDLE;
	surface = VK_NULL_HANDLE;
	physicalDevice = VK_NULL_HANDLE;
	device = VK_NULL_HANDLE;
	graphicsQueue = VK_NULL_HANDLE;
	presentQueue = VK_NULL_HANDLE;
	swapChain = VK_NULL_HANDLE;
	renderPass = VK_NULL_HANDLE;
}

Program::~Program() {
	for (VkFramebuffer frameBuffer : frameBuffers) {
		vkDestroyFramebuffer(device, frameBuffer, nullptr);
	}
	if (renderPass != VK_NULL_HANDLE) {
		vkDestroyRenderPass(device, renderPass, nullptr);
	}
	for (VkImageView imageView : swapChainImageViews) {
		vkDestroyImageView(device, imageView, nullptr);
	}
	if (swapChain != VK_NULL_HANDLE) {
		vkDestroySwapchainKHR(device, swapChain, nullptr);
	}
	if (device != VK_NULL_HANDLE) {
		vkDestroyDevice(device, nullptr);
	}
	if (surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(instance, surface, nullptr);
	}
	if (instance != VK_NULL_HANDLE) {
		vkDestroyInstance(instance, nullptr);
	}
	if (window) {
		glfwDestroyWindow(window);
	}
	glfwTerminate();
}

void	Program::run() {
	initWindow();
	initVulkan();
	mainLoop();
}

void	Program::mainLoop() {
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
	}
}

void	Program::initWindow() {
	if (!glfwInit())
		throw std::runtime_error("Failed to initialize GLFW");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	window = glfwCreateWindow(WIDTH, HEIGHT, "FIRST", nullptr, nullptr);
	if (!window)
		throw std::runtime_error("Failed to create a window");
}

void	Program::initVulkan() {
	createInstance();
	createSurface();
	pickPhysicalDevice();
	createLogicalDevice();
	createSwapChain();
	createImageViews();
	createRenderPass();
	createFrameBuffers();
}

void	Program::createInstance() {
	uint32_t				glfwExtensionCount;
	char const				**glfwExtensions;
	VkApplicationInfo		appInfo{};
	VkInstanceCreateInfo	createInfo{};
	char const				*validationLayerName;

	validationLayerName = "VK_LAYER_KHRONOS_validation";
	if (enableValidationLayers && !checkValidationLayerSupport(validationLayerName))
		throw std::runtime_error("Validation layer unsupported");

	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Scop";
	appInfo.apiVersion = VK_API_VERSION_1_3;

	glfwExtensionCount = 0;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
	if (glfwExtensionCount == 0 || glfwExtensions == nullptr)
		throw std::runtime_error("Failed to get GLFW Vulkan Extensions");

	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;
	createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;

	if (enableValidationLayers) {
		createInfo.enabledLayerCount = 1;
		createInfo.ppEnabledLayerNames = &validationLayerName;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
	}

	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Vulkan instance");
}

void	Program::createSurface() {
	if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
		throw std::runtime_error("Failed to create window surface");
}

void	Program::pickPhysicalDevice() {
	uint32_t	deviceCount = 0;
	if (vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to query physical devices");
	if (deviceCount == 0)
		throw std::runtime_error("No GPUs with vulkan support");
	std::vector<VkPhysicalDevice> devices(deviceCount);
	if (vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to query physical devices");

	for (VkPhysicalDevice const & device : devices) {
		QueueFamilyIndices	indices = findQueueFamilies(device, surface);
		if (isDeviceSuitable(device, surface, indices)) {
			physicalDevice = device;
			queueFamilyIndices = indices;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("No GPU supports graphics commands");
}

void	Program::createLogicalDevice() {
	float									queuePriority = 1.0f;
	VkPhysicalDeviceFeatures				deviceFeatures{};
	VkDeviceCreateInfo						createInfo{};
	char const								*deviceExtensions[]
		= {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
	std::vector<VkDeviceQueueCreateInfo>	queueCreateInfo{};

	queueCreateInfo.emplace_back(VkDeviceQueueCreateInfo{
		VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		nullptr,
		0,
		queueFamilyIndices.graphicsFamily,
		1,
		&queuePriority});

	if (!queueFamilyIndices.isSame()) {
		queueCreateInfo.emplace_back(VkDeviceQueueCreateInfo{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			queueFamilyIndices.presentFamily,
			1,
			&queuePriority});
	}

	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = queueCreateInfo.size();
	createInfo.pQueueCreateInfos = queueCreateInfo.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = 1;
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
		throw std::runtime_error("Failed to create logical device");

	vkGetDeviceQueue(device, queueFamilyIndices.graphicsFamily, 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilyIndices.presentFamily, 0, &presentQueue);
}

void	Program::createSwapChain() {
	SwapChainSupportDetails		support = querySwapChainSupport(physicalDevice, surface);
	VkSurfaceFormatKHR			surfaceFormat = chooseSwapChainSurfaceFormat(support.formats);
	VkPresentModeKHR			presentMode = chooseSwapChainPresentMode(support.presentModes);
	VkExtent2D					extent = chooseSwapChainExtent(support.capabilities, window);

	uint32_t					imageCount = support.capabilities.minImageCount + 1;
	if (support.capabilities.maxImageCount > 0
		&& imageCount > support.capabilities.maxImageCount)
		imageCount = support.capabilities.maxImageCount;

	uint32_t					indices[]
		= { queueFamilyIndices.graphicsFamily, queueFamilyIndices.presentFamily };
	VkSharingMode				sharingMode;
	if (queueFamilyIndices.graphicsFamily != queueFamilyIndices.presentFamily)
		sharingMode = VK_SHARING_MODE_CONCURRENT;
	else
		sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	swapChainImageFormat = surfaceFormat.format;
	swapChainExtent = extent;
	VkSwapchainCreateInfoKHR	createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	createInfo.imageSharingMode = sharingMode;
	if (sharingMode == VK_SHARING_MODE_CONCURRENT) {
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = indices;
	}
	else {
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = support.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS)
		throw std::runtime_error("Failed to create SwapChain");

	uint32_t					image;
	if (vkGetSwapchainImagesKHR(device, swapChain, &image, nullptr) != VK_SUCCESS)
		throw std::runtime_error("Failed to query swapchain images");
	swapChainImages.resize(image);
	if (vkGetSwapchainImagesKHR(device, swapChain, &image, swapChainImages.data()) != VK_SUCCESS)
		throw std::runtime_error("Failed to query swapchain images");
}

void	Program::createImageViews() {
	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); ++i) {
		VkImageViewCreateInfo	createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = swapChainImages[i];
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = swapChainImageFormat;
		createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create Image View");
	}
}

void	Program::createRenderPass() {
	VkAttachmentDescription	colorAttachment{};

	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference	colorAttachmentRef{};

	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription	subpass{};

	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency		dependency{};

	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo	createInfo{};


	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.attachmentCount = 1;
	createInfo.pAttachments = &colorAttachment;
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpass;
	createInfo.dependencyCount = 1;
	createInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(device, &createInfo, nullptr, &renderPass) != VK_SUCCESS)
		throw std::runtime_error("Failed to create render pass");
}

void	Program::createFrameBuffers() {
	frameBuffers.resize(swapChainImageViews.size());
	for (size_t i = 0; i < frameBuffers.size(); ++i) {
		VkFramebufferCreateInfo	createInfo{};

		createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createInfo.renderPass = renderPass;
		createInfo.width = swapChainExtent.width;
		createInfo.height = swapChainExtent.height;
		createInfo.attachmentCount = 1;
		createInfo.pAttachments = &swapChainImageViews[i];
		createInfo.layers = 1;

		if (vkCreateFramebuffer(device, &createInfo, nullptr, &frameBuffers[i]) != VK_SUCCESS)
			throw std::runtime_error("Failed to create a framebuffer");
	}
}

VkShaderModule	Program::createShaderModule(std::vector<uint32_t> const & code) {
	VkShaderModuleCreateInfo	createInfo{};
	VkShaderModule				shaderModule = VK_NULL_HANDLE;

	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pCode = code.data();
	createInfo.codeSize = code.size() * sizeof(uint32_t);

	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
		throw std::runtime_error("Failed to create Shader Module");
	
	return (shaderModule);
}

void	Program::createGraphicsPipeline() {
	VkPipelineShaderStageCreateInfo			vertexShaderStageCreateInfo{};
	VkPipelineShaderStageCreateInfo			fragmentShaderStageCreateInfo{};

	vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertexShaderStageCreateInfo.module = createShaderModule(readSpirvFile("shaders/shader.vert.spv"));
	vertexShaderStageCreateInfo.pName = "main";

	fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragmentShaderStageCreateInfo.module = createShaderModule(readSpirvFile("shaders/shader.frag.spv"));
	fragmentShaderStageCreateInfo.pName = "main";

	VkPipelineShaderStageCreateInfo 		shaderStagesCreateInfo[]
		= { vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo };

	VkVertexInputBindingDescription			vertexInputBindingDescription{};

	vertexInputBindingDescription.binding = 0;
	vertexInputBindingDescription.stride = sizeof(float) * 3;
	vertexInputBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	VkVertexInputAttributeDescription		vertexInputAttributeDescription{};

	vertexInputAttributeDescription.binding = 0;
	vertexInputAttributeDescription.location = 0;
	vertexInputAttributeDescription.format = VK_FORMAT_R32G32B32_SFLOAT;
	vertexInputAttributeDescription.offset = 0;

	VkPipelineVertexInputStateCreateInfo	vertexInputStateCreateInfo{};

	vertexInputStateCreateInfo.sType
		= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputStateCreateInfo.vertexBindingDescriptionCount = 1;
	vertexInputStateCreateInfo.pVertexBindingDescriptions
		= &vertexInputBindingDescription;
	vertexInputStateCreateInfo.vertexAttributeDescriptionCount = 1;
	vertexInputStateCreateInfo.pVertexAttributeDescriptions =
		&vertexInputAttributeDescription;
	
	VkPipelineInputAssemblyStateCreateInfo	pipelineInputAssemblyStateCreateInfo{};

	pipelineInputAssemblyStateCreateInfo.sType
		= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;

	VkViewport								viewport;

	viewport.x = 0;
	viewport.y = 0;
	viewport.width = swapChainExtent.width;
	viewport.height = swapChainExtent.height;
	viewport.minDepth = 0.0;
	viewport.maxDepth = 1.0;

	VkRect2D								scissorRect;

	scissorRect.extent = swapChainExtent;
	scissorRect.offset.x = 0;
	scissorRect.offset.y = 0;

	VkPipelineViewportStateCreateInfo		viewportStatecreateInfo{};

	viewportStatecreateInfo.sType
		= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStatecreateInfo.viewportCount = 1;
	viewportStatecreateInfo.pViewports = &viewport;
	viewportStatecreateInfo.scissorCount = 1;
	viewportStatecreateInfo.pScissors = &scissorRect;

	VkPipelineRasterizationStateCreateInfo	rasterizationStateCreateInfo{};

	rasterizationStateCreateInfo.sType
		= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;
	rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
	rasterizationStateCreateInfo.lineWidth = 1.0f;

	VkPipelineMultisampleStateCreateInfo	multisampleStateCreateInfo{};

	multisampleStateCreateInfo.sType
		= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleStateCreateInfo.minSampleShading = 0.0f;
	multisampleStateCreateInfo.pSampleMask = nullptr;
	multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;

	VkPipelineDepthStencilStateCreateInfo	depthStencilStateCreateInfo{};

	depthStencilStateCreateInfo.sType
		= VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilStateCreateInfo.depthTestEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthWriteEnable = VK_TRUE;
	depthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.stencilTestEnable = VK_FALSE;
}