#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_vulkan.h>

#include <utils/model.h>
#include <utils/camera.h>
#include <utils/imgui_helper.h>

#include <vkal/src/examples/utils/platform.h>
#include <vkal/src/examples/utils/common.h>
#include <vkal/vkal.h>

#include <modelloader/Model.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GL_PI 3.14159f

struct ViewProjectionUBO
{
	glm::mat4 model;
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
GLFWwindow* window;
static int keys[GLFW_KEY_LAST];
static ViewProjectionUBO view_projection_data;
static uint32_t framebuffer_resized;
static MouseState mouse_state;
static bool update_pipeline;


#define CAM_SPEED			0.1f
#define CAM_SPEED_SLOW      (CAM_SPEED*0.1f)
#define MOUSE_SPEED			0.007f



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

static void glfw_framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	framebuffer_resized = 1;
}

static void glfw_mouse_button_callback(GLFWwindow* window, int mouse_button, int action, int mods)
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

GLFWwindow* init_window(int width, int height, char const* title) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	GLFWwindow* window = glfwCreateWindow(width, height, title, 0, 0);
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);

	return window;
}

//je 18 Vertices anlegen f�r Konus und Boden
const int segments = 18; 
static Vertex konusVertices[segments];
static Vertex bodenVertices[segments];
void setupGeometry()
{
	// Die Spitze des Konus ist ein Vertex, den alle Triangles gemeinsam haben;
	// um einen Konus anstatt einen Kreis zu produzieren muss der Vertex einen positiven y-Wert haben
	konusVertices[0].pos = glm::vec3(0, 75, 0);
	konusVertices[0].color = glm::vec4(0, 0.6, 1, 1);
	// Erzeuge einen weiteren Triangle_Fan um den Boden zu bedecken
	// Das Zentrum des Triangle_Fans ist im Ursprung
	bodenVertices[0].pos = glm::vec3(0, 0, 0);
	bodenVertices[0].color = glm::vec4(0, 0.8, 0, 1);
	// Kreise um den Mittelpunkt und spezifiziere Vertices entlang des Kreises
	// um einen Triangle_Fan zu erzeugen
	int iPivot = 1;
	float angle = 2.0f * GL_PI;
	for (int i = 1; i < segments; i++)
	{
		// Berechne x und z Positionen des naechsten Vertex
		float x = 50.0f * sin(i * angle / (segments - 2));
		float z = 50.0f * cos(i * angle / (segments - 2));

		// Alterniere die Farbe
		if ((iPivot % 2) == 0) {
			konusVertices[i].color = glm::vec4(0.235, 0.235, 0.235, 1);
			bodenVertices[i].color = glm::vec4(1, 0.8, 0.2, 1);
		}
		else {
			konusVertices[i].color = glm::vec4(0, 0.6, 1, 1);
			bodenVertices[i].color = glm::vec4(0, 0.8, 0, 1);
		}
		// Inkrementiere iPivot um die Farbe beim naechsten mal zu wechseln
		iPivot++;

		// Spezifiziere den naechsten Vertex des Konus_Triangle_Fans
		konusVertices[i].pos = glm::vec3(x, 0, z);
		// Spezifiziere den naechsten Vertex des Boden_Triangle_Fans
		bodenVertices[i].pos = glm::vec3(x, 0, z);
	}
}

