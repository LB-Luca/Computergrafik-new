#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <stdlib.h>
#include <string.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

//#include <imageloader/ImageLoader.h>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include <utils/model.h>
#include <utils/camera.h>
#include <utils/imgui_helper.h>

#include <vkal/src/examples/utils/platform.h>
#include <vkal/src/examples/utils/common.h>
#include <vkal/vkal.h>


struct ModelUBO
{
	glm::mat4 view;
	glm::mat4 projection;
	uint32_t texture_mapping_type; // 0 = object linear, 1 = eye linear, 2 = cubemap
};

struct SkyboxUBO
{
	glm::mat4 view;
	glm::mat4 projection;
};

struct MouseState
{
	double xpos, ypos;
	double xpos_old, ypos_old;
	uint32_t left_button_down;
	uint32_t right_button_down;
};

/* GLOBALS */
static int keys[GLFW_KEY_LAST];
static ModelUBO uniform_buffer;
static SkyboxUBO uniform_buffer_skybox;
static uint32_t framebuffer_resized;
static MouseState mouse_state;
static GLFWwindow * window;

std::vector<Vertex> cyl_vertices;
std::vector<uint16_t> cyl_indices;

#define CAM_SPEED			0.01f
#define CAM_SPEED_SLOW      (CAM_SPEED*0.1f)
#define MOUSE_SPEED			0.007f
#define WINDOW_WIDTH		1920
#define WINDOW_HEIGHT		1080


// GLFW callbacks
static void glfw_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		printf("escape key pressed\n");
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_W) {
			keys[GLFW_KEY_W] = 1;
		}
		if (key == GLFW_KEY_S) {
			keys[GLFW_KEY_S] = 1;
		}
		if (key == GLFW_KEY_A) {
			keys[GLFW_KEY_A] = 1;
		}
		if (key == GLFW_KEY_D) {
			keys[GLFW_KEY_D] = 1;
		}
		if (key == GLFW_KEY_E) {
			keys[GLFW_KEY_E] = 1;
		}
		if (key == GLFW_KEY_F) {
			keys[GLFW_KEY_F] = 1;
		}
		if (key == GLFW_KEY_LEFT) {
			keys[GLFW_KEY_LEFT] = 1;
		}
		if (key == GLFW_KEY_RIGHT) {
			keys[GLFW_KEY_RIGHT] = 1;
		}
		if (key == GLFW_KEY_UP) {
			keys[GLFW_KEY_UP] = 1;
		}
		if (key == GLFW_KEY_DOWN) {
			keys[GLFW_KEY_DOWN] = 1;
		}
		if (key == GLFW_KEY_PAGE_UP) {
			keys[GLFW_KEY_PAGE_UP] = 1;
		}
		if (key == GLFW_KEY_PAGE_DOWN) {
			keys[GLFW_KEY_PAGE_DOWN] = 1;
		}
		if (key == GLFW_KEY_LEFT_ALT) {
			keys[GLFW_KEY_LEFT_ALT] = 1;
		}
		if (key == GLFW_KEY_LEFT_SHIFT) {
			keys[GLFW_KEY_LEFT_SHIFT] = 1;
		}
	}
	else if (action == GLFW_RELEASE) {
		if (key == GLFW_KEY_W) {
			keys[GLFW_KEY_W] = 0;
		}
		if (key == GLFW_KEY_S) {
			keys[GLFW_KEY_S] = 0;
		}
		if (key == GLFW_KEY_A) {
			keys[GLFW_KEY_A] = 0;
		}
		if (key == GLFW_KEY_D) {
			keys[GLFW_KEY_D] = 0;
		}
		if (key == GLFW_KEY_E) {
			keys[GLFW_KEY_E] = 0;
		}
		if (key == GLFW_KEY_F) {
			keys[GLFW_KEY_F] = 0;
		}
		if (key == GLFW_KEY_LEFT) {
			keys[GLFW_KEY_LEFT] = 0;
		}
		if (key == GLFW_KEY_RIGHT) {
			keys[GLFW_KEY_RIGHT] = 0;
		}
		if (key == GLFW_KEY_UP) {
			keys[GLFW_KEY_UP] = 0;
		}
		if (key == GLFW_KEY_DOWN) {
			keys[GLFW_KEY_DOWN] = 0;
		}
		if (key == GLFW_KEY_PAGE_UP) {
			keys[GLFW_KEY_PAGE_UP] = 0;
		}
		if (key == GLFW_KEY_PAGE_DOWN) {
			keys[GLFW_KEY_PAGE_DOWN] = 0;
		}
		if (key == GLFW_KEY_LEFT_ALT) {
			keys[GLFW_KEY_LEFT_ALT] = 0;
		}
		if (key == GLFW_KEY_LEFT_SHIFT) {
			keys[GLFW_KEY_LEFT_SHIFT] = 0;
		}
	}
}

