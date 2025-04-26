#include "base/base_include.h"
#include "base/base_include.cpp"

#define VK_USE_PLATFORM_WAYLAND_KHR
#define GLFW_INCLUDE_VULKAN
#define GLFW_EXPOSE_NATIVE_WAYLAND
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#define WIDTH 600
#define HEIGHT 600

static u64 RecreateCount = {};

static arena* GlobalArena = {};
static arena* TestDeletionArena = {};
static arena* ShaderArena = {};

#if VK_VALIDATE
    #define VALIDATION_LAYERS X(VK_LAYER_KHRONOS_validation)
    #define VK_REQUIRED_INSTANCE_EXTENSIONS \
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#else
    #define VALIDATION_LAYERS
    #define VK_REQUIRED_INSTANCE_EXTENSIONS VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
#endif

#define VK_REQUIRED_DEVICE_EXTENSIONS \
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME

typedef struct vulkan_context {
    VkInstance Instance;
#if VK_VALIDATE
    VkDebugUtilsMessengerEXT MessengerHandle;
#endif
    VkPhysicalDevice PhysicalDevice;
    VkQueue GraphicsQueue;
    VkDevice Device;
    VkSurfaceKHR Surface;
    VkSurfaceFormatKHR SurfaceFormat;
    VkPresentModeKHR PresentMode;
    u32 ImageCount;
    u32 GraphicsQueueIndex;
    VkExtent2D SwapchainExtent;
    VkSwapchainKHR Swapchain;
    VkImage* Images;
    VkImageView* ImageViews;
    VkShaderModule VertShader;
    VkShaderModule FragShader;
    VkPipelineLayout PipelineLayout;
    VkPipeline GraphicsPipeline;
    VkSurfaceCapabilitiesKHR Capabilities;
    VkCommandPool CommandPool;
    VkCommandBuffer CommandBuffer;
    VkCommandPool TransientCommandPool;
    VkCommandBuffer TransientCommandBuffer;
    VkSemaphore ImageSemaphore;
    VkSemaphore RenderSemaphore;
    VkFence InFlightFence;
    GLFWwindow* Window;
    b32 ShouldResize;
    VkPhysicalDeviceMemoryProperties MemoryProperties;

    VkBuffer VertexBuffer;
    VkDeviceMemory VertexBufferMemory;
    VkBuffer StagingBuffer;
    VkDeviceMemory StagingBufferMemory;

    //    VkBuffer IndexBuffer;
    //    VkDeviceMemory IndexBufferMemory;

} vulkan_context;

typedef struct tutorial_vertex {
    v3 Position;
    v3 Color;
} tutorial_vertex;

typedef struct uniform_buffer {
    m4 Model;
    m4 View;
    m4 Projection;
} uniform_buffer;

typedef enum vulkan_item {
    VULKANITEM_Device,
    VULKANITEM_Memory,
    VULKANITEM_Buffer,
    VULKANITEM_Sempahore,
    VULKANITEM_Fence,
    VULKANITEM_CommandPool,
    VULKANITEM_ShaderModule,
    VULKANITEM_Pipeline,
    VULKANITEM_PipelineLayout,
    VULKANITEM_ImageView,
    VULKANITEM_Surface,
    VULKANITEM_Swapchain,
    VULKANITEM_DebugMessenger,
    VULKANITEM_Insance,
} vulkan_item;

typedef struct vulkan_deletion_node {
    void* Item;
    vulkan_item Type;
    vulkan_deletion_node* Next;
} vulkan_deletion_node;

typedef struct vulkan_deletion_queue {
    vulkan_deletion_node* Last = 0;

    u64 Count = 0;
} vulkan_deletion_queue;

typedef struct push_constants {
    v2 Resolution;
    v2 MousPos;
    f32 Time;
} push_constants;

static vulkan_deletion_queue* DeletionQueue = {};

static void VulkanDeletionQueuePush(arena* __restrict Arena,
                                    vulkan_deletion_queue* __restrict Queue, void* __restrict Item,
                                    vulkan_item Type) {
    vulkan_deletion_node* NewNode =
        (vulkan_deletion_node*)ArenaPush(Arena, sizeof(vulkan_deletion_node));
    NewNode->Item = Item;
    NewNode->Type = Type;
    SllStackPush(Queue->Last, NewNode);
    ++Queue->Count;
}

static void VulkanDeletionQueuePop(vulkan_context* VkCtx, vulkan_deletion_queue* Queue) {
    Assert(Queue->Count > 0);
    vulkan_deletion_node* Node = Queue->Last;
    switch (Node->Type) {
        case VULKANITEM_Device: {
            vkDestroyDevice(*(VkDevice*)Node->Item, 0);
        } break;
        case VULKANITEM_Memory: {
            vkFreeMemory(VkCtx->Device, *(VkDeviceMemory*)Node->Item, 0);
        } break;
        case VULKANITEM_Buffer: {
            vkDestroyBuffer(VkCtx->Device, *(VkBuffer*)Node->Item, 0);
        } break;
        case VULKANITEM_Sempahore: {
            vkDestroySemaphore(VkCtx->Device, *(VkSemaphore*)Node->Item, 0);
        } break;
        case VULKANITEM_Fence: {
            vkDestroyFence(VkCtx->Device, *(VkFence*)Node->Item, 0);
        } break;
        case VULKANITEM_CommandPool: {
            vkDestroyCommandPool(VkCtx->Device, *(VkCommandPool*)Node->Item, 0);
        } break;
        case VULKANITEM_ShaderModule: {
            vkDestroyShaderModule(VkCtx->Device, *(VkShaderModule*)Node->Item, 0);
        } break;
        case VULKANITEM_Pipeline: {
            vkDestroyPipeline(VkCtx->Device, *(VkPipeline*)Node->Item, 0);
        } break;
        case VULKANITEM_PipelineLayout: {
            vkDestroyPipelineLayout(VkCtx->Device, *(VkPipelineLayout*)Node->Item, 0);
        } break;
        case VULKANITEM_ImageView: {
            vkDestroyImageView(VkCtx->Device, *(VkImageView*)Node->Item, 0);
        } break;
        case VULKANITEM_Swapchain: {
            vkDestroySwapchainKHR(VkCtx->Device, *(VkSwapchainKHR*)Node->Item, 0);
        } break;
        case VULKANITEM_Surface: {
            vkDestroySurfaceKHR(VkCtx->Instance, *(VkSurfaceKHR*)Node->Item, 0);
        } break;
        case VULKANITEM_DebugMessenger: {
            PFN_vkDestroyDebugUtilsMessengerEXT DestroyMessengerCallback =
                (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
                    VkCtx->Instance, "vkDestroyDebugUtilsMessengerEXT");
            DestroyMessengerCallback(VkCtx->Instance, *(VkDebugUtilsMessengerEXT*)Node->Item, 0);
        } break;
        case VULKANITEM_Insance: {
            vkDestroyInstance(*(VkInstance*)Node->Item, 0);
        } break;
    }
    --Queue->Count;
    SllStackPop(Queue->Last);
}

static void VulkanDeletionQueueClear(vulkan_context* VkCtx, vulkan_deletion_queue* Queue) {
    Assert(Queue->Count > 0);
    while (Queue->Count > 0) {
        VulkanDeletionQueuePop(VkCtx, Queue);
    }
}

