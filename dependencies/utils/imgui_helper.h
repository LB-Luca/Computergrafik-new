#ifndef _IMGUI_HELPER_H_
#define _IMGUI_HELPER_H_

#include <vkal/vkal.h>
#include <GLFW/glfw3.h>

static void check_vk_result(VkResult err);
void        init_imgui(GLFWwindow* window, VkalInfo* vkal_info);
void        deinit_imgui(VkalInfo* vkal_info);

#endif