static void glfw_framebuffer_size_callback(GLFWwindow * window, int width, int height)
{
	framebuffer_resized = 1;
}

static void glfw_mouse_button_callback(GLFWwindow * window, int mouse_button, int action, int mods)
{
	if (mouse_button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mouse_state.left_button_down = 1;
	}
	if (mouse_button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mouse_state.left_button_down = 0;
	}
	if (mouse_button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		mouse_state.right_button_down = 1;
	}
	if (mouse_button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		mouse_state.right_button_down = 0;
	}
}

void init_window() {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan", 0, 0);
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);
}

void check_vk_result(VkResult err)
{
	if (err == 0) return;
	printf("VkResult %d\n", err);
	if (err < 0)
		abort();
}

Image load_image(const char* file)
{
	Image image = {};

	char exe_path[256];
	get_exe_path(exe_path, 256 * sizeof(char));

	std::string abs_path = concat_paths(get_assets_dir(), std::string(file));
	abs_path = concat_paths(std::string(exe_path), abs_path);

	int width, height, channels;
	unsigned char* data = stbi_load(abs_path.c_str(), &width, &height, &channels, 4);
	if (stbi_failure_reason() != NULL) {
		printf("[STB-Image] %s\n", stbi_failure_reason());
	}
	image.width = uint32_t(width);
	image.height = uint32_t(height);
	image.channels = 4; // force this, even if less than 4. 

	size_t imageSize = size_t(image.width * image.height * image.channels);
	image.data = (unsigned char*)malloc(imageSize);
	memcpy(image.data, data, imageSize);
	stbi_image_free(data);

	return image;
}

static void update_camera(Camera& camera, float dt)
{
	float camera_dolly_speed = CAM_SPEED;
	if (keys[GLFW_KEY_LEFT_SHIFT]) {
		camera_dolly_speed = CAM_SPEED_SLOW;
	}

	if (keys[GLFW_KEY_W]) {
		camera.MoveUp(camera_dolly_speed * dt);
	}
	if (keys[GLFW_KEY_S]) {
		camera.MoveUp(-camera_dolly_speed * dt);
	}
	if (keys[GLFW_KEY_A]) {
		camera.MoveSide(camera_dolly_speed * dt);
	}
	if (keys[GLFW_KEY_D]) {
		camera.MoveSide(-camera_dolly_speed * dt);
	}
	if (keys[GLFW_KEY_E]) {
		camera.MoveForward(camera_dolly_speed * dt);
	}
	if (keys[GLFW_KEY_F]) {
		camera.MoveForward(-camera_dolly_speed * dt);
	}
}

void update_camera_mouse_look(Camera& camera, float dt)
{
	if (mouse_state.left_button_down || mouse_state.right_button_down) {
		mouse_state.xpos_old = mouse_state.xpos;
		mouse_state.ypos_old = mouse_state.ypos;
		glfwGetCursorPos(window, &mouse_state.xpos, &mouse_state.ypos);
		double dx = mouse_state.xpos - mouse_state.xpos_old;
		double dy = mouse_state.ypos - mouse_state.ypos_old;
		if (mouse_state.left_button_down) {
			camera.RotateAroundSide(dy * MOUSE_SPEED);
			camera.RotateAroundUp(-dx * MOUSE_SPEED);
		}
	}
	else {
		glfwGetCursorPos(window, &mouse_state.xpos, &mouse_state.ypos);
		mouse_state.xpos_old = mouse_state.xpos;
		mouse_state.ypos_old = mouse_state.ypos;
	}
}