static void VulkanCopyToGpu(vulkan_context* VkCtx, VkBuffer Source, VkBuffer Dest, u64 Size) {
    vkResetCommandBuffer(VkCtx->TransientCommandBuffer, 0);

    VkCommandBufferBeginInfo BeginInfo = {};
    BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    BeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(VkCtx->TransientCommandBuffer, &BeginInfo);

    VkBufferCopy CopyRegion{};
    CopyRegion.srcOffset = 0;  // Optional
    CopyRegion.dstOffset = 0;  // Optional
    CopyRegion.size = Size;
    vkCmdCopyBuffer(VkCtx->TransientCommandBuffer, Source, Dest, 1, &CopyRegion);

    vkEndCommandBuffer(VkCtx->TransientCommandBuffer);

    VkSubmitInfo SubmitInfo = {};
    SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    SubmitInfo.commandBufferCount = 1;
    SubmitInfo.pCommandBuffers = &VkCtx->TransientCommandBuffer;

    vkQueueSubmit(VkCtx->GraphicsQueue, 1, &SubmitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(VkCtx->GraphicsQueue);

    //    vkFreeCommandBuffers(VkCtx->Device, VkCtx->TransientCommandPool, 1,
    //    &VkCtx->TransientCommandBuffer);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL
DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity,
              VkDebugUtilsMessageTypeFlagsEXT MessageType,
              const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData) {
    if (MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        dprintf(2, TXT_UWHT "INFO" TXT_RST ": %s\n", CallbackData->pMessage);
    } else if (MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        dprintf(2, TXT_UBLU "VERBOSE" TXT_BLU ": " TXT_RST "%s\n\n", CallbackData->pMessage);
    } else if (MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        dprintf(2, TXT_UYEL "WARNING" TXT_YEL ": " TXT_RST "%s\n\n", CallbackData->pMessage);
    } else if (MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        dprintf(2, TXT_URED "ERROR" TXT_RED ":" TXT_RST " %s\n\n", CallbackData->pMessage);
    }

    return VK_FALSE;
}

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                             const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                             const VkAllocationCallbacks* pAllocator,
                                             VkDebugUtilsMessengerEXT* pDebugMessenger) {
    PFN_vkCreateDebugUtilsMessengerEXT func =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance,
                                                                  "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static b32 IsDeviceGucciAndSetup(VkPhysicalDevice Device, vulkan_context* VkCtx,
                                 GLFWwindow* Window) {
    // NOTE(acol): Basic device properties like the name, type and supported Vulkan version

    ProfileFunction();
    VkSurfaceKHR Surface = VkCtx->Surface;
    temp_arena Temp = TempBegin(GlobalArena);
    VkPhysicalDeviceProperties Properties = {};
    vkGetPhysicalDeviceProperties(Device, &Properties);

    // NOTE(acol): support for optional features like texture compression and 64 bit floats
    VkPhysicalDeviceFeatures Features;
    vkGetPhysicalDeviceFeatures(Device, &Features);
    if (!((Properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) &&
          Features.geometryShader)) {
        return 0;
    }

    // NOTE(acol): check for required extensions, which has to be string based cause fuck you ?
    const char* RequiredExtensions[] = {VK_REQUIRED_DEVICE_EXTENSIONS};
    u32 ExtensionCount = 0;
    vkEnumerateDeviceExtensionProperties(Device, 0, &ExtensionCount, 0);
    VkExtensionProperties* Extensions = (VkExtensionProperties*)ArenaPush(
        Temp.Arena, sizeof(VkExtensionProperties) * ExtensionCount);
    vkEnumerateDeviceExtensionProperties(Device, 0, &ExtensionCount, Extensions);
    printf("Device Extensions\n");
    for (u32 i = 0; i < ExtensionCount; i++) {
        printf("    %s\n", Extensions[i].extensionName);
    }
    for (u32 i = 0; i < ArrayCount(RequiredExtensions); i++) {
        b32 Found = 0;
        for (u32 j = 0; j < ExtensionCount; j++) {
            if (strcmp(RequiredExtensions[i], Extensions[j].extensionName) == 0) {
                Found = 1;
                break;
            }
        }
        if (!Found) {
            return 0;
        }
    }

    // TODO(acol): I hate how all this is being done just remake all of it with a simple check for
    // the values I intend to support and nothing more, having arbitrary pixel format and present
    // mode sounds retarded
    VkSurfaceCapabilitiesKHR Capabilities = {};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(Device, Surface, &Capabilities);
    VkCtx->Capabilities = Capabilities;

    u32 FormatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, 0);
    if (!FormatCount) return 0;
    VkSurfaceFormatKHR* Formats =
        (VkSurfaceFormatKHR*)ArenaPush(Temp.Arena, sizeof(VkSurfaceFormatKHR) * FormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(Device, Surface, &FormatCount, Formats);

    u32 PresentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, 0);
    if (!PresentModeCount) return 0;
    VkPresentModeKHR* PresentModes =
        (VkPresentModeKHR*)ArenaPush(Temp.Arena, sizeof(VkPresentModeKHR) * PresentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(Device, Surface, &PresentModeCount, PresentModes);

    // NOTE(acol):  this is probably retarded to do like when will this ever not be supported, just
    // dont target things that dont support the layout you want or something ?
    //     VkCtx->SurfaceFormat = {};
    //     for (u32 i = 0; i < FormatCount; i++) {
    //         if (Formats[i].format == VK_FORMAT_B8G8R8A8_SRGB &&
    //             Formats[i].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
    //             VkCtx->SurfaceFormat = Formats[i];
    //             break;
    //         }
    //     }
    //     if (VkCtx->SurfaceFormat.format != VK_FORMAT_B8G8R8A8_SRGB) VkCtx->SurfaceFormat =
    //     Formats[0];
    VkCtx->SurfaceFormat = {.format = VK_FORMAT_B8G8R8A8_SRGB,
                            .colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    VkCtx->PresentMode = {};
    // NOTE(acol): This is also probably retarded just check to see what you are targetting when
    // would I ever possibly want to rng roll if I am double or tripple buffering or what presesnt
    // mode I am using lmao
    //     for (u32 i = 0; i < PresentModeCount; i++) {
    //         if (PresentModes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR) {
    //             VkCtx->PresentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
    //         }
    //     }
    if (VkCtx->PresentMode != VK_PRESENT_MODE_FIFO_RELAXED_KHR)
        VkCtx->PresentMode = VK_PRESENT_MODE_FIFO_KHR;

    if (Capabilities.currentExtent.width != MaxU32) {
        VkCtx->SwapchainExtent = Capabilities.currentExtent;
    } else {
        u32 Width = 0, Height = 0;
        glfwGetFramebufferSize(Window, (i32*)&Width, (i32*)&Height);
        VkCtx->SwapchainExtent.width =
            Clamp(Capabilities.minImageExtent.width, Width, Capabilities.maxImageExtent.width);
        VkCtx->SwapchainExtent.height =
            Clamp(Capabilities.minImageExtent.height, Height, Capabilities.maxImageExtent.height);
    }

    // NOTE(acol): double buffering has a bit less input lag, I dont really care either way but it
    // sounds like it should reduce complexity for me a bit as long as I stay at speed so will go
    // with that
    u32 ImageCount = ClampBot(Capabilities.minImageCount, 2);
    if (Capabilities.maxImageCount > 0)
        ImageCount = ClampTop(ImageCount, Capabilities.maxImageCount);
    VkCtx->ImageCount = ImageCount;

    vkGetPhysicalDeviceMemoryProperties(Device, &VkCtx->MemoryProperties);

    TempEnd(Temp);
    return 1;
}

static void RecreateSwapchain(vulkan_context* VkCtx) {
    ProfileFunction();
    vkDeviceWaitIdle(VkCtx->Device);

    for (u32 i = 0; i < VkCtx->ImageCount; i++) {
        vkDestroyImageView(VkCtx->Device, VkCtx->ImageViews[i], 0);
    }

    vkDestroySwapchainKHR(VkCtx->Device, VkCtx->Swapchain, 0);

    u32 Width = 0, Height = 0;
    while (Width == 0 || Height == 0) {
        glfwGetFramebufferSize(VkCtx->Window, (i32*)&Width, (i32*)&Height);
        glfwWaitEvents();
    }

    VkCtx->SwapchainExtent.width = Clamp(VkCtx->Capabilities.minImageExtent.width, Width,
                                         VkCtx->Capabilities.maxImageExtent.width);
    VkCtx->SwapchainExtent.height = Clamp(VkCtx->Capabilities.minImageExtent.height, Height,
                                          VkCtx->Capabilities.maxImageExtent.height);

    VkSwapchainCreateInfoKHR SwapchainInfo = {};
    SwapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapchainInfo.surface = VkCtx->Surface;
    SwapchainInfo.minImageCount = VkCtx->ImageCount;
    SwapchainInfo.imageFormat = VkCtx->SurfaceFormat.format;
    SwapchainInfo.imageColorSpace = VkCtx->SurfaceFormat.colorSpace;
    SwapchainInfo.imageExtent = VkCtx->SwapchainExtent;
    SwapchainInfo.imageArrayLayers = 1;
    SwapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SwapchainInfo.preTransform = VkCtx->Capabilities.currentTransform;
    SwapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapchainInfo.presentMode = VkCtx->PresentMode;
    // TODO
    SwapchainInfo.clipped = VK_FALSE;
    SwapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    SwapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(VkCtx->Device, &SwapchainInfo, 0, &VkCtx->Swapchain) != VK_SUCCESS) {
        dprintf(2, "Failed to creat swapchain\n");
        exit(1);
    }

    vkGetSwapchainImagesKHR(VkCtx->Device, VkCtx->Swapchain, &VkCtx->ImageCount, VkCtx->Images);

    VkImageViewCreateInfo ImageViewInfo = {};
    ImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ImageViewInfo.format = VkCtx->SurfaceFormat.format;
    ImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageViewInfo.subresourceRange.baseMipLevel = 0;
    ImageViewInfo.subresourceRange.levelCount = 1;
    ImageViewInfo.subresourceRange.baseArrayLayer = 0;
    ImageViewInfo.subresourceRange.layerCount = 1;
    for (u32 i = 0; i < VkCtx->ImageCount; i++) {
        ImageViewInfo.image = VkCtx->Images[i];
        if (vkCreateImageView(VkCtx->Device, &ImageViewInfo, 0, &VkCtx->ImageViews[i]) !=
            VK_SUCCESS) {
            dprintf(2, "Failed to create image view %u\n", i);
            exit(1);
        }
    }
    RecreateCount++;
    return;
}

static void ResizeCallback(GLFWwindow* Window, int Width, int Height) {
    vulkan_context* VkCtx = (vulkan_context*)glfwGetWindowUserPointer(Window);
    VkCtx->ShouldResize = 1;
}

