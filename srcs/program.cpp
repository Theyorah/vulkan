#include "program.hpp"
#include <string>
#include <stdexcept>
#include <vector>
#include <cstring>

static bool    checkValidationLayerSupport(char const * validationLayerName) {
    uint32_t    layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (VkLayerProperties const &layerProperties : availableLayers)
        if (strcmp(validationLayerName, layerProperties.layerName) == 0)
            return (true);

    return (false);
}

static bool isDeviceSuitable(VkPhysicalDevice const & device, VkSurfaceKHR const & surface) {
    uint32_t    queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport)
            return (true);
    }

    return (false);
}

static uint32_t getQueueFamilyIndex(VkPhysicalDevice const & device, VkSurfaceKHR const & surface) {
    uint32_t    queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT && presentSupport)
            return (true);
    }

    return (-1);
}

Program::Program() {
    window = nullptr;
    instance = VK_NULL_HANDLE;
    surface = VK_NULL_HANDLE;
    physicalDevice = VK_NULL_HANDLE;
    device = VK_NULL_HANDLE;
    graphicsQueue = VK_NULL_HANDLE;
}

Program::~Program() {
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

void    Program::run() {
    initWindow();
    initVulkan();
    mainLoop();
}

void    Program::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void    Program::initWindow() {
    if (!glfwInit()) {
        char const *    description;
        glfwGetError(&description);
        throw std::runtime_error(
            std::string("Error initializing GLFW: ")
            + description
        );
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "FIRST", nullptr, nullptr);
    if (!window) {
        char const *    description;
        glfwGetError(&description);
        throw std::runtime_error(
            std::string("Error creating GLFW window: ")
            + description
        );
    }
}

void    Program::initVulkan() {
    createInstance();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
}

void    Program::createInstance() {
    uint32_t                glfwExtensionCount;
    char const              **glfwExtensions;
    VkApplicationInfo       appInfo{};
    VkInstanceCreateInfo    createInfo{};
    char const              *validationLayerName;

    validationLayerName = "VK_LAYER_KHRONOS_validation";
    if (enableValidationLayers && !checkValidationLayerSupport(validationLayerName))
        throw std::runtime_error("Validation layer unsupported");

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Scop";
    appInfo.apiVersion = VK_API_VERSION_1_3;

    glfwExtensionCount = 0;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    if (enableValidationLayers) {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &validationLayerName;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
        throw std::runtime_error("Failed to create Vulkan instance");
}

void    Program::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        throw std::runtime_error("Failed to create window surface");
}

void    Program::pickPhysicalDevice() {
    uint32_t    deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (deviceCount == 0)
        throw std::runtime_error("No GPUs with vulkan support");
    std::vector<VkPhysicalDevice>   devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (VkPhysicalDevice const & device : devices) {
        if (isDeviceSuitable(device, surface)) {
            physicalDevice = device;
            break ;
        }
    }

    if (physicalDevice == VK_NULL_HANDLE)
        throw std::runtime_error("No GPU supports graphics commands");
}

void    Program::createLogicalDevice() {
    float                       queuePriority = 1.0f;
    VkDeviceQueueCreateInfo     queueCreateInfo{};
    VkPhysicalDeviceFeatures    deviceFeatures{};
    VkDeviceCreateInfo          createInfo{};
    uint32_t                    queueFamilyIndex;

    queueFamilyIndex = getQueueFamilyIndex(physicalDevice, surface);

    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &queuePriority;

    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device)
        != VK_SUCCESS)
        throw std::runtime_error("Failed to create logical device");

    vkGetDeviceQueue(device, queueFamilyIndex, 0, &graphicsQueue);
}