#include "program.hpp"
#include <string>
#include <stdexcept>

Program::Program() {
}

Program::~Program() {
    glfwTerminate();
}

void    Program::run() {
    initWindow();
    mainLoop();
    cleanup();
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

void    Program::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
    }
}

void    Program::cleanup() {
    glfwDestroyWindow(window);
    glfwTerminate();
}