static u32 GetSuitableMemoryIndex(vulkan_context* VkCtx, VkMemoryPropertyFlags Properties,
                                  u32 MemoryTypeBits) {
    for (u32 i = 0; i < VkCtx->MemoryProperties.memoryTypeCount; i++) {
        if ((MemoryTypeBits & (1 << i)) &&
            ((VkCtx->MemoryProperties.memoryTypes[i].propertyFlags & Properties) == Properties)) {
            return i;
        }
    }
    dprintf(2, "failed to find suitable memory\n");
    Assert(0);
    exit(1);
}

static void CreateVulkanBuffer(vulkan_context* VkCtx, VkBuffer* Buffer,
                               VkDeviceMemory* BufferMemory, u64 Size,
                               VkBufferUsageFlags UsageFlags, VkMemoryPropertyFlags MemoryFlags) {
    ProfileFunction();
    // NOTE(acol): memory allocation in gpu
    VkBufferCreateInfo BufferInfo = {};
    BufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    BufferInfo.size = Size;
    BufferInfo.usage = UsageFlags;
    BufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(VkCtx->Device, &BufferInfo, 0, Buffer) != VK_SUCCESS) {
        dprintf(2, "failed to create the vertex buffer\n");
        exit(1);
    }
    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)Buffer, VULKANITEM_Buffer);

    VkMemoryRequirements BufferRequirements = {};
    vkGetBufferMemoryRequirements(VkCtx->Device, *Buffer, &BufferRequirements);

    VkMemoryAllocateInfo BufferAllocateInfo = {};
    BufferAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    BufferAllocateInfo.allocationSize = BufferRequirements.size;
    BufferAllocateInfo.memoryTypeIndex =
        GetSuitableMemoryIndex(VkCtx, MemoryFlags, BufferRequirements.memoryTypeBits);

    if (vkAllocateMemory(VkCtx->Device, &BufferAllocateInfo, 0, BufferMemory) != VK_SUCCESS) {
        dprintf(2, "Failed to allocate buffer memory\n");
        exit(1);
    }
    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)BufferMemory,
                            VULKANITEM_Memory);

    vkBindBufferMemory(VkCtx->Device, *Buffer, *BufferMemory, 0);
}

static void RecreatePipeleine(vulkan_context* VkCtx) {
    ProfileFunction();
    vkDeviceWaitIdle(VkCtx->Device);

    vkDestroyShaderModule(VkCtx->Device, VkCtx->FragShader, 0);
    vkDestroyShaderModule(VkCtx->Device, VkCtx->VertShader, 0);
    vkDestroyPipelineLayout(VkCtx->Device, VkCtx->PipelineLayout, 0);
    vkDestroyPipeline(VkCtx->Device, VkCtx->GraphicsPipeline, 0);
    ArenaReset(ShaderArena);

    file_info FileInfoVertex = {};
    file_info FileInfoFragment = {};

    os_file_handle VertexHandle = OsFileOpen(StringLit("./shaders/vert.spv"), OSACCESS_Read);
    os_file_handle FragmentHandle = OsFileOpen(StringLit("./shaders/frag.spv"), OSACCESS_Read);
    if (!IsValid(VertexHandle) || !IsValid(FragmentHandle)) {
        dprintf(2, "Couldn't open shader files\n");
        exit(1);
    }

    FileInfoVertex = OsFileStat(VertexHandle);
    u8* VertexShader = (u8*)ArenaPush(ShaderArena, FileInfoVertex.Size);
    u64 Read = OsFileRead(VertexHandle, VertexShader, {0, FileInfoVertex.Size});
    if (Read == 0) {
        dprintf(2, "Failed reading Vertex shader file\n");
        exit(1);
    }
    OsFileClose(VertexHandle);

    VkShaderModuleCreateInfo VertexInfo = {};
    VertexInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    VertexInfo.codeSize = FileInfoVertex.Size;
    VertexInfo.pCode = (u32*)VertexShader;
    if (vkCreateShaderModule(VkCtx->Device, &VertexInfo, 0, &VkCtx->VertShader) != VK_SUCCESS) {
        dprintf(2, "Failed to create Vertex shader module\n");
        exit(1);
    }

    FileInfoFragment = OsFileStat(FragmentHandle);
    u8* FragmentShader = (u8*)ArenaPush(ShaderArena, FileInfoFragment.Size);
    Read = OsFileRead(FragmentHandle, FragmentShader, {0, FileInfoFragment.Size});

    if (Read == 0) {
        dprintf(2, "Failed reading Fragment shader file\n");
        exit(1);
    }
    OsFileClose(FragmentHandle);

    VkShaderModuleCreateInfo FragmentInfo = {};
    FragmentInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    FragmentInfo.codeSize = FileInfoFragment.Size;
    FragmentInfo.pCode = (u32*)FragmentShader;
    if (vkCreateShaderModule(VkCtx->Device, &FragmentInfo, 0, &VkCtx->FragShader) != VK_SUCCESS) {
        dprintf(2, "Failed to create Framgnet shader module\n");
        exit(1);
    }

    VkPipelineShaderStageCreateInfo VertShaderInfo = {};
    VertShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VertShaderInfo.module = VkCtx->VertShader;
    VertShaderInfo.pName = "main";

    VkPipelineShaderStageCreateInfo FragShaderInfo = {};
    FragShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    FragShaderInfo.module = VkCtx->FragShader;
    FragShaderInfo.pName = "main";

    VkPipelineShaderStageCreateInfo ShaderStages[2];
    ShaderStages[0] = VertShaderInfo;
    ShaderStages[1] = FragShaderInfo;

    VkDynamicState* DynamicStates =
        (VkDynamicState*)ArenaPush(ShaderArena, sizeof(VkDynamicState) * 3);
    DynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    DynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
    DynamicStates[2] = VK_DYNAMIC_STATE_VERTEX_INPUT_EXT;

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo = {};
    InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineDynamicStateCreateInfo DynamicStateInfo = {};
    DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStateInfo.dynamicStateCount = 3;
    DynamicStateInfo.pDynamicStates = DynamicStates;

    VkPipelineViewportStateCreateInfo ViewportStateInfo = {};
    ViewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportStateInfo.viewportCount = 1;
    ViewportStateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo RasterizationInfo = {};
    RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizationInfo.depthClampEnable = VK_FALSE;
    RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    RasterizationInfo.lineWidth = 1.0f;
    RasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    RasterizationInfo.depthBiasEnable = VK_FALSE;
    RasterizationInfo.depthBiasConstantFactor = 0.0f;
    RasterizationInfo.depthBiasClamp = 0.0f;
    RasterizationInfo.depthBiasSlopeFactor = 0.0f;

    VkPipelineMultisampleStateCreateInfo MultisamplingInfo = {};
    MultisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisamplingInfo.sampleShadingEnable = VK_FALSE;
    MultisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisamplingInfo.minSampleShading = 1.0f;
    MultisamplingInfo.pSampleMask = 0;
    MultisamplingInfo.alphaToCoverageEnable = VK_FALSE;
    MultisamplingInfo.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState ColorBlendAttachment = {};
    ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorBlendAttachment.blendEnable = VK_FALSE;
    ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo ColorBlendingInfo = {};
    ColorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendingInfo.logicOpEnable = VK_FALSE;
    ColorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
    ColorBlendingInfo.attachmentCount = 1;
    ColorBlendingInfo.pAttachments = &ColorBlendAttachment;
    ColorBlendingInfo.blendConstants[0] = 0.0f;
    ColorBlendingInfo.blendConstants[1] = 0.0f;
    ColorBlendingInfo.blendConstants[2] = 0.0f;
    ColorBlendingInfo.blendConstants[3] = 0.0f;

    VkPushConstantRange PushConstantRange = {};
    PushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    PushConstantRange.offset = 0;
    PushConstantRange.size = sizeof(push_constants);

    VkPipelineLayoutCreateInfo PipelineLayoutInfo = {};
    PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInfo.setLayoutCount = 0;
    PipelineLayoutInfo.pSetLayouts = 0;
    PipelineLayoutInfo.pushConstantRangeCount = 1;
    PipelineLayoutInfo.pPushConstantRanges = &PushConstantRange;
    if (vkCreatePipelineLayout(VkCtx->Device, &PipelineLayoutInfo, 0, &VkCtx->PipelineLayout) !=
        VK_SUCCESS) {
        dprintf(2, "Failed to create pipeline layout\n");
        exit(1);
    }

    VkPipelineRenderingCreateInfoKHR PipelineRenderingInfo = {};
    PipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    PipelineRenderingInfo.pNext = 0;
    PipelineRenderingInfo.colorAttachmentCount = 1;
    PipelineRenderingInfo.pColorAttachmentFormats = &VkCtx->SurfaceFormat.format;
    PipelineRenderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

    VkGraphicsPipelineCreateInfo PipelineInfo = {};
    PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineInfo.stageCount = 2;
    PipelineInfo.pStages = ShaderStages;
    PipelineInfo.pVertexInputState = 0;
    PipelineInfo.pInputAssemblyState = &InputAssemblyInfo;
    PipelineInfo.pViewportState = &ViewportStateInfo;
    PipelineInfo.pRasterizationState = &RasterizationInfo;
    PipelineInfo.pMultisampleState = &MultisamplingInfo;
    PipelineInfo.pDepthStencilState = 0;
    PipelineInfo.pColorBlendState = &ColorBlendingInfo;
    PipelineInfo.pDynamicState = &DynamicStateInfo;
    PipelineInfo.layout = VkCtx->PipelineLayout;
    PipelineInfo.pNext = &PipelineRenderingInfo;
    PipelineInfo.subpass = 0;
    // NOTE(acol): These are for creating new pipelines using this one as the base cause it's faster
    // that way and whatnot
    PipelineInfo.basePipelineHandle = VkCtx->GraphicsPipeline;
    PipelineInfo.basePipelineIndex = 0;

    if (vkCreateGraphicsPipelines(VkCtx->Device, VK_NULL_HANDLE, 1, &PipelineInfo, 0,
                                  &VkCtx->GraphicsPipeline) != VK_SUCCESS) {
        dprintf(2, "Failed to creat pipelines\n");
        exit(1);
    }
}

