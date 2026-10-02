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
    void                        createSurface();
    void                        pickPhysicalDevice();
    void                        createLogicalDevice();

    GLFWwindow *                window;
    static int constexpr        WIDTH = 800;
    static int constexpr        HEIGHT = 600;

    #ifdef NDEBUG
        bool const                  enableValidationLayers = false;
    #else
        bool const                  enableValidationLayers = true;
    #endif

    VkInstance                  instance;
    VkSurfaceKHR                surface;
    VkPhysicalDevice            physicalDevice;
    VkDevice                    device;
    VkQueue                     graphicsQueue;

};

#endif