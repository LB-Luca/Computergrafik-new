#include <vkal/vkal.h>

#include <GLFW/glfw3.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

static void check_vk_result(VkResult err)
{
	if (err == 0) return;
		printf("VkResult %d\n", err);
	if (err < 0)
		abort();
}

void init_imgui(GLFWwindow* window, VkalInfo* vkal_info)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	if (ImGui_ImplGlfw_InitForVulkan(window, true)) {
		printf("ImGui initialized.\n");
	}
	else {
		printf("Could not initialize ImGui!\n");
		exit(-1);
	}

	ImGui_ImplVulkan_InitInfo init_info = {};
	init_info.Instance = vkal_info->instance;
	init_info.PhysicalDevice = vkal_info->physical_device;
	init_info.Device = vkal_info->device;
	QueueFamilyIndicies qf = find_queue_families(vkal_info->physical_device, vkal_info->surface);
	init_info.QueueFamily = qf.graphics_family;
	init_info.Queue = vkal_info->graphics_queue;
	init_info.PipelineCache = VK_NULL_HANDLE;
	init_info.DescriptorPool = vkal_info->default_descriptor_pool;
	init_info.Subpass = 0;
	init_info.MinImageCount = 2;
	init_info.ImageCount = vkal_info->swapchain_image_count;
	init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	init_info.Allocator = NULL;
	init_info.CheckVkResultFn = check_vk_result;
	if (ImGui_ImplVulkan_Init(&init_info, vkal_info->render_pass)) {
		printf("ImGui Vulkan part initialized.\n");
	}
	else {
		printf("ImGui Vulkan part NOT initialized!\n");
		exit(-1);
	}

	// Upload Fonts
	{
		VkResult err;
		VkCommandBuffer font_cmd_buffer = vkal_create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
		ImGui_ImplVulkan_CreateFontsTexture(font_cmd_buffer);
		err = vkEndCommandBuffer(font_cmd_buffer);
		check_vk_result(err);
		VkSubmitInfo submit_info = {  };
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1; // vkal_info.default_command_buffer_count;
		submit_info.pCommandBuffers = &font_cmd_buffer;  //vkal_info.default_command_buffers;
		err = vkQueueSubmit(vkal_info->graphics_queue, 1, &submit_info, VK_NULL_HANDLE);
		check_vk_result(err);
		err = vkDeviceWaitIdle(vkal_info->device);
		check_vk_result(err);
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}
}

void deinit_imgui(VkalInfo* vkal_info)
{
	VkResult err = vkDeviceWaitIdle(vkal_info->device);
	check_vk_result(err);
	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}