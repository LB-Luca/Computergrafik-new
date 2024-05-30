#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <stdio.h>

#define GLM_FORCE_RADIANS
// necessary so that glm::perspective produces z-clip coordinates 0<z_clip<w_clip
// see: https://matthewwellings.com/blog/the-new-vulkan-coordinate-system/
// see: https://twitter.com/pythno/status/1230478042096709632
#define GLM_FORCE_DEPTH_ZERO_TO_ONE 

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include <vkal/src/examples/utils/platform.h>
#include <vkal/src/examples/utils/glslcompile.h>
#include <vkal/src/examples/utils/common.h>

#include <utils/model.h>
#include <utils/camera.h>
#include <utils/imgui_helper.h>
#include <vkal/vkal.h>
#include <vulkan/vulkan.h>

#define GL_PI 3.1415f

#pragma pack(push, 16)
struct SceneUBO
{
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 light_view_mat;
	glm::mat4 light_proj_mat;

	glm::vec4 light_pos;

	glm::vec2 light_radius_uv;
	float	  light_camera_near;
	float	  light_camera_far;
	uint32_t  filter_method;
	uint32_t  poisson_disk_size;
	uint32_t  use_textures;
	float     light_radius_bias;

	glm::vec3 model_pos;
};
#pragma pack(pop, 16)

struct ShadowUBO
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
static SceneUBO scene_data;
static ShadowUBO shadow_data;
static uint32_t framebuffer_resized;
static MouseState mouse_state;
static GLFWwindow * window;

#define CAM_SPEED			0.1f
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
	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "A5_PCSS", 0, 0);
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


