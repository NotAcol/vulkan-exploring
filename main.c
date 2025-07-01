#include "base/base_include.h"
thread_local tctx* TctxThreadLocal;
#include "base/base_include.c"

C_LINKAGE const char* __asan_default_options(void) { return "detect_leaks=0"; }

#define VK_USE_PLATFORM_WAYLAND_KHR
#include <vulkan/vulkan.h>

#if VULKAN_DEBUG
    #define VulkanLog(...) dprintf(2, __VA_ARGS__)
    #define VALIDATION_LAYERS "VK_LAYER_KHRONOS_validation",
    #define VALIDATION_EXTENSIONS VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#else
    #define VulkanLog(...)
    #define VALIDATION_LAYERS
    #define VALIDATION_EXTENSIONS
#endif

#define VULKAN_REQUIRED_INSTANCE_LAYERS VALIDATION_LAYERS
#define VULKAN_REQUIRED_INSTANCE_EXTENSIONS \
    VULKAN_WINDOWING_INSTANCE_EXTENSIONS    \
    VALIDATION_EXTENSIONS VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME

#define VK_REQUIRED_DEVICE_EXTENSIONS \
    VULKAN_WINDOWING_DEVICE_EXTENSIONS VK_EXT_VERTEX_INPUT_DYNAMIC_STATE_EXTENSION_NAME

typedef struct vulkan_context {
    arena* Arena;
    VkInstance Instance;

#if VULKAN_DEBUG
    VkDebugUtilsMessengerEXT MessengerHandle;
#endif
} vulkan_context;

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT MessageSeverity, VkDebugUtilsMessageTypeFlagsEXT MessageType,
    const VkDebugUtilsMessengerCallbackDataEXT* CallbackData, void* UserData) {
    if (MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        // dprintf(2, TXT_UWHT "INFO" TXT_RST ": %s\n", CallbackData->pMessage);
    } else if (MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        dprintf(2, TXT_UBLU "VERBOSE" TXT_BLU ": " TXT_RST "%s\n", CallbackData->pMessage);
    } else if (MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        dprintf(2, TXT_UYEL "WARNING" TXT_YEL ": " TXT_RST "%s\n", CallbackData->pMessage);
    } else if (MessageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        dprintf(2, TXT_URED "ERROR" TXT_RED ":" TXT_RST " %s\n", CallbackData->pMessage);
    }
    // NOTE(acol): turn off warning for unused variables
    (void)MessageType;
    (void)UserData;
    return VK_FALSE;
}

