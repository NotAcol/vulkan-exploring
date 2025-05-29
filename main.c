#include <vulkan/vulkan.h>
#include "base/base_include.h"
thread_local tctx* TctxThreadLocal;
#include "base/base_include.c"

#if VK_VALIDATE
    #define VulkanLog(...) printf(__VA_ARGS__)

    #define VALIDATION_LAYERS "VK_LAYER_KHRONOS_validation"
    #define VK_REQUIRED_INSTANCE_EXTENSIONS \
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME, VK_EXT_DEBUG_UTILS_EXTENSION_NAME
#else
    #define VulkanLog(...)

    #define VALIDATION_LAYERS
    #define VK_REQUIRED_INSTANCE_EXTENSIONS VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
#endif

#define VK_REQUIRED_DEVICE_EXTENSIONS \
    VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME

typedef struct vulkan_context {
    arena* Arena;
    VkInstance Instance;
#if VK_VALIDATE
    VkDebugUtilsMessengerEXT MessengerHandle;
#endif
} vulkan_context;

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType,
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

int main(void) {
    BeginProfile();
    tctx Tctx;
    TctxInitAndEquip(&Tctx);

    wayland_context WlCtx = {0};
    WaylandInit(&WlCtx, 100, 100, String8Lit("Test"));

    vulkan_context VkCtx = {0};

    VkCtx.Arena = ArenaAlloc();
    arena* ShaderArena = ArenaAlloc({.ReserveSize = MB(4), .CommitSize = KB(4)});

    /* =============================================================================================

                                    // NOTE(acol): Vulkan init stuff

      =========================================================================================== */

    VkApplicationInfo AppInfo = {0};
    AppInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName = "Triangle of vulkan and misery";
    AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.pEngineName = "Hella learning rn my nigga";
    AppInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.apiVersion = VK_API_VERSION_1_4;

    VkInstanceCreateInfo CreateInfo = {0};
    CreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo = &AppInfo;
    CreateInfo.enabledExtensionCount = ArrayCount(VULKAN_WINDOWING_EXTENSIONS);
    CreateInfo.ppEnabledExtensionNames = VULKAN_WINDOWING_EXTENSIONS;
    CreateInfo.enabledLayerCount = 0;

    u32 ExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &ExtensionCount, 0);
    VkExtensionProperties* ExtensionProperties =
        (VkExtensionProperties*)ArenaPush(VkCtx.Arena, sizeof(VkExtensionProperties) * ExtensionCount);
    vkEnumerateInstanceExtensionProperties(0, &ExtensionCount, ExtensionProperties);
    VulkanLog("Available extensions\n");
    for (u32 i = 0; i < ExtensionCount; i++) {
        VulkanLog("   %s\n", ExtensionProperties[i].extensionName);
    }

    u32 LayerCount = 0;
    vkEnumerateInstanceLayerProperties(&LayerCount, 0);
    VkLayerProperties* LayerProperties =
        (VkLayerProperties*)ArenaPush(VkCtx.Arena, sizeof(VkLayerProperties) * LayerCount);
    vkEnumerateInstanceLayerProperties(&LayerCount, LayerProperties);
    VulkanLog("Available layers\n");
    for (u32 j = 0; j < LayerCount; j++) {
        VulkanLog("   %s\n", LayerProperties[j].layerName);
    }

    WaylandTerminate(&WlCtx);
    TctxRelease();
    EndAndPrintProfile();
    return 0;
}