void update_LightModelPos(glm::vec3& pos, float dt)
{
	float camera_dolly_speed = CAM_SPEED;
	if (keys[GLFW_KEY_LEFT_SHIFT]) {
		camera_dolly_speed = CAM_SPEED_SLOW;
	}

	if (keys[GLFW_KEY_LEFT]) {
		pos += camera_dolly_speed * dt * glm::vec3(1.0f, 0.0f, 0.0f);
	}
	if (keys[GLFW_KEY_RIGHT]) {
		pos += -camera_dolly_speed * dt * glm::vec3(1.0f, 0.0f, 0.0f);
	}
	if (keys[GLFW_KEY_UP]) {
		pos += camera_dolly_speed * dt * glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (keys[GLFW_KEY_DOWN]) {
		pos += -camera_dolly_speed * dt * glm::vec3(0.0f, 1.0f, 0.0f);
	}
	if (keys[GLFW_KEY_PAGE_UP]) {
		pos += camera_dolly_speed * dt * glm::vec3(0.0f, 0.0f, 1.0f);
	}
	if (keys[GLFW_KEY_PAGE_DOWN]) {
		pos += -camera_dolly_speed * dt * glm::vec3(0.0f, 0.0f, 1.0f);
	}
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

glm::vec3 transformCoord(glm::mat4 m, glm::vec3 v)
{
	const float zero = (0.0);
	const float one = (1.0);
	glm::vec4 r = m * glm::vec4(v, one);
	float oow = r.w == zero ? one : (one / r.w);
	return glm::vec3(r) * oow;
}

int main(int argc, char** argv) {

	init_window();

	std::string assets_dir = "../../assets";
	std::string shaders_dir = "../../A5_PCSS/shaders";
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

	VkalWantedFeatures features{};
	
	VkalWantedFeatures wanted_features{};
	wanted_features.features12.runtimeDescriptorArray = VK_TRUE;
	VkalInfo* vkal_info = vkal_init(device_extensions, device_extension_count, wanted_features, VK_INDEX_TYPE_UINT16);

	init_imgui(window, vkal_info);

	/* Configure the Vulkan Pipeline and Uniform Buffers*/
	// Descriptor Sets
	VkDescriptorSetLayoutBinding set_layout_scene[] = {
		{   // Material information
			0, // binding id ( matches with shader )
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
			1, // number of resources with this layout binding
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0
		},
		{   // view-, proijection matrices of both main camera and light-camera, light position, etc.
			1, // binding id ( matches with shader )
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1, // number of resources with this layout binding
			VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			0
		},
		{
			2,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			2, /* up to 2 textures */
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0
		},
		{
			3,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0
		}
	};
	VkDescriptorSetLayout descriptor_set_layout_scene = vkal_create_descriptor_set_layout(set_layout_scene, 4);

	VkDescriptorSetLayoutBinding set_layout_depth_sampler[] = {
		{
			0, // binding id ( matches with shader )
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1, // number of resources with this layout binding
			VK_SHADER_STAGE_VERTEX_BIT,
			0
		}
	};
	VkDescriptorSetLayout descriptor_set_layout_depth_sampler = vkal_create_descriptor_set_layout(set_layout_depth_sampler, 1);

	/* The order of the entries determines how the shaders access the sets! */
	VkDescriptorSetLayout descriptor_set_layouts[] =
	{
		descriptor_set_layout_scene,			/* in shader: set = 0 */
		descriptor_set_layout_depth_sampler		/* in shader: set = 1 */
	};
	uint32_t descriptor_set_layout_count = sizeof(descriptor_set_layouts) / sizeof(*descriptor_set_layouts);

	VkDescriptorSet* descriptor_sets = (VkDescriptorSet*)malloc(descriptor_set_layout_count * sizeof(VkDescriptorSet)); // 2 Sets, on for the depth map, one for the main scene
	vkal_allocate_descriptor_sets(vkal_info->default_descriptor_pool, descriptor_set_layouts, descriptor_set_layout_count, &descriptor_sets);

	/* Vertex Input Assembly */
	VkVertexInputBindingDescription vertex_input_bindings[] =
	{
		{ 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX }
	};

	VkVertexInputAttributeDescription vertex_attributes[] =
	{
		{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 },																				// position
		{ 1, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(glm::vec3) },																	// UV
		{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec2) + sizeof(glm::vec3) },											// normal
		{ 3, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) },						// color
		{ 4, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(glm::vec3) + sizeof(glm::vec2) + sizeof(glm::vec3) + sizeof(glm::vec4) }		// tangent
	};
	uint32_t vertex_attribute_count = sizeof(vertex_attributes) / sizeof(*vertex_attributes);

	// Shaders
	uint8_t* vertex_byte_code = 0;
	int vertex_code_size;
	load_glsl_and_compile("/shader.vert", &vertex_byte_code, &vertex_code_size, SHADER_TYPE_VERTEX);

	uint8_t* fragment_byte_code = 0;
	int fragment_code_size;
	load_glsl_and_compile("/shader.frag", &fragment_byte_code, &fragment_code_size, SHADER_TYPE_FRAGMENT);
	ShaderStageSetup shader_setup = vkal_create_shaders(
		vertex_byte_code, vertex_code_size,
		fragment_byte_code, fragment_code_size,
		NULL, 0);
	free(vertex_byte_code);
	free(fragment_byte_code);

	load_glsl_and_compile("/shadowmap.vert", &vertex_byte_code, &vertex_code_size, SHADER_TYPE_VERTEX);
	load_glsl_and_compile("/shadowmap.frag", &fragment_byte_code, &fragment_code_size, SHADER_TYPE_FRAGMENT);
	ShaderStageSetup shader_setup_shadowmap = vkal_create_shaders(
		vertex_byte_code, vertex_code_size,
		fragment_byte_code, fragment_code_size,
		NULL, 0);
	free(vertex_byte_code);
	free(fragment_byte_code);

	VkPushConstantRange push_constant_ranges[] =
	{
		{ // Model Matrix
			VK_SHADER_STAGE_VERTEX_BIT,
			0, // offset
			sizeof(glm::mat4) // size, not sure if this has to be aligned... probably...
		}
	};

	VkPipelineLayout pipeline_layout = vkal_create_pipeline_layout(descriptor_set_layouts, descriptor_set_layout_count, push_constant_ranges, 1);
	VkPipelineLayout pipeline_layout_shadowmap = vkal_create_pipeline_layout(&descriptor_set_layouts[1], 1, push_constant_ranges, 1);

	VkPipeline graphics_pipeline = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FRONT_FACE_COUNTER_CLOCKWISE, vkal_info->render_pass, pipeline_layout);

	VkPipeline shadow_pipeline = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup_shadowmap, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, VK_FRONT_FACE_COUNTER_CLOCKWISE, vkal_info->render_to_image_render_pass, pipeline_layout_shadowmap);

	/* Use Textures at startup */
	scene_data.use_textures = 1;

	/* Model Loading and Texture assignments */
	Model model_plane = create_model_from_file("/obj/plane.obj");
	model_plane.offset = vkal_vertex_buffer_add(model_plane.vertices, sizeof(Vertex), model_plane.vertex_count);
	glm::mat4 plane_scale = glm::scale(glm::mat4(1), glm::vec3(10));
	glm::mat4 translate = glm::translate(glm::mat4(1), glm::vec3(0.0f, 0.0f, 0.0f));
	glm::mat4 model_mat = translate * plane_scale;
	model_plane.pos = glm::vec3(0.0f, 0.0f, 0.0f);
	model_plane.model_matrix = glm::translate(glm::mat4(1), model_plane.pos) * plane_scale;
	model_plane.material.diffuse = glm::vec4(1, 1, 1, 1);

	Model model_tree = create_model_from_file("/obj/tree_single.obj");
	model_tree.offset = vkal_vertex_buffer_add(model_tree.vertices, sizeof(Vertex), model_tree.vertex_count);
	model_tree.pos = glm::vec3(0.f, 0.f, 0.f);
	translate = glm::translate(glm::mat4(1), model_tree.pos);
	model_tree.model_matrix = translate * glm::scale(glm::mat4(1), glm::vec3(10.0f));
	model_tree.material.diffuse = glm::vec4(1.0, 0.0, 0.0, 1);

	Model model_light = create_model_from_file("/obj/sphere_mit.obj");
	model_light.offset = vkal_vertex_buffer_add(model_light.vertices, sizeof(Vertex), model_light.vertex_count);
	model_light.material.emissive = glm::vec4(1, 1, 1, 1);
	model_light.pos = glm::vec3(2.5f, 40.0f, -35.0f);
	model_light.model_matrix = glm::translate(glm::mat4(1), model_light.pos);



	Image sand_image = load_image("/textures/sand_diffuse.jpg");
	Image tree_image = load_image("/textures/sand_diffuse.jpg");
	
	VkalTexture plane_texture = vkal_create_texture(2, sand_image.data, sand_image.width, sand_image.height, 4, 0,
		VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, 0, 1, 0, 1, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	VkalTexture tree_texture = vkal_create_texture(2, tree_image.data, tree_image.width, tree_image.height, 4, 0,
		VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM, 0, 1, 0, 1, VK_FILTER_LINEAR, VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	
	assign_texture_to_model(&model_plane, plane_texture, 0, TEXTURE_TYPE_DIFFUSE);
	assign_texture_to_model(&model_tree, tree_texture, 1, TEXTURE_TYPE_DIFFUSE);
	model_plane.material.is_textured = 1;
	model_tree.material.is_textured = 1;
	model_light.material.is_textured = 0;
	
#define NUM_MODELS 3 // Change this if you remove/add models to the scene!
	Model * models[NUM_MODELS] = {
		&model_plane,
		&model_tree,
		&model_light
	};

	/* Uniform Buffers */
	// Global for the whole scene
	UniformBuffer uniform_buffer_handle = vkal_create_uniform_buffer(sizeof(SceneUBO), 1, 1);
	vkal_update_descriptor_set_uniform(descriptor_sets[0], uniform_buffer_handle, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	vkal_update_uniform(&uniform_buffer_handle, &scene_data);

	// Dynamic Uniform Buffer since for each model the material data may be different
	UniformBuffer material_uniform_buffer = vkal_create_uniform_buffer(sizeof(Material), NUM_MODELS,	 0);
	Material* materials = (Material*)malloc(NUM_MODELS * material_uniform_buffer.alignment);
	for (size_t i = 0; i < NUM_MODELS; i++) {
		*((Material*)((uint8_t*)materials + i*material_uniform_buffer.alignment)) = models[i]->material;
	}
	vkal_update_descriptor_set_uniform(descriptor_sets[0], material_uniform_buffer, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
	vkal_update_uniform(&material_uniform_buffer, materials);

	// Uniform for shadow map
	UniformBuffer uniform_buffer_handle_shadow = vkal_create_uniform_buffer(sizeof(ShadowUBO), 1, 0);
	vkal_update_descriptor_set_uniform(descriptor_sets[1], uniform_buffer_handle_shadow, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	vkal_update_uniform(&uniform_buffer_handle_shadow, &shadow_data);

	/* Update Descriptor Sets */
	vkal_update_descriptor_set_texturearray(
		descriptor_sets[0], 
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		model_plane.material.texture_id, 
		model_plane.texture);
	vkal_update_descriptor_set_texturearray(
		descriptor_sets[0], 
		VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
		model_tree.material.texture_id, 
		model_tree.texture);

	/* Render Image */
	RenderImage render_image = create_render_image(4096, 4096);
	VkSampler sampler = create_sampler(
		VK_FILTER_LINEAR, VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER,
		VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
	vkal_update_descriptor_set_render_image(
		descriptor_sets[0], 3,
		get_image_view(render_image.depth_image.image_view), sampler);

	// Init Camera
	Camera camera = Camera(glm::vec3(70, 70, 70));

	// Init Shadow Data Stuff
	Camera light_cam = Camera(glm::vec3(scene_data.light_pos));

	double timer_frequency = glfwGetTimerFrequency();
	double timestep = 1.f / timer_frequency; // step in second
	double title_update_timer = 0;
	double dt = 0;
	float light_animator = 0.f;

	int width = WINDOW_WIDTH;
	int height = WINDOW_HEIGHT;

	/* Set default information for the light-camera's setting */
	scene_data.light_radius_bias = 1.5f; // Must be at least 1.0f
	scene_data.light_camera_near = 10.0f;
	scene_data.light_camera_far = 200.0f;
	scene_data.filter_method = 0;
	scene_data.poisson_disk_size = 6; // 32x32 fixed

	/* Light Camera */
	float light_camera_fov = 90.0f;

	// MAIN LOOP
	while (!glfwWindowShouldClose(window)) {
		double start_time = glfwGetTime();

		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		ImGui::Begin("Shadow Mapping");
		ImGui::Checkbox("Use Textures", (bool*)&scene_data.use_textures);
		
		ImGui::Text("Filter");
		ImGui::RadioButton("PCF", (int*)&scene_data.filter_method, 0);
		ImGui::RadioButton("PCSS", (int*)&scene_data.filter_method, 1);
		ImGui::RadioButton("No Shadow", (int*)&scene_data.filter_method, 2);
		
		if (scene_data.filter_method == 1) {
			ImGui::Text("Poisson Disk");
			ImGui::RadioButton("25", (int*)&scene_data.poisson_disk_size, 0);
			ImGui::RadioButton("32", (int*)&scene_data.poisson_disk_size, 1);
			ImGui::RadioButton("64", (int*)&scene_data.poisson_disk_size, 2);
			if (scene_data.poisson_disk_size >2) scene_data.poisson_disk_size = 0;
		}
		else if (scene_data.filter_method == 0) {
			ImGui::Text("Shadow Sampler"); 
			ImGui::RadioButton("2x2 fixed", (int*)&scene_data.poisson_disk_size, 3);
			ImGui::RadioButton("4x4 fixed", (int*)&scene_data.poisson_disk_size, 4);
			ImGui::RadioButton("16x16 fixed", (int*)&scene_data.poisson_disk_size, 5);
			ImGui::RadioButton("32x32 fixed", (int*)&scene_data.poisson_disk_size, 6);
			ImGui::RadioButton("64x64 fixed", (int*)&scene_data.poisson_disk_size, 7);
			if (scene_data.poisson_disk_size < 3) scene_data.poisson_disk_size = 3;
		}
		ImGui::Text("Light Camera Settings");
		ImGui::SliderFloat("Search Radius Bias", &scene_data.light_radius_bias, 1.0, 10.0);
		ImGui::SliderFloat3("Light Position", &model_light.pos[0], -50, 50);
		static float light_size = 0.5f;
		ImGui::SliderFloat("Light Size", &light_size, 0, 10);
		ImGui::SliderFloat("FOV", &light_camera_fov, 0.0f, 180.0f);
		ImGui::SliderFloat("Near Plane", &scene_data.light_camera_near, 1.0f, 50.0f);
		ImGui::SliderFloat("Far Plane", &scene_data.light_camera_far, 50.1f, 1000.0f);

		ImGui::Text("Plane Model Properties");
		ImGui::SliderFloat3("Position", (float*)&scene_data.model_pos, -10.f, 10.f);

		ImGui::End();

		/* Update Light Model Matrix */
		update_LightModelPos(model_light.pos, float(dt * 100.f));
		model_light.model_matrix = glm::translate(glm::mat4(1), model_light.pos);

		// Need new dimensions?
		if (framebuffer_resized) {
			framebuffer_resized = 0;
			glfwGetFramebufferSize(window, &width, &height);
		}

		update_camera(camera, float(dt*1000.0f));

		// Mouse update
		if (!ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()) && !ImGui::IsAnyItemActive()) {
			update_camera_mouse_look(camera, dt);
		}

		/* Update view and projection matrices for main camera and set the light's position */
		scene_data.view = glm::lookAt(camera.m_Pos, camera.m_Center, camera.m_Up);
		scene_data.projection = adjust_y_for_vulkan_ndc * glm::perspective(glm::radians(45.f), float(width) / float(height), 0.1f, 1000.f);
		scene_data.light_pos = glm::vec4(model_light.pos, 1);

		/*update light view projection*/
		{
			light_cam = Camera(model_light.pos);
			float frustum_width = glm::sin(glm::radians(light_camera_fov) / 2.0f) * scene_data.light_camera_near * 2;
			float frustum_height = frustum_width;
			scene_data.light_radius_uv = glm::vec2(light_size / frustum_width, light_size / frustum_height);
			shadow_data.view = glm::lookAt(light_cam.m_Pos, light_cam.m_Center, light_cam.m_Up);
			frustum_height = glm::max(frustum_height, 0.01f); // Make sure no division by 0!
			shadow_data.projection = adjust_y_for_vulkan_ndc * glm::perspective(glm::radians(light_camera_fov), frustum_width/frustum_height, scene_data.light_camera_near, scene_data.light_camera_far);
			scene_data.light_view_mat = shadow_data.view;
			scene_data.light_proj_mat = shadow_data.projection;
		}

		vkal_update_uniform(&uniform_buffer_handle, &scene_data);
		vkal_update_uniform(&uniform_buffer_handle_shadow, &shadow_data);

		/* Plane model's position is adjustable */
		model_plane.model_matrix = glm::translate(glm::mat4(1), model_plane.pos + scene_data.model_pos) * plane_scale;

		uint32_t image_id = vkal_get_image();
		 {
			vkal_begin_command_buffer(image_id);

			// Render scene into a shadowmap from the light's perspective
			{
				vkal_begin_render_to_image_render_pass(image_id, vkal_info->default_command_buffers[image_id],
					vkal_info->render_to_image_render_pass, render_image);

				vkal_viewport(vkal_info->default_command_buffers[image_id], 0, 0, render_image.width, render_image.height);
				vkal_scissor(vkal_info->default_command_buffers[image_id], 0, 0, render_image.width, render_image.height);

				vkal_bind_descriptor_set(image_id, &descriptor_sets[1], pipeline_layout_shadowmap);

				vkCmdPushConstants(
					vkal_info->default_command_buffers[image_id],
					pipeline_layout_shadowmap, VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(glm::mat4),
					&model_plane.model_matrix);
				vkal_draw(image_id, shadow_pipeline, model_plane.offset, model_plane.vertex_count);

				vkCmdPushConstants(
					vkal_info->default_command_buffers[image_id],
					pipeline_layout_shadowmap, VK_SHADER_STAGE_VERTEX_BIT,
					0, sizeof(glm::mat4),
					&model_tree.model_matrix);
				vkal_draw(image_id, shadow_pipeline, model_tree.offset, model_tree.vertex_count);

				vkal_end_renderpass(image_id);
			}

			// Render scene from main camera's perspective
			vkal_begin_render_pass(image_id, vkal_info->render_pass); // Scene and ImGUI share renderpass.
			{

				vkal_viewport(vkal_info->default_command_buffers[image_id], 0, 0, width, height);
				vkal_scissor(vkal_info->default_command_buffers[image_id], 0, 0, width, height);

				// Set Model matrices via Push Constants and Materials using dynamic uniform buffer. Finally, draw model.
				for (uint64_t i = 0; i < NUM_MODELS; i++) {
					Model * model = models[i];				
					vkCmdPushConstants(
						vkal_info->default_command_buffers[image_id],
						pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT,
						0, sizeof(glm::mat4),
						&model->model_matrix
					);
					uint32_t dynamic_offset = (uint32_t)(i * (material_uniform_buffer.alignment));
					vkal_bind_descriptor_set_dynamic(
						image_id,
						&descriptor_sets[0],
						pipeline_layout,
						dynamic_offset
					);
					vkal_draw(image_id, graphics_pipeline, model->offset, model->vertex_count);
				}
			}

			// Rendering ImGUI
			{
				ImGui::Render();
				ImDrawData* draw_data = ImGui::GetDrawData();
				// Record dear imgui primitives into command buffer
				ImGui_ImplVulkan_RenderDrawData(draw_data, vkal_info->default_command_buffers[image_id]);

				// End renderpass and submit the buffer to graphics-queue.
			}
			vkal_end_renderpass(image_id);

			vkal_end_command_buffer(image_id);

			// Submit to GPU
			VkCommandBuffer command_buffers[] = { vkal_info->default_command_buffers[image_id] };
			vkal_queue_submit(command_buffers, 1);
			
			/* Blit to surface */
			vkal_present(image_id);
		}
		  
		double end_time = glfwGetTime();
		dt = end_time - start_time;
		title_update_timer += dt;

		if ((title_update_timer) > 1.f) {
			char window_title[256];
			sprintf(window_title, "frametime: %fms (%f FPS)", dt * 1000.f, 1.f / dt);
			glfwSetWindowTitle(window, window_title);
			title_update_timer = 0;
		}
	}

	deinit_imgui(vkal_info);

	vkal_cleanup();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

