#include "program.hpp"
#include <string>
#include <stdexcept>

Program::Program() {
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
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create Vulkan instance");
    }
}

void    Program::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}