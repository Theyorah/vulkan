#ifndef PROGRAM
# define PROGRAM
# define GLFW_INCLUDE_VULKAN
# include <GLFW/glfw3.h>

class   Program {

public:

    Program();
    ~Program();

    void    run();

private:

    void                        initWindow();
    void                        initVulkan();
    void                        mainLoop();
    void                        createInstance();

    GLFWwindow *                window = nullptr;
    static int constexpr        WIDTH = 800;
    static int constexpr        HEIGHT = 600;

    VkInstance                  instance = VK_NULL_HANDLE;

};

#endif