// TODO:
Model create_cylinder(float radius, float height, int segments)
{
	Model cylinder;
	cylinder.vertices = (Vertex*)malloc(segments * sizeof(Vertex));
	// Create vertices for the top and bottom circles
	for (int i = 0; i < segments; i = i + 2) {
		float theta = i * 2.0f * glm::pi<float>() / (segments);
		float x = radius * cos(theta);
		float z = radius * sin(theta);

		// Top circle vertex
		Vertex top_vert;
		top_vert.pos = glm::vec3(x, height / 2, z);
		//top_vert.id = i;
		cylinder.vertices[i].pos = top_vert.pos;
		cylinder.vertices[i].color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
		//cyl_vertices.push_back(top_vert);
		
		// Bottom circle vertex
		Vertex bot_vert;
		bot_vert.pos = glm::vec3(x, -height / 2, z);
		//bot_vert.id = i + 1;
		cylinder.vertices[i+1].pos = bot_vert.pos;
		cylinder.vertices[i+1].color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
		//cyl_vertices.push_back(bot_vert);
	}

	// Create the side faces ccw
	// A---C
	// | \ |
	// B---D
	for (int i = 0; i <= segments - 2; i = i + 2) {
		int A = i;
		int B = i + 1;
		int C = i + 2;
		int D = i + 3;

		// last face, connected back to first
		if (i == segments - 2) {
			C = 0;
			D = 1;
		}
		cyl_indices.push_back(A); cyl_indices.push_back(B); cyl_indices.push_back(D);
		cyl_indices.push_back(A); cyl_indices.push_back(D); cyl_indices.push_back(C);
		//cylinder.indices = cyl_indices;
	}
	return cylinder;
}

Model build_skybox_model()
{
	Model cube;
	cube.vertices = (Vertex*)malloc(36 * sizeof(Vertex));
	float cube_pos[] = {
		-10.0f, 10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, 10.0f, -10.0f,
		-10.0f, 10.0f, -10.0f,
		-10.0f, -10.0f, 10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f, 10.0f, -10.0f,
		-10.0f, 10.0f, -10.0f,
		-10.0f, 10.0f, 10.0f,
		-10.0f, -10.0f, 10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f, 10.0f,
		-10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, -10.0f, 10.0f,
		-10.0f, -10.0f, 10.0f,
		-10.0f, 10.0f, -10.0f,
		10.0f, 10.0f, -10.0f,
		10.0f, 10.0f, 10.0f,
		10.0f, 10.0f, 10.0f,
		-10.0f, 10.0f, 10.0f,
		-10.0f, 10.0f, -10.0f,
		-10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f, 10.0f,
		10.0f, -10.0f, -10.0f,
		10.0f, -10.0f, -10.0f,
		-10.0f, -10.0f, 10.0f,
		10.0f, -10.0f, 10.0f
	};
	for (int i = 0; i < 36; ++i) {
		cube.vertices[i].pos = glm::vec3(cube_pos[3 * i], cube_pos[3 * i + 1], cube_pos[3 * i + 2]);
	}
	cube.vertex_count = 36;
	return cube;
}

