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
#include <imageloader/ImageLoader.h>

#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <stdlib.h>
#include <string.h>

#define GL_PI 3.14159f

struct ViewProjectionUBO
{
	glm::mat4 view;
	glm::mat4 projection;
};

struct SettingsUBO
{
	glm::vec4 light_pos;
	glm::vec4 light_ambient;
	glm::vec4 light_diffuse;
	glm::vec4 light_specular;
	uint32_t show_normals;
};

struct MaterialUBO
{
	glm::vec4 emissive;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
};

struct ModelUBO
{
	glm::mat4 model_mat;
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
static SettingsUBO settings_data;
static MaterialUBO material_data;
static ModelUBO model_data;
static uint32_t framebuffer_resized;
static MouseState mouse_state;
static bool update_pipeline;
static bool draw_normals;
static bool phong;
static Model g_model;
static Model g_model_normals;
static float angle_threshold = 45.0;
std::vector<obj::Vertex> cyl_vertices;
std::vector<int> cyl_indices;

#define CAM_SPEED			0.01f
#define CAM_SPEED_SLOW      (CAM_SPEED*0.1f)
#define MOUSE_SPEED			0.007f
#define GL_PLANAR_THRESHHOLD 0.001f //Degree for planar triangles



// GLFW callbacks
static void glfw_key_callback(GLFWwindow * window, int key, int scancode, int action, int mods)
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

GLFWwindow* init_window(int width, int height, char const * title) {
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	window = glfwCreateWindow(width, height, title, 0, 0);
	glfwSetKeyCallback(window, glfw_key_callback);
	glfwSetMouseButtonCallback(window, glfw_mouse_button_callback);
	glfwSetFramebufferSizeCallback(window, glfw_framebuffer_size_callback);

	return window;
}

// Funktion zum berechnen des obj::Vetor normal aller Triangles
void calculateAndStoreNormal(obj::Triangle* triangle)
{
	if (!triangle || !triangle->vertex[0] || !triangle->vertex[1] || !triangle->vertex[2]) {
		return;
	}

	// Berechnung der Vektoren des Dreiecks
	glm::vec3 v0 = glm::vec3(triangle->vertex[0]->pos.x, triangle->vertex[0]->pos.y, triangle->vertex[0]->pos.z);
	glm::vec3 v1 = glm::vec3(triangle->vertex[1]->pos.x, triangle->vertex[1]->pos.y, triangle->vertex[1]->pos.z);
	glm::vec3 v2 = glm::vec3(triangle->vertex[2]->pos.x, triangle->vertex[2]->pos.y, triangle->vertex[2]->pos.z);

	// Berechnung des Normalenvektors mit dem Kreuzprodukt
	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;
	glm::vec3 normal = glm::cross(edge1, edge2);

	// Normalisierung des Normalenvektors
	normal = glm::normalize(normal);

	// Speicherung des normalisierten Normalenvektors in jedem Vertex des Dreiecks
	triangle->vertex[0]->normal = { normal.x,normal.y,normal.z };
	triangle->vertex[1]->normal = { normal.x,normal.y,normal.z };
	triangle->vertex[2]->normal = { normal.x,normal.y,normal.z };
	triangle->normal = { normal.x,normal.y,normal.z };
}

//Funktion zur Berechnung des Winkels in Grad zwischen zwei glm::vec3 Vektoren
float angleBetweenVectors(const glm::vec3& vec1, const glm::vec3& vec2)
{
	// Berechnung des Skalarprodukts
	float dotProduct = glm::dot(vec1, vec2);

	// Berechnung der Längen der Vektoren
	float lengthVec1 = glm::length(vec1);
	float lengthVec2 = glm::length(vec2);

	// Berechnung des Cosinus des Winkels
	float cosTheta = dotProduct / (lengthVec1 * lengthVec2);

	// Errorhandling
	if (cosTheta > 1.0) {cosTheta = 1.0;}
	else if (cosTheta < 0.0) {cosTheta = 0.0;}

	// Arccos zur Berechnung des Winkels in Radiant
	float angleRadians = acos(cosTheta);

	// Konvertierung des Winkels in Grad
	float angleDegrees = glm::degrees(angleRadians);

	return angleDegrees;
}

// TODO:
void create_cylinder(float radius, float height, int segments)
{
	// Create vertices for the top and bottom circles
	for (int i = 0; i < segments; i=i+2) {
		float theta = i * 2.0f * glm::pi<float>() / (segments/2);
		float x = radius * cos(theta);
		float z = radius * sin(theta);

		// Top circle vertex
		obj::Vertex top_vert;
		top_vert.pos = obj::Vector3(x, height / 2, z);
		top_vert.id = i;
		cyl_vertices.push_back(top_vert);
		// Bottom circle vertex
		obj::Vertex bot_vert;
		bot_vert.pos = obj::Vector3(x, -height / 2, z);
		bot_vert.id = i+1;
		cyl_vertices.push_back(bot_vert);
	}

	// Create the side faces ccw
	// A---C
	// | \ |
	// B---D
	for (int i = 0; i <= segments-2; i=i+2) {
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
	}
}


void setup_geometry(Model& model)
{
	const int segments = 32; // muss gerade Zahl sein
	float radius = 10;
	float height = 30;
	create_cylinder(radius, height, segments);
	g_model.vertex_count = segments;
	g_model.offset = vkal_vertex_buffer_add(&cyl_vertices, sizeof(Vertex), g_model.vertex_count);
	g_model.index_count = cyl_indices.size();
	g_model.index_buffer_offset = vkal_index_buffer_add(cyl_indices.data(), g_model.index_count);

	/*
	//obj::Model model("../assets/obj/bunny.obj");
	g_model.vertices = (Vertex*)malloc(3 * model.GetTriangleCount() * sizeof(Vertex));
	g_model_normals.vertices = (Vertex*)malloc(2 * 3 * model.GetTriangleCount() * sizeof(Vertex));
	uint32_t normal_index = 0;

	// Initialize normals for Triangles
	for (int i = 0; i < model.GetTriangleCount(); ++i) {
		obj::Triangle* tri = model.GetTriangle(i);
		calculateAndStoreNormal(tri);
	}

	for (int i = 0; i < model.GetTriangleCount(); ++i) {
		obj::Triangle* tri = model.GetTriangle(i);
		glm::vec3 face_normal = glm::vec3(1);

		for (int j = 0; j < 3; ++j) {
			Vertex vertex = tri.vertex[j];
			glm::vec3 vertex_pos = glm::vec3(vertex.pos.x, vertex.pos.y, vertex.pos.z);
			g_model.vertices[3 * i + j].pos = vertex_pos;

			std::vector<Tri> adj_tris;
			// mit der Methode GetAdjacentTriangles erhält man alle Dreiecke, die einen gemeinsamen Vertex besitzen
			//model.GetAdjacentTriangles(adj_tris, vertex);
			adj_tris = get_adjacent_triangles(&model, vertex);
			std::vector<glm::vec3> normals;
			normals.push_back(face_normal);

			glm::vec3 mean_normal(0);
			mean_normal = glm::vec3(1); 
			g_model.vertices[3 * i + j].normal = mean_normal;
			g_model_normals.vertices[normal_index++].pos = vertex_pos;
			g_model_normals.vertices[normal_index++].pos = vertex_pos + 0.5f * mean_normal;
		}
		
	}

	g_model.vertex_count = 3 * model.GetTriangleCount();
	g_model_normals.vertex_count = normal_index - 1;
	g_model.pos = glm::vec3(0);
	g_model_normals.pos = glm::vec3(0);
	glm::mat4 translate = glm::translate(glm::mat4(1), g_model.pos);
	g_model.model_matrix = translate;
	g_model_normals.model_matrix = translate;
	g_model.offset = vkal_vertex_buffer_add(g_model.vertices, sizeof(Vertex), g_model.vertex_count);
	g_model_normals.offset = vkal_vertex_buffer_add(g_model_normals.vertices, sizeof(Vertex), g_model_normals.vertex_count);
	g_model_normals.material.emissive = glm::vec4(0, 0.1, 0, 1);
	g_model.material.emissive = glm::vec4(0.0, 0, 0.0, 1);
	g_model.material.ambient = glm::vec4(0.3, 0, 0.0, 1);
	g_model.material.diffuse = glm::vec4(0.0, 1.0, 0.0, 1);
	g_model.material.specular = glm::vec4(0.0f, 0.0f, 1.0f, 10.0f);
	*/
}

void update_geometry(Model& model)
{

	const int segments = 32; // muss gerade Zahl sein
	float radius = 10;
	float height = 30;
	//create_cylinder(radius, height, segments);
	// TODO:
	//vertex_buffer_update
	//index_buffer_update
	
	uint32_t normal_index = 0;
	bool skip = false; // for skipping duplicate normal vectors

	/*
	// Initialize normals for Triangles
	for (int i = 0; i < model.GetTriangleCount(); ++i) {
		obj::Triangle* tri = model.GetTriangle(i);
		calculateAndStoreNormal(tri);
	}

	for (int i = 0; i < model.GetTriangleCount(); ++i) {
		obj::Triangle* tri = model.GetTriangle(i);
		auto normal_vec = tri->normal;
		glm::vec3 face_normal = glm::vec3(normal_vec.x, normal_vec.y, normal_vec.z);

		for (int j = 0; j < 3; ++j) {
			obj::Vertex* vertex = tri->vertex[j];
			glm::vec3 vertex_pos = glm::vec3(vertex->pos.x, vertex->pos.y, vertex->pos.z);
			g_model.vertices[3 * i + j].pos = vertex_pos;

			std::vector<obj::Triangle*> adj_tris;
			model.GetAdjacentTriangles(adj_tris, vertex);

			glm::vec3 mean_normal(0);

			std::vector<glm::vec3> unique_vec;
			unique_vec.push_back(face_normal);

			// iterate over adjacent triangles
			for (auto* adj_triangle : adj_tris) {
				auto adj_normal_vec = adj_triangle->normal;
				glm::vec3 adj_normal = glm::vec3(adj_normal_vec.x, adj_normal_vec.y, adj_normal_vec.z);

				skip = false;

				// search for adjacent normal vectors that match conditions (degree)
				if (angleBetweenVectors(adj_normal, face_normal) < angle_threshold && angleBetweenVectors(adj_normal, face_normal) > GL_PLANAR_THRESHHOLD) {

					// check for duplicate vector
					for (glm::vec3 uv : unique_vec) {
						if (angleBetweenVectors(adj_normal, uv) < GL_PLANAR_THRESHHOLD) {
							skip = true;
						}
					}
					if (!skip) {
						unique_vec.push_back(adj_normal);
						//mean_normal += adj_normal;
					}
				}
			}
			// mean normalenvektor aufaddieren und normieren
			for (glm::vec3 uv : unique_vec) {
				mean_normal += uv;
			}
			mean_normal = glm::normalize(mean_normal);

			g_model.vertices[3 * i + j].normal = mean_normal;
			g_model_normals.vertices[normal_index++].pos = vertex_pos;
			g_model_normals.vertices[normal_index++].pos = vertex_pos + 0.5f * mean_normal;
		}
	}

	g_model_normals.vertex_count = normal_index - 1;
	*/
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
	GLFWwindow* window = init_window(800, 600, "A4: Normalenvektoren");

	std::string assets_dir = "../../assets";
	std::string shaders_dir = "../../A4_Normalenvektoren/shaders";
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

	/* Configure the Vulkan Pipeline and Uniform Buffers*/
	// Descriptor Sets
	VkDescriptorSetLayoutBinding set_layout[] = {
		{
			0, // binding id ( matches with shader )
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1, // number of resources with this layout binding
			VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			0
		},
		{
			1,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT | VK_SHADER_STAGE_VERTEX_BIT,
			0
		},
		{
			2,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
			0
		},
		{
			3,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			1,
			VK_SHADER_STAGE_VERTEX_BIT,
			0
		},
		{
			4,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			0
		}
	};

	VkDescriptorSetLayout descriptor_set_layout_1 = vkal_create_descriptor_set_layout(set_layout, 5);

	VkDescriptorSetLayout layouts[] = {
		descriptor_set_layout_1
	};
	uint32_t descriptor_set_layout_count = sizeof(layouts)/sizeof(*layouts);

	VkDescriptorSet * descriptor_set_1 = (VkDescriptorSet*)malloc(descriptor_set_layout_count * sizeof(VkDescriptorSet));
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
	uint8_t * vertex_byte_code = 0;
	int vertex_code_size;
	read_shader_file("gouraud_vert.spv", &vertex_byte_code, &vertex_code_size);
	uint8_t * fragment_byte_code = 0;
	int fragment_code_size;
	read_shader_file("gouraud_frag.spv", &fragment_byte_code, &fragment_code_size);
	ShaderStageSetup shader_setup = vkal_create_shaders(vertex_byte_code, vertex_code_size, fragment_byte_code, fragment_code_size, NULL, 0);

	uint8_t* vertex_byte_code_phong = 0;
	int vertex_code_size_phong;
	read_shader_file("phong_vert.spv", &vertex_byte_code_phong, &vertex_code_size_phong);
	uint8_t* fragment_byte_code_phong = 0;
	int fragment_code_size_phong;
	read_shader_file("phong_frag.spv", &fragment_byte_code_phong, &fragment_code_size_phong);
	ShaderStageSetup shader_setup_phong = vkal_create_shaders(vertex_byte_code_phong, vertex_code_size_phong, fragment_byte_code_phong, fragment_code_size_phong, NULL, 0);

	VkPipelineLayout pipeline_layout = vkal_create_pipeline_layout(layouts, descriptor_set_layout_count, NULL, 0);

	VkPipeline pipeline_filled_polys_gouraud = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL, 
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	VkPipeline pipeline_filled_polys_phong = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup_phong, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_BACK_BIT, VK_POLYGON_MODE_FILL,
		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	VkPipeline pipeline_lines = vkal_create_graphics_pipeline(
		vertex_input_bindings, 1,
		vertex_attributes, vertex_attribute_count,
		shader_setup, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL, VK_CULL_MODE_NONE, VK_POLYGON_MODE_FILL,
		VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
		VK_FRONT_FACE_COUNTER_CLOCKWISE,
		vkal_info->render_pass, pipeline_layout);

	// Uniform Buffers
	UniformBuffer uniform_buffer_handle = vkal_create_uniform_buffer(sizeof(ViewProjectionUBO), 1, 0);
	UniformBuffer uniform_buffer_handle2 = vkal_create_uniform_buffer(sizeof(SettingsUBO), 1, 1);
	UniformBuffer uniform_buffer_material = vkal_create_uniform_buffer(sizeof(MaterialUBO), 1, 2);
	UniformBuffer uniform_buffer_model = vkal_create_uniform_buffer(sizeof(ModelUBO), 1, 3);

	vkal_update_descriptor_set_uniform(descriptor_set_1[0], uniform_buffer_handle, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	vkal_update_descriptor_set_uniform(descriptor_set_1[0], uniform_buffer_handle2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	vkal_update_descriptor_set_uniform(descriptor_set_1[0], uniform_buffer_material, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
	vkal_update_descriptor_set_uniform(descriptor_set_1[0], uniform_buffer_model, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);

	// Initialize Uniform Buffer
	settings_data.show_normals = 1;
	settings_data.light_ambient = glm::vec4(1, 1, 1, 1);
	settings_data.light_diffuse = glm::vec4(1, 1, 1, 1);
	settings_data.light_specular = glm::vec4(1, 1, 1, 1);
	settings_data.light_pos = glm::vec4(100, 0, 0, 0);
	vkal_update_uniform(&uniform_buffer_handle, &view_projection_data);
	vkal_update_uniform(&uniform_buffer_handle2, &settings_data);

	// TODO:
	Image mantle_img = load_image("textures/brickwork-texture.jpg");
	uint32_t mantle_img_size = mantle_img.height * mantle_img.width * 4;
	unsigned char* mantle_data = (unsigned char*)malloc(mantle_img_size);
	memcpy(mantle_data, mantle_img.data, mantle_img_size);

	VkalTexture mantle_texture = vkal_create_texture(
		4,
		mantle_data,
		mantle_img.width, mantle_img.height, mantle_img.channels,
		0,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_FORMAT_R8G8B8A8_UNORM,
		0, 1,
		0, 1,
		VK_FILTER_LINEAR, VK_FILTER_LINEAR,
		VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	vkal_update_descriptor_set_texture(descriptor_set_1[0], mantle_texture);


	// Model and Projection Matrices
	glm::mat4 model = glm::translate(glm::mat4(1), glm::vec3(0, 0, 0));
	glm::mat4 projection = adjust_y_for_vulkan_ndc * glm::perspective(glm::radians(45.f), 1.f, 0.1f, 1000.0f);

	// Build cylinder geometry
	Model cylinderModel;
	setup_geometry(cylinderModel);

	// Camera
	Camera camera = Camera(glm::vec3(0, 0, 5));

	// User Interface 
	double timer_frequency = glfwGetTimerFrequency();
	double timestep = 1.f / timer_frequency; // step in second
	double title_update_timer = 0;
	double dt = 0;

	int width  = 800;
	int height = 600;
	// MAIN LOOP
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
	float old_angle_threshold = angle_threshold;
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
			ImGui::Begin("Versuch 3 - Normalenvektoren");
			ImGui::SliderFloat("Threshold", &angle_threshold, 0, 180.0f);
			ImGui::Checkbox("Draw Normals", &draw_normals);
			ImGui::ColorEdit3("Light Ambient", (float*)&settings_data.light_ambient);
			ImGui::ColorEdit3("Light Diffuse", (float*)&settings_data.light_diffuse);
			ImGui::ColorEdit3("Light Specular", (float*)&settings_data.light_specular);
			ImGui::ColorEdit3("Material Emissive", (float*)&g_model.material.emissive);
			ImGui::ColorEdit3("Material Ambient", (float*)&g_model.material.ambient);
			ImGui::ColorEdit3("Material Diffuse", (float*)&g_model.material.diffuse);
			ImGui::ColorEdit3("Material Specular", (float*)&g_model.material.specular);
			ImGui::SliderFloat("Shininess", &g_model.material.specular.w, 1.0f, 128.0f);
			ImGui::SliderFloat3("Light Position", (float*)&settings_data.light_pos[0], -100.0, 100.0);
			ImGui::Checkbox("Phong", &phong);
			ImGui::End();
		}

		/* Update Material and Model UBOs */
		material_data.emissive = g_model.material.emissive;
		material_data.ambient = g_model.material.ambient;
		material_data.diffuse = g_model.material.diffuse;
		material_data.specular = g_model.material.specular;
		vkal_update_uniform(&uniform_buffer_material, &material_data);
		update_modelMatrix(g_model.model_matrix, float(dt * 100.f));
		model_data.model_mat = g_model.model_matrix;
		vkal_update_uniform(&uniform_buffer_model, &model_data);

		update_camera(camera, float(dt*1000.f));


		// Mouse update
		if (!ImGui::IsMouseHoveringRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax()) && !ImGui::IsAnyItemActive()) {
			update_camera_mouse_look(camera, dt);
		}

		// Angle Threshold has changed -> update vertex buffers of the models
		/*
		if (old_angle_threshold != angle_threshold) {
			update_geometry(cylinderModel);
			vkal_vertex_buffer_update(g_model_normals.vertices, g_model_normals.vertex_count, sizeof(Vertex), g_model_normals.offset);
			vkal_vertex_buffer_update(g_model.vertices, g_model.vertex_count, sizeof(Vertex), g_model.offset);
			old_angle_threshold = angle_threshold;
		}*/

		view_projection_data.projection = adjust_y_for_vulkan_ndc * glm::perspective(glm::radians(45.f), float(width) / float(height), 0.1f, 1000.f);
		view_projection_data.view = glm::lookAt(camera.m_Pos, camera.m_Center, camera.m_Up);
		vkal_update_uniform(&uniform_buffer_handle, &view_projection_data);
		vkal_update_uniform(&uniform_buffer_handle2, &settings_data);

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
			if (!phong) {
				vkal_draw(image_id, pipeline_filled_polys_gouraud, g_model.offset, g_model.vertex_count);
			}
			else {
				vkal_draw(image_id, pipeline_filled_polys_phong, g_model.offset, g_model.vertex_count);
			}
			if (draw_normals) {
				vkal_draw(image_id, pipeline_lines, g_model_normals.offset, g_model_normals.vertex_count);
			}

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
			sprintf(window_title, "frametime: %fms (%f FPS)", (dt * 1000.f), (1000.f)/(dt*1000.f));
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