int main(void) {
    BeginProfile();
    tctx Tctx;
    TctxInitAndEquip(&Tctx);

    ProfileBandwidthStart(wayland, 0);
    wayland_context WlCtx = {0};
    WaylandInit(&WlCtx, 100, 100, String8Lit("Test"));
    ProfileBandwidthEnd(wayland);

    vulkan_context VkCtx = {0};

    VkCtx.Arena        = ArenaAlloc();
    arena* ShaderArena = ArenaAlloc(.ReserveSize = MB(4), .CommitSize = KB(4));

    /* =============================================================================================

                                    // NOTE(acol): Vulkan init stuff

      =========================================================================================== */

    VkApplicationInfo AppInfo  = {0};
    AppInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    AppInfo.pApplicationName   = "Triangle of vulkan and misery";
    AppInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.pEngineName        = "Hella learning rn my nigga";
    AppInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    AppInfo.apiVersion         = VK_API_VERSION_1_4;

    const char* RequiredLayers[]             = {VULKAN_REQUIRED_INSTANCE_LAYERS};
    const char* RequiredInstanceExtensions[] = {VULKAN_REQUIRED_INSTANCE_EXTENSIONS};

    ProfileBandwidthStart(Vulkan_Instance, 0);
    VkInstanceCreateInfo CreateInfo    = {0};
    CreateInfo.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    CreateInfo.pApplicationInfo        = &AppInfo;
    CreateInfo.enabledExtensionCount   = ArrayCount(RequiredInstanceExtensions);
    CreateInfo.ppEnabledExtensionNames = RequiredInstanceExtensions;
    CreateInfo.ppEnabledLayerNames     = RequiredLayers;
    CreateInfo.enabledLayerCount       = ArrayCount(RequiredLayers);

#if VULKAN_DEBUG
    // NOTE(acol): Some init info if validation layers are on
    u32 LayerCount = 0;
    vkEnumerateInstanceLayerProperties(&LayerCount, 0);
    VkLayerProperties* LayerProperties =
        (VkLayerProperties*)PushArray(VkCtx.Arena, VkLayerProperties, LayerCount);
    vkEnumerateInstanceLayerProperties(&LayerCount, LayerProperties);
    VulkanLog("Instance Layers\n");
    for (u32 j = 0; j < LayerCount; j++) {
        VulkanLog("   %s\n", LayerProperties[j].layerName);
    }
    u32 ExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(0, &ExtensionCount, 0);
    VkExtensionProperties* ExtensionProperties =
        (VkExtensionProperties*)PushArray(VkCtx.Arena, VkExtensionProperties, ExtensionCount);
    vkEnumerateInstanceExtensionProperties(0, &ExtensionCount, ExtensionProperties);
    VulkanLog("\nInstance Extensions\n");
    for (u32 i = 0; i < ExtensionCount; i++) {
        VulkanLog("   %s\n", ExtensionProperties[i].extensionName);
    }
    VulkanLog("\n");

    VkDebugUtilsMessengerCreateInfoEXT MessengerInfo = {0};
    MessengerInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    MessengerInfo.messageSeverity =
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    MessengerInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    MessengerInfo.pfnUserCallback = VulkanDebugCallback;

    // NOTE(acol): This is needed to log instance creation before we can CreateDebugUtilsMessenger
    CreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&MessengerInfo;
#endif

    if (vkCreateInstance(&CreateInfo, 0, &VkCtx.Instance) != VK_SUCCESS) {
        dprintf(2, "Well shit\n");
        return 1;
    }
    ProfileBandwidthEnd(Vulkan_Instance);

#if VULKAN_DEBUG
    PFN_vkCreateDebugUtilsMessengerEXT CreateMessengerCallback =
        (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(VkCtx.Instance,
                                                                  "vkCreateDebugUtilsMessengerEXT");
    CreateMessengerCallback(VkCtx.Instance, &MessengerInfo, 0, &VkCtx.MessengerHandle);
#endif

    // NOTE(acol): select GPU
    u32 PhysicalDeviceCount = 0;
    vkEnumeratePhysicalDevices(VkCtx.Instance, &PhysicalDeviceCount, 0);
    VkPhysicalDevice* PhysicalDevices =
        (VkPhysicalDevice*)PushArray(VkCtx.Arena, VkPhysicalDevice, PhysicalDeviceCount);
    vkEnumeratePhysicalDevices(VkCtx.Instance, &PhysicalDeviceCount, PhysicalDevices);

    const char* RequiredExtensions[] = {VK_REQUIRED_DEVICE_EXTENSIONS};

    VkPhysicalDevice PhysicalDevice = {0};

    b32 DeviceFound = 0;
    for (u32 i = 0; i < PhysicalDeviceCount && !DeviceFound; i++) {
        PhysicalDevice                         = PhysicalDevices[i];
        VkPhysicalDeviceProperties2 Properties = {.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2};
        vkGetPhysicalDeviceProperties2(PhysicalDevice, &Properties);

        u32 ExtensionCount = 0;
        vkEnumerateDeviceExtensionProperties(PhysicalDevice, 0, &ExtensionCount, 0);
        VkExtensionProperties* Extensions =
            (VkExtensionProperties*)PushArray(VkCtx.Arena, VkExtensionProperties, ExtensionCount);
        vkEnumerateDeviceExtensionProperties(PhysicalDevice, 0, &ExtensionCount, Extensions);
        VulkanLog("\nDevice %s Extensions\n", Properties.properties.deviceName);
        for (u32 i = 0; i < ExtensionCount; i++) {
            VulkanLog("    %s\n", Extensions[i].extensionName);
        }
        VulkanLog("\n");

        u64 RequiredExtensionCount = ArrayCount(RequiredExtensions);
        u64 ExtensionsFound        = 0;

        for (u64 i = 0; i < RequiredExtensionCount && ExtensionsFound != RequiredExtensionCount; i++) {
            b32 Found = 0;
            for (u64 j = 0; j < ExtensionCount && !Found; j++) {
                if (strcmp(RequiredExtensions[i], Extensions[j].extensionName) == 0) {
                    Found = 1;
                    ExtensionsFound++;
                }
            }
            if (!Found) {
                dprintf(2, "\nFailed to find extension %s\n", RequiredExtensions[i]);
                *(volatile int*)0 = 0;
            }
        }
    }

    WaylandTerminate(&WlCtx);
    TctxRelease();
    EndAndPrintProfile();
    return 0;
}