int main(void) {
    BeginProfile();

    arena_alloc_params AllocParams = {.Flags = ARENA_NoChainGrow,
                                      .ReserveSize = MB(100),
                                      .CommitSize = ARENA_DEFAULT_RESERVE_SIZE};
    GlobalArena = ArenaAlloc(AllocParams);

    vulkan_context VkCtx = {};

    AllocParams.ReserveSize = MB(4);
    AllocParams.CommitSize = KB(4);
    ShaderArena = ArenaAlloc(AllocParams);
    TestDeletionArena = ArenaAlloc(AllocParams);

    DeletionQueue =
        (vulkan_deletion_queue*)ArenaPush(TestDeletionArena, sizeof(vulkan_deletion_queue));

    /* ==============================================================================================

                                    // NOTE(acol): Glfw init stuff

      =============================================================================================
    */
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    GLFWwindow* Window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan hell", 0, 0);
    glfwSetWindowUserPointer(Window, &VkCtx);
    glfwSetFramebufferSizeCallback(Window, ResizeCallback);
    if (!Window) {
        dprintf(2, "Glfw failed to create a Window\n");
        return 1;
    }
    VkCtx.Window = Window;

    u32 GlfwExtensionCount = {};
    const char** GlfwExtensions;
    GlfwExtensions = glfwGetRequiredInstanceExtensions(&GlfwExtensionCount);

    /* ==============================================================================================

                                    // NOTE(acol): Vulkan init stuff

      =============================================================================================
    */

    VkApplicationInfo AppInfo = {};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "Triangle of vulkan and misery";
    AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.pEngineName = "Hella learning rn my nigga";
    AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo CreateInfo = {};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;
    CreateInfo.enabledExtensionCount = GlfwExtensionCount;
    CreateInfo.ppEnabledExtensionNames = GlfwExtensions;
    CreateInfo.enabledLayerCount = 0;

    // Get number of extensions, commit a buffer and go through them
    u32 ExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &ExtensionCount, 0);
    VkExtensionProperties* ExtensionProperties = (VkExtensionProperties*)ArenaPush(
        GlobalArena, sizeof(VkExtensionProperties) * ExtensionCount);
    vkEnumerateInstanceExtensionProperties(0, &ExtensionCount, ExtensionProperties);
    printf("Available extensions\n");
    for (u32 i = 0; i < ExtensionCount; i++) {
        printf("   %s\n", ExtensionProperties[i].extensionName);
    }

    // NOTE(acol): Get and print the layers
    u32 LayerCount = 0;
    vkEnumerateInstanceLayerProperties(&LayerCount, 0);
    VkLayerProperties* LayerProperties =
        (VkLayerProperties*)ArenaPush(GlobalArena, sizeof(VkLayerProperties) * LayerCount);
    vkEnumerateInstanceLayerProperties(&LayerCount, LayerProperties);
    printf("Available layers\n");
    for (u32 j = 0; j < LayerCount; j++) {
        printf("   %s\n", LayerProperties[j].layerName);
    }

#if VK_VALIDATE
    // NOTE(acol): grab the requred layers and check them against the available ones, activate
    // whatever is needed
    #define X(v) #v,
    const char* RequiredLayers[] = {VALIDATION_LAYERS};
    #undef X

    i32 LayersFound = 0;
    for (u32 i = 0; i < ArrayCount(RequiredLayers); i++) {
        for (u32 j = 0; j < LayerCount; j++) {
            if (strcmp(RequiredLayers[i], LayerProperties[j].layerName) == 0) LayersFound++;
            if (LayersFound == ArrayCount(RequiredLayers)) break;
        }
    }
    if (LayersFound != ArrayCount(RequiredLayers)) {
        dprintf(2, "Requested validation layers not supported\n");
        return 1;
    }
    CreateInfo.enabledLayerCount = ArrayCount(RequiredLayers);
    CreateInfo.ppEnabledLayerNames = RequiredLayers;
#endif

    const char* RequiredInstanceExtensions[] = {VK_REQUIRED_INSTANCE_EXTENSIONS};

    // NOTE(acol): instance extensiosn
    const char** Extensions = (const char**)ArenaPush(
        GlobalArena,
        sizeof(*GlfwExtensions) * (GlfwExtensionCount + ArrayCount(RequiredInstanceExtensions)));
    MemoryCopyTyped(Extensions, GlfwExtensions, GlfwExtensionCount);
    MemoryCopyTyped(Extensions + GlfwExtensionCount, RequiredInstanceExtensions,
                    ArrayCount(RequiredInstanceExtensions));
    CreateInfo.ppEnabledExtensionNames = Extensions;
    CreateInfo.enabledExtensionCount = GlfwExtensionCount + ArrayCount(RequiredInstanceExtensions);

    VkDebugUtilsMessengerCreateInfoEXT MessengerInfo = {};
    MessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    MessengerInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    MessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    MessengerInfo.pfnUserCallback = DebugCallback;

    // NOTE(acol): this thing is so crazy that you cant just ask for the validation layers on
    // initialization but I need to ask here and then get a function pointer to a function that
    // registers the validation object handle and then manually destroy it too.
    CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&MessengerInfo;

    if (vkCreateInstance(&CreateInfo, 0, &VkCtx.Instance) != VK_SUCCESS) {
        dprintf(2, "Well shit\n");
        return 1;
    }

    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.Instance,
                            VULKANITEM_Insance);

#if VK_VALIDATE
    // NOTE(acol): this shitty api doesnt even give you the function call to create a callback so
    // you have to ask for a poitner to it

    PFN_vkCreateDebugUtilsMessengerEXT CreateMessengerCallback =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VkCtx.Instance,
                                                                  "vkCreateDebugUtilsMessengerEXT");

    CreateMessengerCallback(VkCtx.Instance, &MessengerInfo, 0, &VkCtx.MessengerHandle);

    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.MessengerHandle,
                            VULKANITEM_DebugMessenger);

