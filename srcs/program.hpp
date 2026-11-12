#ifndef PROGRAM
# define PROGRAM
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class   Program {

public:

	struct	QueueFamilyIndices {
		uint32_t	graphicsFamily = 0;
		uint32_t	presentFamily = 0;

		bool		graphicsFound = false;
		bool		presentFound = false;

		bool	isComplete() const {
			return (graphicsFound && presentFound);
		}

		bool	isSame() const {
			return (isComplete() && graphicsFamily == presentFamily);
		}
	};

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
	void						createSwapChain();
	void						createImageViews();
	void						createRenderPass();
	void						createFrameBuffers();

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
	QueueFamilyIndices			queueFamilyIndices;
    VkDevice                    device;
    VkQueue                     graphicsQueue;
	VkQueue						presentQueue;
	VkSwapchainKHR				swapChain;
	VkFormat					swapChainImageFormat{};
	VkExtent2D					swapChainExtent{};
	std::vector<VkImage>		swapChainImages;
	std::vector<VkImageView>	swapChainImageViews;
	VkRenderPass				renderPass;
	std::vector<VkFramebuffer>	frameBuffers;
};

#endif
