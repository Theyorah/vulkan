#include "program.hpp"
#include <string>
#include <stdexcept>
#include <vector>
#include <cstring>

static bool    checkValidationLayerSupport(char const *validationLayerName) {
    uint32_t    layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (VkLayerProperties const &layerProperties : availableLayers)
        if (strcmp(validationLayerName, layerProperties.layerName) == 0)
            return (true);

    return (false);
}

Program::Program() {
    window = nullptr;
    instance = VK_NULL_HANDLE;
}

Program::~Program() {
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
}

void    Program::createInstance() {
    uint32_t                glfwExtensionCount;
    char const              **glfwExtensions;
    VkApplicationInfo       appInfo{};
    VkInstanceCreateInfo    createInfo{};
    char const              *validationLayerName;

    validationLayerName = "VK_LAYER_KHRONOS_validation";
    if (enableValidationLayers && !checkValidationLayerSupport(validationLayerName))
        throw std::runtime_error("validation layer unsupported");

    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Scop";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
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

void    Program::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}