int main(int argc, char** argv) {
    
	init_window();

	std::string assets_dir = "../../assets";
	std::string shaders_dir = "../../A4_Cubemapping/shaders";
	if (argc > 1) {
		assets_dir = argv[1];
	}
	if (argc > 2) {
		shaders_dir = argv[2];
	}

	init_directories(assets_dir.c_str(), shaders_dir.c_str());

	char* device_extensions[] = {
		  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		  VK_KHR_MAINTENANCE3_EXTENSION_NAME
	};
	uint32_t device_extension_count = sizeof(device_extensions) / sizeof(*device_extensions);

	char* instance_extensions[] = {
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME
		#ifdef __APPLE__
			,VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME
		#endif
		#ifdef _DEBUG
			,VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		#endif
	};
	uint32_t instance_extension_count = sizeof(instance_extensions) / sizeof(*instance_extensions);

	char* instance_layers[] = {
		"VK_LAYER_KHRONOS_validation"
#if defined(WIN32) || defined(__linux__)
		,"VK_LAYER_LUNARG_monitor" // Not available on MacOS!
#endif
	};
	uint32_t instance_layer_count = 0;
#ifdef _DEBUG
	instance_layer_count = sizeof(instance_layers) / sizeof(*instance_layers);
#endif

	vkal_create_instance_glfw(window, instance_extensions, instance_extension_count, instance_layers, instance_layer_count);
	
	VkalPhysicalDevice* devices = 0;
	uint32_t device_count;
	vkal_find_suitable_devices(device_extensions, device_extension_count,
		&devices, &device_count);
	assert(device_count > 0);
	printf("Suitable Devices:\n");
	for (uint32_t i = 0; i < device_count; ++i) {
		printf("    Phyiscal Device %d: %s\n", i, devices[i].property.deviceName);
	}
	vkal_select_physical_device(&devices[0]);

	VkalWantedFeatures wanted_features{};
	VkalInfo* vkal_info = vkal_init(device_extensions, device_extension_count, wanted_features, VK_INDEX_TYPE_UINT16);

	init_imgui(window, vkal_info);

	// Descriptor Sets Setup
	VkDescriptorSetLayoutBinding set_layout_global[] = {
		{
			0, // binding id ( matches with shader )
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1, // number of resources with this layout binding
			VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			0
		},
		{
			1, // binding ID
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0
		}
	};
    

	VkDescriptorSetLayout descriptor_set_layout_global = vkal_create_descriptor_set_layout(set_layout_global, 2);
	VkDescriptorSetLayout descriptor_set_layouts[] = {
		descriptor_set_layout_global
	}; // Order in this array matches the set number in shaders. We only have one set, in this example.
	uint32_t descriptor_set_layout_count = sizeof(descriptor_set_layouts) / sizeof(*descriptor_set_layouts);
    
	// 3. Allocate Descriptor Set(s)
	// allocate a descriptor set for each shader from a Descriptor Pool with the Layout defined by
	// Descriptor Set Layout(s).
	VkDescriptorSet * descriptor_set_scene = (VkDescriptorSet*)malloc(sizeof(VkDescriptorSet));
	vkal_allocate_descriptor_sets(vkal_info->default_descriptor_pool, descriptor_set_layouts, 1, &descriptor_set_scene);
	VkDescriptorSet * descriptor_set = (VkDescriptorSet*)malloc(sizeof(VkDescriptorSet));
	vkal_allocate_descriptor_sets(vkal_info->default_descriptor_pool, descriptor_set_layouts, 1, &descriptor_set);
	VkDescriptorSet* descriptor_set_cylinder = (VkDescriptorSet*)malloc(sizeof(VkDescriptorSet));
	vkal_allocate_descriptor_sets(vkal_info->default_descriptor_pool, descriptor_set_layouts, 1, &descriptor_set_cylinder);

	/* Vertex Input Assembly */
	VkVertexInputBindingDescription vertex_input_bindings[] =
	{
		{ 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }
	};

	VkVertexInputAttributeDescription vertex_attributes[] =
	{
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },																				// position
		{ 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec3) },																	// texCoords
		{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec2) + sizeof(glm::vec3) },											// normal
		{ 3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) },						// color
		{ 4, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(glm::vec4) }		// tangent
	};
	uint32_t vertex_attribute_count = sizeof(vertex_attributes) / sizeof(*vertex_attributes);

	// Model shader
	uint8_t * vertex_byte_code = 0;
	int vertex_code_size;
	read_shader_file ("scene_vert.spv", &vertex_byte_code, &vertex_code_size);
	uint8_t * fragment_byte_code = 0;
	int fragment_code_size;
	read_shader_file("scene_frag.spv", &fragment_byte_code, &fragment_code_size);
	ShaderStageSetup shader_setup = vkal_create_shaders(vertex_byte_code, vertex_code_size, fragment_byte_code, fragment_code_size, NULL, 0);
	
    // Skybox shader
    read_shader_file("skybox_vert.spv", &vertex_byte_code, &vertex_code_size);
	read_shader_file("skybox_frag.spv", &fragment_byte_code, &fragment_code_size);
    ShaderStageSetup shader_setup_skybox = vkal_create_shaders(vertex_byte_code, vertex_code_size, fragment_byte_code, fragment_code_size, NULL, 0);
    
	// TODO: Some GPUs only have a max push constant size of 128 bytes!
	// Need to use a uniform buffer the Material.
	VkPushConstantRange push_constant_ranges[] =
	{
		{ // Model Matrix
			VK_SHADER_STAGE_VERTEX_BIT,
			0, // offset
			sizeof(glm::mat4)
		}
	};

    // Pipeline for regular models
	VkPipelineLayout pipeline_layout = vkal_create_pipeline_layout(
		descriptor_set_layouts, descriptor_set_layout_count, 
		push_constant_ranges, 1);
	
	VkPipeline model_pipeline = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
        shader_setup, 
        VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, 
        VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, 
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);
	
	// Another pipeline for rendering the skybox
	VkPipeline skybox_pipeline = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
        shader_setup_skybox, 
        VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL, 
        VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, 
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);
    
	
	// Uniform Buffers
	UniformBuffer model_ubo  = vkal_create_uniform_buffer(sizeof(ModelUBO), 1, 0);
	vkal_update_descriptor_set_uniform(descriptor_set_scene[0], model_ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	
	UniformBuffer skybox_ubo = vkal_create_uniform_buffer(sizeof(SkyboxUBO), 1, 0);
	vkal_update_descriptor_set_uniform(descriptor_set[0], skybox_ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	UniformBuffer cylinder_ubo = vkal_create_uniform_buffer(sizeof(ModelUBO), 1, 0);
	vkal_update_descriptor_set_uniform(descriptor_set_cylinder[0], cylinder_ubo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

    // Camera init
	Camera camera = Camera(glm::vec3(0, 0, 20));

	// Initial Projection Matrix 
	glm::mat4 projection = adjust_y_for_vulkan_ndc * glm::perspective(glm::radians(45.f), 1.f, 0.1f, 1000.0f);

	// Cubemap Textures
	Image cubemap_posx = load_image("textures/mountains_cubemap/posx.jpg");
	Image cubemap_negx = load_image("textures/mountains_cubemap/negx.jpg");
	Image cubemap_posy = load_image("textures/mountains_cubemap/posy.jpg");
	Image cubemap_negy = load_image("textures/mountains_cubemap/negy.jpg");
	Image cubemap_posz = load_image("textures/mountains_cubemap/posz.jpg");
	Image cubemap_negz = load_image("textures/mountains_cubemap/negz.jpg");
	Image cubemap_array[] = {
		cubemap_posx, cubemap_negx,
		cubemap_posy, cubemap_negy,
		cubemap_posz, cubemap_negz
	};
	uint32_t size_per_cubemap_image = cubemap_negx.width * cubemap_negx.height * 4;
	unsigned char* cubemap_data = (unsigned char*)malloc(6 * size_per_cubemap_image);
	unsigned char* cubemap_ptr = cubemap_data;
	for (int i = 0; i < 6; ++i) {
		memcpy(cubemap_ptr, cubemap_array[i].data, size_per_cubemap_image);
		cubemap_ptr += size_per_cubemap_image;
	}
	/*
	VkalTexture cubemap_texture = vkal_create_texture(
		1,
		cubemap_data,
		cubemap_negx.width, cubemap_negx.height, cubemap_negx.channels, 
		VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT,
		VK_IMAGE_VIEW_TYPE_CUBE,
		VK_FORMAT_R8G8B8A8_UNORM, 
		0, 1,
		0, 6,
		VK_FILTER_LINEAR, VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	vkal_update_descriptor_set_texture(descriptor_set[0], cubemap_texture);
	vkal_update_descriptor_set_texture(descriptor_set_scene[0], cubemap_texture);
	*/

	// Models
	Model model = create_model_from_file("obj/sphere_mit.obj");
	model.offset = vkal_vertex_buffer_add(model.vertices, sizeof(Vertex), model.vertex_count);
	model.pos = glm::vec3(0, -1.0f, 0);
	glm::mat4 scale = glm::scale(glm::mat4(1.f), glm::vec3(1.0f));	
	model.model_matrix = scale * glm::translate(glm::mat4(1.f), model.pos);;

	Model model_small = create_model_from_file("obj/sphere_mit.obj");
	model_small.offset = vkal_vertex_buffer_add(model_small.vertices, sizeof(Vertex), model_small.vertex_count);

	Model skybox_model = build_skybox_model();
	skybox_model.offset = vkal_vertex_buffer_add(skybox_model.vertices, sizeof(Vertex), skybox_model.vertex_count);
	skybox_model.pos = glm::vec3(0, 0, 0);
	skybox_model.model_matrix = glm::translate(glm::mat4(1.f), skybox_model.pos);

	float cyl_radius = 10;
	float cyl_height = 20;
	int cyl_segments = 32;
	Model cylinder_model = create_cylinder(cyl_radius, cyl_height, cyl_segments);
	cylinder_model.vertex_count = cyl_segments;
	cylinder_model.offset = vkal_vertex_buffer_add(cylinder_model.vertices, sizeof(Vertex), cylinder_model.vertex_count);
	cylinder_model.pos = glm::vec3(0, 0, 0);
	cylinder_model.index_count = cyl_indices.size();
	cylinder_model.index_buffer_offset = vkal_index_buffer_add(cyl_indices.data(), cylinder_model.index_count);
	cylinder_model.model_matrix = glm::mat4(1);

	// Cylinder Texture
	VkalTexture cylinder_texture = vkal_create_texture(
		1,
		cubemap_data,
		cubemap_negx.width, cubemap_negx.height, cubemap_negx.channels,
		0,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		0, 1,
		0, 1,
		VK_FILTER_LINEAR, VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	vkal_update_descriptor_set_texture(descriptor_set[0], cylinder_texture);
	vkal_update_descriptor_set_texture(descriptor_set_scene[0], cylinder_texture);
    
    //Timer Setup
	double timer_frequency = glfwGetTimerFrequency();
	double timestep = 1.f / timer_frequency; // step in second
	double title_update_timer = 0;
	double dt = 0;
    
	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;
	float foo = 1.0f;

	// MAIN LOOP
	while (!glfwWindowShouldClose(window)) {
        
		double start_time = glfwGetTime();
        
		glfwPollEvents();
        
		if (framebuffer_resized) {
			framebuffer_resized = 0;
			glfwGetFramebufferSize(window, &width, &height);
		}

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();

		ImGui::NewFrame();
		ImGui::Begin("Versuch 4 - Cubemapping");
		ImGui::Text("Select Texturing Mode");
		ImGui::RadioButton("Object Linear", (int*)&uniform_buffer.texture_mapping_type, 0);
		ImGui::RadioButton("Eye Linear", (int*)&uniform_buffer.texture_mapping_type, 1);
		ImGui::RadioButton("Cubemapping", (int*)&uniform_buffer.texture_mapping_type, 2);
		ImGui::End();

		model_small.pos.x = 10.0 * sinf(foo);
		model_small.pos.z = 10.0 * cosf(foo);
		model_small.model_matrix = glm::scale(glm::translate(glm::mat4(1), model_small.pos), glm::vec3(0.5f, 0.5f, 0.5f));
		foo += 0.5f * dt; 
		update_camera(camera, float(dt*1000.0f));
        
		// Mouse update
		if (!ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()) && !ImGui::IsAnyItemActive()) {
			update_camera_mouse_look(camera, dt);
		}
		
		uniform_buffer.projection = adjust_y_for_vulkan_ndc * glm::perspective(glm::radians(45.f), float(width) / float(height), 0.1f, 1000.f);
		uniform_buffer.view = glm::lookAt(camera.m_Pos, camera.m_Center, camera.m_Up);
		vkal_update_uniform(&model_ubo, &uniform_buffer);
        
		uniform_buffer_skybox.projection = adjust_y_for_vulkan_ndc * glm::perspective(glm::radians(45.f), float(width) / float(height), 0.1f, 1000.f);
		uniform_buffer_skybox.view = glm::lookAt(camera.m_Pos, camera.m_Center, camera.m_Up);
		vkal_update_uniform(&skybox_ubo, &uniform_buffer_skybox);
        

		{
			/* Acquire next image from Swapchain */
			uint32_t image_id = vkal_get_image();

			vkal_begin_command_buffer(image_id);
			vkal_begin_render_pass(image_id, vkal_info->render_pass);

			vkal_viewport(vkal_info->default_command_buffers[image_id],
				0, 0,
				(float)width, (float)height);
			vkal_scissor(vkal_info->default_command_buffers[image_id],
				0, 0,
				(float)width, (float)height);
			vkal_set_clear_color({ 0.6f, 0.6f, 0.6f, 1.0f });



			/* Bind descriptor set for Skybox and draw models */
			//vkal_bind_descriptor_set(image_id, &descriptor_set[0], pipeline_layout);
			//vkCmdPushConstants(vkal_info->default_command_buffers[image_id], pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &skybox_model.model_matrix);
			//vkal_draw(image_id, skybox_pipeline, skybox_model.offset, skybox_model.vertex_count);

			/* Bind descriptor set for rest of the scene and draw models */
			//vkal_bind_descriptor_set(image_id, &descriptor_set_scene[0], pipeline_layout);
			//vkCmdPushConstants(vkal_info->default_command_buffers[image_id], pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model.model_matrix);
			//vkal_draw(image_id, model_pipeline, model.offset, model.vertex_count); 
			//vkCmdPushConstants(vkal_info->default_command_buffers[image_id], pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &model_small.model_matrix);
			//vkal_draw(image_id, model_pipeline, model_small.offset, model_small.vertex_count);

			/* Bind descriptor set for Cylinder and draw model */
			vkal_bind_descriptor_set(image_id, &descriptor_set_cylinder[0], pipeline_layout);
			vkCmdPushConstants(vkal_info->default_command_buffers[image_id], pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &cylinder_model.model_matrix);
			vkal_draw_indexed(image_id, model_pipeline, cylinder_model.index_buffer_offset, cylinder_model.index_count, cylinder_model.offset, 1);

			// Rendering ImGUI
			ImGui::Render();
			ImDrawData* draw_data = ImGui::GetDrawData();
			// Record dear imgui primitives into command buffer
			ImGui_ImplVulkan_RenderDrawData(draw_data, vkal_info->default_command_buffers[image_id]);
			
			// End renderpass and submit the buffer to graphics-queue.
			vkal_end_renderpass(image_id);
			vkal_end_command_buffer(image_id);
			VkCommandBuffer command_buffers[] = { vkal_info->default_command_buffers[image_id] };
			vkal_queue_submit(command_buffers, 1);

			/* Present */
			vkal_present(image_id);			
		}
        
		double end_time = glfwGetTime();
		dt = end_time - start_time;
		title_update_timer += dt;
#ifndef _DEBUG
		if ((title_update_timer) > 1.f) {
			char window_title[256];
			sprintf(window_title, "frametime: %fms (%f FPS)", dt * 1000.f, 1.f / dt);
			glfwSetWindowTitle(window, window_title);
			title_update_timer = 0;
		}
#endif
	}
    
	deinit_imgui(vkal_info);

	free(descriptor_set);
	free(descriptor_set_scene);

	vkal_cleanup();

	glfwDestroyWindow(window);

	glfwTerminate();
    
	return 0;
}