void update_modelMatrix(glm::mat4& mMatrix, float dt)
{
	float camera_dolly_speed = CAM_SPEED;
	if (keys[GLFW_KEY_LEFT_SHIFT]) {
		camera_dolly_speed = CAM_SPEED_SLOW;
	}

	if (keys[GLFW_KEY_LEFT]) {
		mMatrix = glm::rotate(mMatrix, camera_dolly_speed * dt * glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	}
	if (keys[GLFW_KEY_RIGHT]) {
		mMatrix = glm::rotate(mMatrix, -camera_dolly_speed * dt * glm::radians(30.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	}
	if (keys[GLFW_KEY_PAGE_UP]) {
		mMatrix = glm::rotate(mMatrix, camera_dolly_speed * dt * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	if (keys[GLFW_KEY_PAGE_DOWN]) {
		mMatrix = glm::rotate(mMatrix, -camera_dolly_speed * dt * glm::radians(30.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	}
	if (keys[GLFW_KEY_UP]) {
		mMatrix = glm::rotate(mMatrix, camera_dolly_speed * dt * glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	}
	if (keys[GLFW_KEY_DOWN]) {
		mMatrix = glm::rotate(mMatrix, -camera_dolly_speed * dt * glm::radians(30.0f), glm::vec3(0.0f, 0.0f, 1.0f));
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

int main(int argc, char** argv) {
	window = init_window(800, 600, "A1: Versuch 1c: Geometry & Transforms");

	std::string assets_dir = "../../assets";
	std::string shaders_dir = "../../A1_Versuch1a/shaders";
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
	wanted_features.features2.features.fillModeNonSolid = VK_TRUE;
	VkalInfo* vkal_info = vkal_init(device_extensions, device_extension_count, wanted_features, VK_INDEX_TYPE_UINT16);

	init_imgui(window, vkal_info);

	/* Configure the Vulkan Pipeline and Uniform Buffers*/
	// Descriptor Sets
	VkDescriptorSetLayoutBinding set_layout[] = {
		{
			0, // binding id ( matches with shader )
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1, // number of resources with this layout binding
			VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			0
		}
	};

	VkDescriptorSetLayout descriptor_set_layout_1 = vkal_create_descriptor_set_layout(set_layout, 1);

	VkDescriptorSetLayout layouts[] = {
		descriptor_set_layout_1
	};
	uint32_t descriptor_set_layout_count = sizeof(layouts) / sizeof(*layouts);

	VkDescriptorSet* descriptor_set_1 = (VkDescriptorSet*)malloc(descriptor_set_layout_count * sizeof(VkDescriptorSet));
	vkal_allocate_descriptor_sets(vkal_info->default_descriptor_pool, layouts, 1, &descriptor_set_1);

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
	read_shader_file("vert.spv", &vertex_byte_code, &vertex_code_size);
	uint8_t* fragment_byte_code = 0;
	int fragment_code_size;
	read_shader_file("frag.spv", &fragment_byte_code, &fragment_code_size);
	ShaderStageSetup shader_setup = vkal_create_shaders(vertex_byte_code, vertex_code_size, fragment_byte_code, fragment_code_size, NULL, 0);

	VkPipelineLayout pipeline_layout = vkal_create_pipeline_layout(layouts, descriptor_set_layout_count, NULL, 0);

	VkPipeline pipeline_filled_polys = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	VkPipeline pipeline_filled_polys_no_cull = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	VkPipeline pipeline_filled_polys_no_depth = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	VkPipeline pipeline_filled_polys_no_depth_no_cull = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	VkPipeline pipeline_lines = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_LINE,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	VkPipeline pipeline_lines_no_depth = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_LINE,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	VkPipeline pipeline_lines_no_cull = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_NONE, VK_POLYGON_MODE_LINE,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	VkPipeline pipeline_lines_no_depth_no_cull = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_NONE, VK_POLYGON_MODE_LINE,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	// Uniform Buffers
	UniformBuffer uniform_buffer_handle = vkal_create_uniform_buffer(sizeof(ViewProjectionUBO), 1, 0);
	vkal_update_descriptor_set_uniform(descriptor_set_1[0], uniform_buffer_handle, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	// Initialize Uniform Buffer
	vkal_update_uniform(&uniform_buffer_handle, &view_projection_data);

	// Build cone geometry
	setupGeometry();
	Model coneMantle{};
	coneMantle.vertex_count = 18;
	coneMantle.offset = vkal_vertex_buffer_add(konusVertices, sizeof(Vertex), 18);
	Model coneBase{};
	coneBase.vertex_count = 18;
	coneBase.offset = vkal_vertex_buffer_add(bodenVertices, sizeof(Vertex), 18);

	// ModelMatrix
	glm::mat4 modelMatrix = glm::mat4(1);
	// Camera
	Camera camera = Camera(glm::vec3(0, 0, 100));

	// User Interface 
	double timer_frequency = glfwGetTimerFrequency();
	double timestep = 1.f / timer_frequency; // step in second
	double title_update_timer = 0;
	double dt = 0;

	int width = 800;
	int height = 600;

	// MAIN LOOP
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	bool enableDepthTest = true;
	bool enableCulling = true;
	bool enableFillMode = true;
	VkPipeline active_pipeline = pipeline_filled_polys;
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

		{
			ImGui::Begin("Versuch 1");
			ImGui::Checkbox("Enable Depth test", &enableDepthTest);
			ImGui::Checkbox("Enable Face culling", &enableCulling);
			ImGui::Checkbox("Enable Fill Mode", &enableFillMode);
			ImGui::End();
		}

		update_modelMatrix(modelMatrix, float(dt * 100.f));
		update_camera(camera, float(dt * 1000.f));


		// Mouse update
		if (!ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()) && !ImGui::IsAnyItemActive()) {
			update_camera_mouse_look(camera, dt*1000.0f);
		}

		view_projection_data.model = modelMatrix;
		view_projection_data.projection = adjust_y_for_vulkan_ndc * glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, -1000.0f, 1000.0f);
		view_projection_data.view = glm::lookAt(camera.m_Pos, camera.m_Center, camera.m_Up);
		vkal_update_uniform(&uniform_buffer_handle, &view_projection_data);

		// Select pipeline
		if (enableDepthTest && enableCulling && enableFillMode) {
			active_pipeline = pipeline_filled_polys;
		}
		else if (enableDepthTest && !enableCulling && enableFillMode) {
			active_pipeline = pipeline_filled_polys_no_cull;
		}
		else if (!enableDepthTest && enableCulling && enableFillMode) {
			active_pipeline = pipeline_filled_polys_no_depth;
		}
		else if (!enableDepthTest && !enableCulling && enableFillMode){
			active_pipeline = pipeline_filled_polys_no_depth_no_cull;
		}
		else if (enableDepthTest && enableCulling && !enableFillMode) {
			active_pipeline = pipeline_lines;
		}
		else if (enableDepthTest && !enableCulling && !enableFillMode) {
			active_pipeline = pipeline_lines_no_cull;
		}
		else if (!enableDepthTest && enableCulling && !enableFillMode) {
			active_pipeline = pipeline_lines_no_depth;
		}
		else if (!enableDepthTest && !enableCulling && !enableFillMode) {
			active_pipeline = pipeline_lines_no_depth_no_cull;
		}

		{
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

			vkal_bind_descriptor_set(image_id, &descriptor_set_1[0], pipeline_layout);
			vkal_draw(image_id, active_pipeline, coneBase.offset, coneBase.vertex_count);
			vkal_draw(image_id, active_pipeline, coneMantle.offset, coneMantle.vertex_count);

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

			vkal_present(image_id);
		}


		double end_time = glfwGetTime();
		dt = end_time - start_time;
		title_update_timer += dt;

		if ((title_update_timer) > .5f) {
			char window_title[256];
			sprintf(window_title, "frametime: %fms (%f FPS)", (dt * 1000.f), (1000.f) / (dt * 1000.f));
			glfwSetWindowTitle(window, window_title);
			title_update_timer = 0;
		}
	}

	deinit_imgui(vkal_info);

	free(descriptor_set_1);

	vkal_cleanup();

	glfwDestroyWindow(window);

	glfwTerminate();

	return 0;
}

