#ifndef MODEL_H
#define MODEL_H

#include <vkal/vkal.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

//#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

#include <vkal/src/examples/utils/platform.h>

struct Vertex /* Packed as 20 floats */
{
	glm::vec3 pos;
	glm::vec2 uv;
	glm::vec3 normal;
	//glm::vec3 barycentric;
	glm::vec4 color;
	glm::vec3 tangent;
	float     dummy; /* round up to 16 floats. Easier to access vertex data in shader through indexing. */

	uint16_t  id;    // Holds its own index
	uint16_t  id_h;
	glm::vec3 dummy2; /* round up to 20 floats. Same reason as above. */
};

struct Tri
{
	union {
		struct {
			uint16_t a;
			uint16_t b;
			uint16_t c;
		};
		uint16_t indices[3];
	};
};

struct Material
{
	glm::vec4 emissive;
	glm::vec4 ambient;
	glm::vec4 diffuse;
	glm::vec4 specular;
	uint32_t  texture_id; /* index into texture-array in shader. Extension GL_EXT_nonuniform_qualifier required! */
	uint32_t  is_textured;
	uint32_t  normal_id;
	uint32_t  has_normal_map;
};

struct Image
{
	uint32_t width, height, channels;
	unsigned char * data;
};

enum TextureType
{
	TEXTURE_TYPE_DIFFUSE,
	TEXTURE_TYPE_NORMAL,
	MAX_TEXTURE_TYPES
};

struct Texture
{
	uint32_t  device_memory_id;
	VkSampler sampler;
	uint32_t  image;
	uint32_t  image_view;
	uint32_t  width;
	uint32_t  height;
	uint32_t  channels;
	uint32_t  binding;
	char      texture_file[64];
};

struct BoundingBox
{
	glm::vec4 min_xyz;
	glm::vec4 max_xyz;
};

struct Model
{
	Vertex *    vertices;
	uint32_t    vertex_count;
	uint64_t	offset;
	uint32_t    binding;
	std::vector<Tri> tris; // Reference to vertices by index
	uint32_t    face_count;
	glm::vec3   pos;
	glm::mat4   model_matrix;
	Material    material; // TODO: Materials should be references as multiple models may share the same material!
	uint32_t    material_id; 
	VkalTexture texture;
	VkalTexture normal_map;
	BoundingBox bounding_box;	

	uint16_t * indices;
	uint32_t index_count;
	uint32_t index_buffer_offset;

	uint32_t* indices_32;
};


Model create_model_from_file(char const* file);
Model create_model_from_file_indexed(char const* file);
Model create_model_from_file_indexed_rt(char const* file);
void  assign_texture_to_model(Model* model, VkalTexture texture, uint32_t id, TextureType texture_type);
std::vector<Tri> get_adjacent_triangles(Model* model, Vertex v);

#endif