#endif

    // NOTE(acol): create Wayland surface
    // TODO(acol): this needs to be abstracted in some way, maybe do it when I get rid of glfw?
    VkWaylandSurfaceCreateInfoKHR SurfaceInfo = {};
    SurfaceInfo.sType = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR;
    SurfaceInfo.display = glfwGetWaylandDisplay();
    SurfaceInfo.surface = glfwGetWaylandWindow(Window);
    if (vkCreateWaylandSurfaceKHR(VkCtx.Instance, &SurfaceInfo, 0, &VkCtx.Surface) != VK_SUCCESS) {
        dprintf(2, "Failed to create Wayland surface\n");
        return 1;
    }

    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.Surface,
                            VULKANITEM_Surface);

    // NOTE(acol): select physical device
    u32 DeviceCount = 0;
    vkEnumeratePhysicalDevices(VkCtx.Instance, &DeviceCount, 0);
    if (!DeviceCount) {
        dprintf(2, "No available GPUs with vulkan support\n");
        return 1;
    }
    VkPhysicalDevice* Devices =
        (VkPhysicalDevice*)ArenaPush(GlobalArena, sizeof(VkPhysicalDevice) * DeviceCount);
    vkEnumeratePhysicalDevices(VkCtx.Instance, &DeviceCount, Devices);
    for (u32 i = 0; i < DeviceCount; i++) {
        if (IsDeviceGucciAndSetup(Devices[i], &VkCtx, Window)) {
            VkCtx.PhysicalDevice = Devices[i];
            break;
        }
    }
    if (!VkCtx.PhysicalDevice) {
        dprintf(2, "No suitable GPUs\n");
        return 1;
    }

    // NOTE(acol): find whatever queues are needed
    u32 QueueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(VkCtx.PhysicalDevice, &QueueFamilyCount, 0);

    VkQueueFamilyProperties* QueueFamilies = (VkQueueFamilyProperties*)ArenaPush(
        GlobalArena, sizeof(VkQueueFamilyProperties) * QueueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(VkCtx.PhysicalDevice, &QueueFamilyCount,
                                             QueueFamilies);

    u32 GraphicsQueueIndex = 0;
    for (u32 i = 0; i < QueueFamilyCount; i++) {
        VkBool32 CanPresent = {};
        if ((QueueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) &&
            (vkGetPhysicalDeviceSurfaceSupportKHR(VkCtx.PhysicalDevice, i, VkCtx.Surface,
                                                  &CanPresent))) {
            GraphicsQueueIndex = i;
            break;
        }
    }
    if (GraphicsQueueIndex == MaxU32) {
        dprintf(2, "Couldnt fine suitable vulkan queue\n");
        return 1;
    }
    VkCtx.GraphicsQueueIndex = GraphicsQueueIndex;
    // NOTE(acol): Create the queues we found above
    VkDeviceQueueCreateInfo QueueInfo = {};
    QueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    QueueInfo.queueFamilyIndex = GraphicsQueueIndex;
    QueueInfo.queueCount = 1;
    f32 QueuePrio = 1.0f;
    QueueInfo.pQueuePriorities = &QueuePrio;

    // NOTE(acol): device features
    VkPhysicalDeviceVertexInputDynamicStateFeaturesEXT DynamicVertexInputFeatures = {};
    DynamicVertexInputFeatures.sType =
        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VERTEX_INPUT_DYNAMIC_STATE_FEATURES_EXT;
    DynamicVertexInputFeatures.vertexInputDynamicState = VK_TRUE;

    VkPhysicalDeviceVulkan12Features Features12 = {};
    Features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    Features12.pNext = &DynamicVertexInputFeatures;
    Features12.bufferDeviceAddress = VK_TRUE;
    Features12.descriptorIndexing = VK_TRUE;

    VkPhysicalDeviceVulkan13Features Features13 = {};
    Features13.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES;
    Features13.pNext = &Features12;
    Features13.dynamicRendering = VK_TRUE;
    Features13.synchronization2 = VK_TRUE;

    // NOTE(acol): Create logical device
    const char* RequiredExtensions[] = {VK_REQUIRED_DEVICE_EXTENSIONS};
    VkDeviceCreateInfo DeviceInfo = {};
    DeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    DeviceInfo.pQueueCreateInfos = &QueueInfo;
    DeviceInfo.queueCreateInfoCount = 1;
    VkPhysicalDeviceFeatures DeviceFeatures = {};
    DeviceFeatures.geometryShader = VK_FALSE;
    DeviceInfo.pEnabledFeatures = &DeviceFeatures;
    DeviceInfo.enabledExtensionCount = ArrayCount(RequiredExtensions);
    DeviceInfo.ppEnabledExtensionNames = RequiredExtensions;
    DeviceInfo.pNext = &Features13;
    if (vkCreateDevice(VkCtx.PhysicalDevice, &DeviceInfo, 0, &VkCtx.Device) != VK_SUCCESS) {
        dprintf(2, "Failed to create logical device\n");
        return 1;
    }
    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.Device,
                            VULKANITEM_Device);

    vkGetDeviceQueue(VkCtx.Device, GraphicsQueueIndex, 0, &VkCtx.GraphicsQueue);

    VkSwapchainCreateInfoKHR SwapchainInfo = {};
    SwapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    SwapchainInfo.surface = VkCtx.Surface;
    SwapchainInfo.minImageCount = VkCtx.ImageCount;
    SwapchainInfo.imageFormat = VkCtx.SurfaceFormat.format;
    SwapchainInfo.imageColorSpace = VkCtx.SurfaceFormat.colorSpace;
    SwapchainInfo.imageExtent = VkCtx.SwapchainExtent;
    SwapchainInfo.imageArrayLayers = 1;
    // NOTE(acol): using this cause I plan to render to it and just splat it in the screen, for post
    // processsing youd need to do something else
    SwapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    SwapchainInfo.preTransform = VkCtx.Capabilities.currentTransform;
    SwapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    SwapchainInfo.presentMode = VkCtx.PresentMode;
    // TODO
    SwapchainInfo.clipped = VK_FALSE;
    // TODO(acol): sharing mode can also be concurrent but it's slower and I'd need to specify here
    // what queue family indexes are involved. I should restructure this code so it has access to
    // the indexes and is maybe a bit more generic. Or maybe fuck that and just rewrite it next time
    // different idk
    SwapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    SwapchainInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(VkCtx.Device, &SwapchainInfo, 0, &VkCtx.Swapchain) != VK_SUCCESS) {
        dprintf(2, "Failed to creat swapchain\n");
        return 1;
    }
    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.Swapchain,
                            VULKANITEM_Swapchain);

    ArenaReset(GlobalArena);
    // NOTE(acol): This feels kinda weird cause I shouldn't pop it off the arena. Maybe I need to
    // make an arena that wont reset ?

    vkGetSwapchainImagesKHR(VkCtx.Device, VkCtx.Swapchain, &VkCtx.ImageCount, 0);
    VkCtx.Images = (VkImage*)ArenaPush(GlobalArena, sizeof(VkImage) * VkCtx.ImageCount);
    vkGetSwapchainImagesKHR(VkCtx.Device, VkCtx.Swapchain, &VkCtx.ImageCount, VkCtx.Images);

    VkCtx.ImageViews = (VkImageView*)ArenaPush(GlobalArena, sizeof(VkImageView) * VkCtx.ImageCount);

    VkImageViewCreateInfo ImageViewInfo = {};
    ImageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    ImageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    ImageViewInfo.format = VkCtx.SurfaceFormat.format;
    ImageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    ImageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    ImageViewInfo.subresourceRange.baseMipLevel = 0;
    ImageViewInfo.subresourceRange.levelCount = 1;
    ImageViewInfo.subresourceRange.baseArrayLayer = 0;
    ImageViewInfo.subresourceRange.layerCount = 1;
    for (u32 i = 0; i < VkCtx.ImageCount; i++) {
        ImageViewInfo.image = VkCtx.Images[i];
        if (vkCreateImageView(VkCtx.Device, &ImageViewInfo, 0, &VkCtx.ImageViews[i]) !=
            VK_SUCCESS) {
            dprintf(2, "Failed to create image view %u\n", i);
            return 1;
        }
        VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.ImageViews[i],
                                VULKANITEM_ImageView);
    }

    dprintf(2, "Sizeof Vulkan context %lu\n", sizeof(VkCtx));

    /* ==============================================================================================

                                // NOTE(acol): Pipeline stuff

      =============================================================================================
    */

    // NOTE(acol): Grab the two shaders
    file_info FileInfoVertex = {};
    file_info FileInfoFragment = {};

    os_file_handle VertexHandle = OsFileOpen(StringLit("./shaders/vert.spv"), OSACCESS_Read);
    os_file_handle FragmentHandle = OsFileOpen(StringLit("./shaders/frag.spv"), OSACCESS_Read);
    if (!IsValid(VertexHandle) || !IsValid(FragmentHandle)) {
        dprintf(2, "Couldn't open shader files\n");
        return 1;
    }

    FileInfoVertex = OsFileStat(VertexHandle);
    u8* VertexShader = (u8*)ArenaPush(ShaderArena, FileInfoVertex.Size);
    u64 Read = OsFileRead(VertexHandle, VertexShader, {0, FileInfoVertex.Size});
    if (Read == 0) {
        dprintf(2, "Failed reading Vertex shader file\n");
        return 1;
    }
    OsFileClose(VertexHandle);

    VkShaderModuleCreateInfo VertexInfo = {};
    VertexInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    VertexInfo.codeSize = FileInfoVertex.Size;
    VertexInfo.pCode = (u32*)VertexShader;
    if (vkCreateShaderModule(VkCtx.Device, &VertexInfo, 0, &VkCtx.VertShader) != VK_SUCCESS) {
        dprintf(2, "Failed to create Vertex shader module\n");
        return 1;
    }

    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.VertShader,
                            VULKANITEM_ShaderModule);

    FileInfoFragment = OsFileStat(FragmentHandle);
    u8* FragmentShader = (u8*)ArenaPush(ShaderArena, FileInfoFragment.Size);
    Read = OsFileRead(FragmentHandle, FragmentShader, {0, FileInfoFragment.Size});

    if (Read == 0) {
        dprintf(2, "Failed reading Fragment shader file\n");
        return 1;
    }
    OsFileClose(FragmentHandle);

    VkShaderModuleCreateInfo FragmentInfo = {};
    FragmentInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    FragmentInfo.codeSize = FileInfoFragment.Size;
    FragmentInfo.pCode = (u32*)FragmentShader;
    if (vkCreateShaderModule(VkCtx.Device, &FragmentInfo, 0, &VkCtx.FragShader) != VK_SUCCESS) {
        dprintf(2, "Failed to create Framgnet shader module\n");
        return 1;
    }

    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.FragShader,
                            VULKANITEM_ShaderModule);

    VkPipelineShaderStageCreateInfo VertShaderInfo = {};
    VertShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    VertShaderInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    VertShaderInfo.module = VkCtx.VertShader;
    VertShaderInfo.pName = "main";

    VkPipelineShaderStageCreateInfo FragShaderInfo = {};
    FragShaderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    FragShaderInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    FragShaderInfo.module = VkCtx.FragShader;
    FragShaderInfo.pName = "main";

    VkPipelineShaderStageCreateInfo ShaderStages[2] = {VertShaderInfo, FragShaderInfo};

    VkDynamicState* DynamicStates =
        (VkDynamicState*)ArenaPush(GlobalArena, sizeof(VkDynamicState) * 3);
    DynamicStates[0] = VK_DYNAMIC_STATE_VIEWPORT;
    DynamicStates[1] = VK_DYNAMIC_STATE_SCISSOR;
    DynamicStates[2] = VK_DYNAMIC_STATE_VERTEX_INPUT_EXT;

    VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo = {};
    InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    VkPipelineDynamicStateCreateInfo DynamicStateInfo = {};
    DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    DynamicStateInfo.dynamicStateCount = 3;
    DynamicStateInfo.pDynamicStates = DynamicStates;

    VkPipelineViewportStateCreateInfo ViewportStateInfo = {};
    ViewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    ViewportStateInfo.viewportCount = 1;
    ViewportStateInfo.scissorCount = 1;

    VkPipelineRasterizationStateCreateInfo RasterizationInfo = {};
    RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    RasterizationInfo.depthClampEnable = VK_FALSE;
    RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    RasterizationInfo.lineWidth = 1.0f;
    RasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
    RasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
    RasterizationInfo.depthBiasEnable = VK_FALSE;
    RasterizationInfo.depthBiasConstantFactor = 0.0f;
    RasterizationInfo.depthBiasClamp = 0.0f;
    RasterizationInfo.depthBiasSlopeFactor = 0.0f;

    // TODO(acol): this shit needs some exploring later
    VkPipelineMultisampleStateCreateInfo MultisamplingInfo = {};
    MultisamplingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    MultisamplingInfo.sampleShadingEnable = VK_FALSE;
    MultisamplingInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    MultisamplingInfo.minSampleShading = 1.0f;
    MultisamplingInfo.pSampleMask = 0;
    MultisamplingInfo.alphaToCoverageEnable = VK_FALSE;
    MultisamplingInfo.alphaToOneEnable = VK_FALSE;

    // NOTE(acol): This is per frame buffer
    VkPipelineColorBlendAttachmentState ColorBlendAttachment = {};
    ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    ColorBlendAttachment.blendEnable = VK_FALSE;
    // NOTE(acol): this will blend depending on the new frames alpha value, kinda cool. It just
    // multiplies the color by blend factor and then adds them together, the tutorial is wrong here
    ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    // NOTE(acol): Could use a logic op and that will auto disable the above way of blending. The
    // last 4 values are used if srcColorBlendFactor in the above function is set for example to
    // VK_BLEND_FACTOR_CONSTANT_ALPHA to define a constand alpha.
    VkPipelineColorBlendStateCreateInfo ColorBlendingInfo = {};
    ColorBlendingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    ColorBlendingInfo.logicOpEnable = VK_FALSE;
    ColorBlendingInfo.logicOp = VK_LOGIC_OP_COPY;
    ColorBlendingInfo.attachmentCount = 1;
    ColorBlendingInfo.pAttachments = &ColorBlendAttachment;
    ColorBlendingInfo.blendConstants[0] = 0.0f;
    ColorBlendingInfo.blendConstants[1] = 0.0f;
    ColorBlendingInfo.blendConstants[2] = 0.0f;
    ColorBlendingInfo.blendConstants[3] = 0.0f;

    VkPushConstantRange PushConstantRange = {};
    PushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    PushConstantRange.offset = 0;
    PushConstantRange.size = sizeof(push_constants);

    VkPipelineLayoutCreateInfo PipelineLayoutInfo = {};
    PipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    PipelineLayoutInfo.setLayoutCount = 0;
    PipelineLayoutInfo.pSetLayouts = 0;
    PipelineLayoutInfo.pushConstantRangeCount = 1;
    PipelineLayoutInfo.pPushConstantRanges = &PushConstantRange;
    if (vkCreatePipelineLayout(VkCtx.Device, &PipelineLayoutInfo, 0, &VkCtx.PipelineLayout) !=
        VK_SUCCESS) {
        dprintf(2, "Failed to create pipeline layout\n");
        return 1;
    }

    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.PipelineLayout,
                            VULKANITEM_PipelineLayout);

    VkPipelineRenderingCreateInfoKHR PipelineRenderingInfo = {};
    PipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    PipelineRenderingInfo.pNext = 0;
    PipelineRenderingInfo.colorAttachmentCount = 1;
    PipelineRenderingInfo.pColorAttachmentFormats = &VkCtx.SurfaceFormat.format;
    PipelineRenderingInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;

    VkGraphicsPipelineCreateInfo PipelineInfo = {};
    PipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    PipelineInfo.stageCount = 2;
    PipelineInfo.pStages = ShaderStages;
    PipelineInfo.pVertexInputState = 0;
    PipelineInfo.pInputAssemblyState = &InputAssemblyInfo;
    PipelineInfo.pViewportState = &ViewportStateInfo;
    PipelineInfo.pRasterizationState = &RasterizationInfo;
    PipelineInfo.pMultisampleState = &MultisamplingInfo;
    PipelineInfo.pDepthStencilState = 0;
    PipelineInfo.pColorBlendState = &ColorBlendingInfo;
    PipelineInfo.pDynamicState = &DynamicStateInfo;
    PipelineInfo.layout = VkCtx.PipelineLayout;
    PipelineInfo.pNext = &PipelineRenderingInfo;
    PipelineInfo.subpass = 0;
    // NOTE(acol): These are for creating new pipelines using this one as the base cause it's faster
    // that way and whatnot
    PipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    PipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(VkCtx.Device, VK_NULL_HANDLE, 1, &PipelineInfo, 0,
                                  &VkCtx.GraphicsPipeline) != VK_SUCCESS) {
        dprintf(2, "Failed to creat pipelines\n");
        return 1;
    }

    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.GraphicsPipeline,
                            VULKANITEM_Pipeline);

    /* ==============================================================================================

                        // NOTE(acol): Create command pools

      =============================================================================================
    */

    VkCommandPoolCreateInfo PoolInfo = {};
    PoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    PoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    PoolInfo.queueFamilyIndex = VkCtx.GraphicsQueueIndex;
    if (vkCreateCommandPool(VkCtx.Device, &PoolInfo, 0, &VkCtx.CommandPool) != VK_SUCCESS) {
        dprintf(2, "Failed to create commnad pool\n");
        return 1;
    }

    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.CommandPool,
                            VULKANITEM_CommandPool);

    if (vkCreateCommandPool(VkCtx.Device, &PoolInfo, 0, &VkCtx.TransientCommandPool) !=
        VK_SUCCESS) {
        dprintf(2, "Failed to create commnad pool\n");
        return 1;
    }

    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.TransientCommandPool,
                            VULKANITEM_CommandPool);

    /* ==============================================================================================

                            // NOTE(acol): Allocate Command buffer and sync primitives

      =============================================================================================
    */

    VkCommandBufferAllocateInfo CommandBufferAllocateInfo = {};
    CommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandBufferAllocateInfo.commandPool = VkCtx.CommandPool;
    CommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    CommandBufferAllocateInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(VkCtx.Device, &CommandBufferAllocateInfo, &VkCtx.CommandBuffer) !=
        VK_SUCCESS) {
        dprintf(2, "Failed to allocate commnad buffer\n");
        return 1;
    }

    VkCommandBufferAllocateInfo TransientCommandBufferAllocateInfo = {};
    TransientCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    TransientCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    TransientCommandBufferAllocateInfo.commandPool = VkCtx.TransientCommandPool;
    TransientCommandBufferAllocateInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(VkCtx.Device, &TransientCommandBufferAllocateInfo,
                                 &VkCtx.TransientCommandBuffer) != VK_SUCCESS) {
        dprintf(2, "Failed to allocate commnad buffer\n");
        return 1;
    }

    VkSemaphoreCreateInfo SemaphoreInfo = {};
    SemaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    if (vkCreateSemaphore(VkCtx.Device, &SemaphoreInfo, 0, &VkCtx.RenderSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(VkCtx.Device, &SemaphoreInfo, 0, &VkCtx.ImageSemaphore) != VK_SUCCESS) {
        dprintf(2, "Failed to create semaphores\n");
        return 1;
    }

    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.RenderSemaphore,
                            VULKANITEM_Sempahore);
    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.ImageSemaphore,
                            VULKANITEM_Sempahore);

    VkFenceCreateInfo FenceInfo = {};
    FenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    FenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (vkCreateFence(VkCtx.Device, &FenceInfo, 0, &VkCtx.InFlightFence) != VK_SUCCESS) {
        dprintf(2, "Failed to create fence\n");
        return 1;
    }
    VulkanDeletionQueuePush(TestDeletionArena, DeletionQueue, (void*)&VkCtx.InFlightFence,
                            VULKANITEM_Fence);

    /* ==============================================================================================

                                    // NOTE(acol): Main loop

      =============================================================================================
    */

    printf("VkInstance: %lu\n", sizeof(VkInstance));
    printf("VkDebugUtilsMessengerEXT: %lu\n", sizeof(VkDebugUtilsMessengerEXT));
    printf("VkPhysicalDevice: %lu\n", sizeof(VkPhysicalDevice));
    printf("VkQueue: %lu\n", sizeof(VkQueue));
    printf("VkDevice: %lu\n", sizeof(VkDevice));
    printf("VkSurfaceKHR: %lu\n", sizeof(VkSurfaceKHR));
    printf("VkSurfaceFormatKHR: %lu\n", sizeof(VkSurfaceFormatKHR));
    printf("VkPresentModeKHR: %lu\n", sizeof(VkPresentModeKHR));
    printf("VkSurfaceCapabilitiesKHR: %lu\n", sizeof(VkSurfaceCapabilitiesKHR));
    printf("VkExtent2D: %lu\n", sizeof(VkExtent2D));
    printf("u32: %lu\n", sizeof(u32));
    printf("VkSwapchainKHR: %lu\n", sizeof(VkSwapchainKHR));
    printf("VkImage: %lu\n", sizeof(VkImage));
    printf("VkImageView: %lu\n", sizeof(VkImageView));
    printf("VkShaderModule: %lu\n", sizeof(VkShaderModule));
    printf("VkShaderModule: %lu\n", sizeof(VkShaderModule));
    printf("VkPipelineLayout: %lu\n", sizeof(VkPipelineLayout));
    printf("VkRenderPass: %lu\n", sizeof(VkRenderPass));
    printf("VkPipeline: %lu\n", sizeof(VkPipeline));
    printf("VkFramebuffer: %lu\n", sizeof(VkFramebuffer));
    printf("VkCommandPool: %lu\n", sizeof(VkCommandPool));
    printf("VkCommandBuffer: %lu\n", sizeof(VkCommandBuffer));
    printf("VkSemaphore: %lu\n", sizeof(VkSemaphore));
    printf("VkSemaphore: %lu\n", sizeof(VkSemaphore));
    printf("VkFence: %lu\n", sizeof(VkFence));
    printf("VkBuffer: %lu\n", sizeof(VkBuffer));
    printf("VkPhysicalDeviceMemoryProperties: %lu\n", sizeof(VkPhysicalDeviceMemoryProperties));

    // NOTE(acol): Dynamic vertex stuff
    tutorial_vertex Vertices[] = {
        //        {{1.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        //        {{-1.0f, 0.0f, 0.0f}, {1.0f, 0.8f, 0.8f}},
        //
        //        {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        //        {{0.0f, -1.0f, 0.0f}, {0.8f, 1.0f, 0.8f}},
        //
        //        {{0.0f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}},
        //        {{0.0f, 0.0f, -1.0f}, {0.8f, 0.8f, 1.0f}}

        // NOTE(acol): TRIANG1
        {{0.0f, -0.577f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, 0.289f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.289f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        // NOTE(acol): TRIANG2
        {{0.0f, 0.0f, 0.8165f}, {0.0f, 1.0f, 0.0f}},
        {{0.0f, -0.577f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.289f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        // NOTE(acol): TRIANG3
        {{0.0f, 0.0f, 0.8165f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.289f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{0.0f, -0.577f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        // NOTE(acol): TRIANG4
        {{0.0f, 0.0f, 0.8165f}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, 0.289f, 0.0f}, {1.0f, 1.0f, 1.0f}},
        {{-0.5f, 0.289f, 0.0f}, {1.0f, 1.0f, 1.0f}},
    };
    //    u32 Indices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};

    VkVertexInputBindingDescription2EXT BindingDescription = {};
    BindingDescription.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
    BindingDescription.binding = 0;
    BindingDescription.stride = sizeof(tutorial_vertex);
    BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    BindingDescription.divisor = 1;

    VkVertexInputAttributeDescription2EXT* AttributeDescriptions =
        (VkVertexInputAttributeDescription2EXT*)ArenaPush(
            GlobalArena, 2 * sizeof(VkVertexInputAttributeDescription));

    AttributeDescriptions[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
    AttributeDescriptions[0].binding = 0;
    AttributeDescriptions[0].location = 0;
    AttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    AttributeDescriptions[0].offset = OffsetOfMember(tutorial_vertex, Position);

    AttributeDescriptions[1].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
    AttributeDescriptions[1].binding = 0;
    AttributeDescriptions[1].location = 1;
    AttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    AttributeDescriptions[1].offset = OffsetOfMember(tutorial_vertex, Color);

    PFN_vkCmdSetVertexInputEXT vkCmdSetVertexInputEXT =
        (PFN_vkCmdSetVertexInputEXT)vkGetDeviceProcAddr(VkCtx.Device, "vkCmdSetVertexInputEXT");

    CreateVulkanBuffer(&VkCtx, &VkCtx.VertexBuffer, &VkCtx.VertexBufferMemory, sizeof(Vertices),
                       VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    CreateVulkanBuffer(&VkCtx, &VkCtx.StagingBuffer, &VkCtx.StagingBufferMemory, sizeof(Vertices),
                       VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* Buffer = {};
    vkMapMemory(VkCtx.Device, VkCtx.StagingBufferMemory, 0, sizeof(Vertices), 0, &Buffer);
    memcpy(Buffer, Vertices, sizeof(Vertices));

    VulkanCopyToGpu(&VkCtx, VkCtx.StagingBuffer, VkCtx.VertexBuffer, sizeof(Vertices));

    //    CreateVulkanBuffer(&VkCtx, &VkCtx.IndexBuffer, &VkCtx.IndexBufferMemory, sizeof(Indices),
    //                       VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    //                       VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    //
    //    memcpy(Buffer, Indices, sizeof(Indices));
    //    vkUnmapMemory(VkCtx.Device, VkCtx.StagingBufferMemory);
    //
    //    VulkanCopyToGpu(&VkCtx, VkCtx.StagingBuffer, VkCtx.IndexBuffer, sizeof(Indices));

    push_constants PushConstants = {};
    u32 ImageIndex = 0;
    u64 FrameCount = 0;
    f64 RollingAvg = 0;
    // TODO(acol): Need to make the rdtsc frequency thing cross platform and put it in os layer!
    f32 RdtscFrequency = EstimateBlockTimerFreq();
    u64 TimeStart = __rdtsc();
    while (!glfwWindowShouldClose(Window)) {
        u64 StartOfFrame = -__rdtsc();
        ProfileBlock("Main Loop");
        glfwPollEvents();

        vkWaitForFences(VkCtx.Device, 1, &VkCtx.InFlightFence, VK_TRUE, MaxU64);

        file_info FileVertexShader = OsFileStat(StringLit("./shaders/vert.spv"));
        file_info FileFragmentShader = OsFileStat(StringLit("./shaders/frag.spv"));

        if ((FileVertexShader.LastModified != FileInfoVertex.LastModified) ||
            (FileFragmentShader.LastModified != FileInfoFragment.LastModified)) {
            RecreatePipeleine(&VkCtx);
            FileVertexShader.LastModified = FileInfoVertex.LastModified;
            FileFragmentShader.LastModified = FileInfoFragment.LastModified;
        }

        VkResult Res = vkAcquireNextImageKHR(VkCtx.Device, VkCtx.Swapchain, MaxU64,
                                             VkCtx.ImageSemaphore, VK_NULL_HANDLE, &ImageIndex);

        if (Unlikely(Res != VK_SUCCESS)) {
            if (Res == VK_ERROR_OUT_OF_DATE_KHR || Res == VK_SUBOPTIMAL_KHR) {
                RecreateSwapchain(&VkCtx);
                continue;
            } else {
                dprintf(2, "failed acquiring swapchain image\n");
                return 1;
            }
        }
        vkResetFences(VkCtx.Device, 1, &VkCtx.InFlightFence);

        vkResetCommandBuffer(VkCtx.CommandBuffer, 0);

        // NOTE(acol): Record Command buffer
        VkCommandBufferBeginInfo CommandBufferInfo = {};
        CommandBufferInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(VkCtx.CommandBuffer, &CommandBufferInfo) != VK_SUCCESS) {
            dprintf(2, "Failed to begin the commnad buffer\n");
            return 1;
        }

        VkClearValue ClearColor = {{{0.0f, 0.0f, 0.0f, 0.0f}}};

        VkRenderingAttachmentInfoKHR AttachmentInfo = {};
        AttachmentInfo.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        AttachmentInfo.imageView = VkCtx.ImageViews[ImageIndex];
        AttachmentInfo.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        AttachmentInfo.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        AttachmentInfo.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        AttachmentInfo.clearValue = ClearColor;

        VkRenderingInfo RenderInfo = {};
        RenderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        RenderInfo.renderArea = {.offset = {0, 0}, .extent = VkCtx.SwapchainExtent};
        RenderInfo.layerCount = 1;
        RenderInfo.colorAttachmentCount = 1;
        RenderInfo.pColorAttachments = &AttachmentInfo;
        RenderInfo.pDepthAttachment = 0;
        RenderInfo.pStencilAttachment = 0;

        VkViewport Viewport = {};
        Viewport.x = 0.0f;
        Viewport.y = 0.0f;
        Viewport.width = (f32)VkCtx.SwapchainExtent.width;
        Viewport.height = (f32)VkCtx.SwapchainExtent.height;
        Viewport.minDepth = 0.0f;
        Viewport.maxDepth = 1.0f;

        VkRect2D Scissor = {};
        Scissor.offset = {0, 0};
        Scissor.extent = VkCtx.SwapchainExtent;
        VkDeviceSize Offsets[] = {0};

        BindingDescription.sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_BINDING_DESCRIPTION_2_EXT;
        BindingDescription.binding = 0;
        BindingDescription.stride = sizeof(tutorial_vertex);
        BindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        BindingDescription.divisor = 1;

        AttributeDescriptions[0].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        AttributeDescriptions[0].binding = 0;
        AttributeDescriptions[0].location = 0;
        AttributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDescriptions[0].offset = OffsetOfMember(tutorial_vertex, Position);

        AttributeDescriptions[1].sType = VK_STRUCTURE_TYPE_VERTEX_INPUT_ATTRIBUTE_DESCRIPTION_2_EXT;
        AttributeDescriptions[1].binding = 0;
        AttributeDescriptions[1].location = 1;
        AttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        AttributeDescriptions[1].offset = OffsetOfMember(tutorial_vertex, Color);

        // NOTE(acol): transform swapchain image you get to format for writing onto
        VkImageMemoryBarrier2 ImageBarriers[2] = {};
        ImageBarriers[0].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        ImageBarriers[0].image = VkCtx.Images[ImageIndex];
        ImageBarriers[0].srcAccessMask = 0;  // memory op to wait for
        ImageBarriers[0].srcStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;  // stage to wait for
        ImageBarriers[0].dstAccessMask =
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;  // memory op to wait on
        ImageBarriers[0].dstStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;  // stage to wait on
        ImageBarriers[0].oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        ImageBarriers[0].newLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        ImageBarriers[0].subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                             .baseMipLevel = 0,
                                             .levelCount = 1,
                                             .baseArrayLayer = 0,
                                             .layerCount = 1};

        // NOTE(acol): transform swapchain image from pipleine output to present format
        ImageBarriers[1].sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
        ImageBarriers[1].image = VkCtx.Images[ImageIndex];
        ImageBarriers[1].srcAccessMask =
            VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT;  // memory op to wait for
        ImageBarriers[1].srcStageMask =
            VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;           // stage to wait for
        ImageBarriers[1].dstAccessMask = VK_ACCESS_2_MEMORY_READ_BIT;  // memory op to wait on
        ImageBarriers[1].dstStageMask = VK_PIPELINE_STAGE_2_BOTTOM_OF_PIPE_BIT;  // stage to wait on
        ImageBarriers[1].oldLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL;
        ImageBarriers[1].newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        ImageBarriers[1].subresourceRange = {.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                             .baseMipLevel = 0,
                                             .levelCount = 1,
                                             .baseArrayLayer = 0,
                                             .layerCount = 1};

        VkDependencyInfo SourceImageDependency = {};
        SourceImageDependency.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
        SourceImageDependency.imageMemoryBarrierCount = 2;
        SourceImageDependency.pImageMemoryBarriers = ImageBarriers;

        vkCmdPipelineBarrier2(VkCtx.CommandBuffer, &SourceImageDependency);

        PushConstants.Time = (f32)(__rdtsc() - TimeStart) / (RdtscFrequency / 1000.0f);
        PushConstants.Resolution = {(f32)VkCtx.SwapchainExtent.width,
                                    (f32)VkCtx.SwapchainExtent.height};

        vkCmdPushConstants(VkCtx.CommandBuffer, VkCtx.PipelineLayout,
                           VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0,
                           sizeof(push_constants), &PushConstants);

        // NOTE(acol): Actual rendering commands
        vkCmdBeginRendering(VkCtx.CommandBuffer, &RenderInfo);

        // NOTE(acol): Resolve dynamic state of the pipeline
        vkCmdSetVertexInputEXT(VkCtx.CommandBuffer, 1, &BindingDescription, 2,
                               AttributeDescriptions);
        vkCmdSetViewport(VkCtx.CommandBuffer, 0, 1, &Viewport);
        vkCmdSetScissor(VkCtx.CommandBuffer, 0, 1, &Scissor);
        vkCmdSetPrimitiveTopology(VkCtx.CommandBuffer, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

        // NOTE(acol): Bind to pipeline
        vkCmdBindPipeline(VkCtx.CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          VkCtx.GraphicsPipeline);

        // NOTE(acol): Vertexes to draw and their indexes to avoid repeating, this is in GPU memory
        vkCmdBindVertexBuffers(VkCtx.CommandBuffer, 0, 1, &VkCtx.VertexBuffer, Offsets);
        //        vkCmdBindIndexBuffer(VkCtx.CommandBuffer, VkCtx.IndexBuffer, 0,
        //        VK_INDEX_TYPE_UINT32);

        // NOTE(acol): Draw command
        //        vkCmdDrawIndexed(VkCtx.CommandBuffer, ArrayCount(Indices), 1, 0, 0, 0);

        vkCmdDraw(VkCtx.CommandBuffer, ArrayCount(Vertices), 1, 0, 0);
        vkCmdEndRendering(VkCtx.CommandBuffer);

        if (vkEndCommandBuffer(VkCtx.CommandBuffer) != VK_SUCCESS) {
            dprintf(2, "failed to record command buffer\n");
            return 1;
        }

        // NOTE(acol): set up in gpu synchronization
        VkSemaphoreSubmitInfo WaitSemaInfo = {};
        WaitSemaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        WaitSemaInfo.semaphore = VkCtx.ImageSemaphore;
        WaitSemaInfo.value = 1;
        WaitSemaInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSemaphoreSubmitInfo SignalSemaInfo = {};
        SignalSemaInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
        SignalSemaInfo.semaphore = VkCtx.RenderSemaphore;
        SignalSemaInfo.value = 1;
        SignalSemaInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;

        VkCommandBufferSubmitInfo CommandBufferSubmitInfo = {};
        CommandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
        CommandBufferSubmitInfo.commandBuffer = VkCtx.CommandBuffer;

        // NOTE(acol): Submit command buffer with a fence
        VkSubmitInfo2 SubmitInfo = {};
        SubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
        SubmitInfo.waitSemaphoreInfoCount = 1;
        SubmitInfo.pWaitSemaphoreInfos = &WaitSemaInfo;
        SubmitInfo.commandBufferInfoCount = 1;
        SubmitInfo.pCommandBufferInfos = &CommandBufferSubmitInfo;
        SubmitInfo.signalSemaphoreInfoCount = 1;
        SubmitInfo.pSignalSemaphoreInfos = &SignalSemaInfo;

        vkQueueSubmit2(VkCtx.GraphicsQueue, 1, &SubmitInfo, VkCtx.InFlightFence);

        VkPresentInfoKHR PresentInfo = {};
        PresentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        PresentInfo.waitSemaphoreCount = 1;
        PresentInfo.pWaitSemaphores = &VkCtx.RenderSemaphore;
        PresentInfo.swapchainCount = 1;
        PresentInfo.pSwapchains = &VkCtx.Swapchain;
        PresentInfo.pImageIndices = &ImageIndex;
        PresentInfo.pResults = 0;
        Res = vkQueuePresentKHR(VkCtx.GraphicsQueue, &PresentInfo);

        if (Unlikely(Res != VK_SUCCESS || VkCtx.ShouldResize)) {
            if (Res == VK_ERROR_OUT_OF_DATE_KHR || VkCtx.ShouldResize || Res == VK_SUBOPTIMAL_KHR) {
                VkCtx.ShouldResize = 0;
                RecreateSwapchain(&VkCtx);
            } else {
                dprintf(2, "failed to present swapchain image\n");
                return 1;
            }
        }

        StartOfFrame += __rdtsc();
        FrameCount++;
        f64 Fps = 1000000 / (StartOfFrame / 3600);
        RollingAvg = (RollingAvg * (FrameCount - 1) + Fps) / FrameCount;
        //        printf("FPS: %f               \r", Fps);
    }
    printf("\n");
    printf(TXT_RED "Avg FPS: %f\n\n" TXT_RST, RollingAvg);
    vkDeviceWaitIdle(VkCtx.Device);

    /* ==============================================================================================

                                    // NOTE(acol): Vulkan terminate stuff

      =============================================================================================
    */
    VulkanDeletionQueueClear(&VkCtx, DeletionQueue);

    ArenaRelease(GlobalArena);
    // glfw terminate stuff
    glfwDestroyWindow(Window);
    glfwTerminate();

    EndAndPrintProfile();
    return 0;
}
