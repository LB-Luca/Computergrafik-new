#include "model.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tinyobjloader/tiny_obj_loader.h>


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include <vkal/src/examples/utils/platform.h>

Model create_model_from_file(char const* file)
{
	Model model = {};
	
	char exe_path[256];
	get_exe_path(exe_path, 256);

	std::string abs_path = concat_paths(std::string(exe_path), get_assets_dir());
	abs_path = concat_paths(abs_path, std::string(file));

	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "./";

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(abs_path, reader_config)) {
		if (!reader.Error().empty()) {
			printf("[TinyObjReader]: %s\n", reader.Error().c_str());
			getchar();
		}
		exit(-1);
	}

	if (!reader.Warning().empty()) {
		printf("[TinyObjReader]: %s\n", reader.Warning().c_str());
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	for (size_t s = 0; s < shapes.size(); s++) {
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			model.vertex_count += shapes[s].mesh.num_face_vertices[f];
		}
	}
	model.vertices = (Vertex*)malloc(model.vertex_count * sizeof(Vertex));
	memset(model.vertices, 0, model.vertex_count * sizeof(Vertex));

	// NOTE: We still want the indices, even if they are a one to one mapping to the vertices.
	//       It sometimes is convenient to have these (see Raytracing).
	model.index_count = model.vertex_count;
	model.indices = (uint16_t*)malloc(model.index_count * sizeof(uint16_t));
	memset(model.indices, 0, model.index_count * sizeof(uint16_t));

	// Loop over shapes
	size_t vert_count = 0;
	tinyobj::real_t minX =  99999;
	tinyobj::real_t maxX = -99999;
	tinyobj::real_t minY =  99999;
	tinyobj::real_t maxY = -99999;
	tinyobj::real_t minZ =  99999;
	tinyobj::real_t maxZ = -99999;	

	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			Tri tri{};

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
				model.vertices[vert_count + 0].pos.x = vx;
				model.vertices[vert_count + 0].pos.y = vy;
				model.vertices[vert_count + 0].pos.z = vz;
				
				model.vertices[vert_count].id = size_t(idx.vertex_index);
				model.indices[vert_count] = vert_count;

				/*if (idx.vertex_index >= 0) {
				}				*/

				// Check for min/max x,y,z for AABB
				if (vx < minX)			minX = vx;
				else if (vx >= maxX)	maxX = vx;
				if (vy < minY)			minY = vy;
				else if (vy >= maxY)	maxY = vy;
				if (vz < minZ)			minZ = vz;
				else if (vz >= maxZ)	maxZ = vz;

				// Check if `normal_index` is zero or positive. negative = no normal data
				if (idx.normal_index >= 0) {
					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
					model.vertices[vert_count + 0].normal.x = nx;
					model.vertices[vert_count + 0].normal.y = ny;
					model.vertices[vert_count + 0].normal.z = nz;
				}

				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (idx.texcoord_index >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
					model.vertices[vert_count + 0].uv.x = tx;
					model.vertices[vert_count + 0].uv.y = 1.0f - ty;
				}

				// Optional: vertex colors
				// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
				// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
				// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

				tri.indices[v] = vert_count;

				vert_count++;
			}
			model.tris.push_back(tri);
			index_offset += fv;

			// per-face material
			//shapes[s].mesh.material_ids[f];
		}
	}

	model.bounding_box.min_xyz = glm::vec4(minX, minY, minZ, 1.0f);
	model.bounding_box.max_xyz = glm::vec4(maxX, maxY, maxZ, 1.0f);

	return model;
}

Model create_model_from_file_indexed(char const* file)
{
	Model model = {};

	char exe_path[256];
	get_exe_path(exe_path, 256);

	std::string abs_path = concat_paths(std::string(exe_path), get_assets_dir());
	abs_path = concat_paths(abs_path, std::string(file));

	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "./";

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(abs_path, reader_config)) {
		if (!reader.Error().empty()) {
			printf("[TinyObjReader]: %s\n", reader.Error().c_str());
			getchar();
		}
		exit(-1);
	}

	if (!reader.Warning().empty()) {
		printf("[TinyObjReader]: %s\n", reader.Warning().c_str());
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	for (size_t s = 0; s < shapes.size(); s++) {
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			model.vertex_count += shapes[s].mesh.num_face_vertices[f];
		}
	}
	model.vertices = (Vertex*)malloc(model.vertex_count * sizeof(Vertex));
	memset(model.vertices, 0, model.vertex_count * sizeof(Vertex));

	// NOTE: We still want the indices, even if they are a one to one mapping to the vertices.
	//       It sometimes is convenient to have these (see Raytracing).
	model.index_count = model.vertex_count;
	model.indices = (uint16_t*)malloc(model.index_count * sizeof(uint16_t));
	memset(model.indices, 0, model.index_count * sizeof(uint16_t));

	// Loop over shapes
	size_t vert_count = 0;
	tinyobj::real_t minX = 99999;
	tinyobj::real_t maxX = -99999;
	tinyobj::real_t minY = 99999;
	tinyobj::real_t maxY = -99999;
	tinyobj::real_t minZ = 99999;
	tinyobj::real_t maxZ = -99999;

	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			Tri tri{};

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
				int vert_index = idx.vertex_index;

				model.vertices[vert_index].pos.x = vx;
				model.vertices[vert_index].pos.y = vy;
				model.vertices[vert_index].pos.z = vz;

				model.vertices[vert_index].id = (uint16_t)vert_index;
				model.indices[vert_count] = (uint16_t)vert_index;				

				// Check for min/max x,y,z for AABB
				if (vx < minX)			minX = vx;
				else if (vx >= maxX)	maxX = vx;
				if (vy < minY)			minY = vy;
				else if (vy >= maxY)	maxY = vy;
				if (vz < minZ)			minZ = vz;
				else if (vz >= maxZ)	maxZ = vz;

				int normal_index = idx.normal_index;
				// Check if `normal_index` is zero or positive. negative = no normal data
				if (normal_index >= 0) {
					tinyobj::real_t nx = attrib.normals[3 * size_t(normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(normal_index) + 2];
					model.vertices[vert_index].normal.x = nx;
					model.vertices[vert_index].normal.y = ny;
					model.vertices[vert_index].normal.z = nz;
				}

				int texcoord_index = idx.texcoord_index;
				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (texcoord_index >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(texcoord_index) + 1];
					model.vertices[vert_index].uv.x = tx;
					model.vertices[vert_index].uv.y = 1.0f - ty;
				}

				// Optional: vertex colors
				// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
				// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
				// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

				tri.indices[v] = (uint16_t)vert_index;

				vert_count++;
			}
			model.tris.push_back(tri);
			index_offset += fv;

			// per-face material
			//shapes[s].mesh.material_ids[f];
		}
	}

	model.bounding_box.min_xyz = glm::vec4(minX, minY, minZ, 1.0f);
	model.bounding_box.max_xyz = glm::vec4(maxX, maxY, maxZ, 1.0f);

	return model;
}

Model create_model_from_file_indexed_rt(char const* file)
{		
	Model model = {};

	char exe_path[256];
	get_exe_path(exe_path, 256);

	std::string abs_path = concat_paths(std::string(exe_path), get_assets_dir());
	abs_path = concat_paths(abs_path, std::string(file));

	tinyobj::ObjReaderConfig reader_config;
	reader_config.mtl_search_path = "./";

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(abs_path, reader_config)) {
		if (!reader.Error().empty()) {
			printf("[TinyObjReader]: %s\n", reader.Error().c_str());
			getchar();
		}
		exit(-1);
	}

	if (!reader.Warning().empty()) {
		printf("[TinyObjReader]: %s\n", reader.Warning().c_str());
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& materials = reader.GetMaterials();

	for (size_t s = 0; s < shapes.size(); s++) {
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			model.vertex_count += shapes[s].mesh.num_face_vertices[f];
		}
	}
	model.vertices = (Vertex*)malloc(model.vertex_count * sizeof(Vertex));
	memset(model.vertices, 0, model.vertex_count * sizeof(Vertex));

	// NOTE: We still want the indices, even if they are a one to one mapping to the vertices.
	//       It sometimes is convenient to have these (see Raytracing).
	model.index_count = model.vertex_count;
	model.indices_32 = (uint32_t*)malloc(model.index_count * sizeof(uint32_t));
	memset(model.indices_32, 0, model.index_count * sizeof(uint32_t));

	// Loop over shapes
	size_t vert_count = 0;
	tinyobj::real_t minX = 99999;
	tinyobj::real_t maxX = -99999;
	tinyobj::real_t minY = 99999;
	tinyobj::real_t maxY = -99999;
	tinyobj::real_t minZ = 99999;
	tinyobj::real_t maxZ = -99999;

	for (size_t s = 0; s < shapes.size(); s++) {
		// Loop over faces(polygon)
		size_t index_offset = 0;
		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
			Tri tri{};

			// Loop over vertices in the face.
			for (size_t v = 0; v < fv; v++) {
				// access to vertex
				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
				int vert_index = idx.vertex_index;

				model.vertices[vert_count].pos.x = vx;
				model.vertices[vert_count].pos.y = vy;
				model.vertices[vert_count].pos.z = vz;

				model.vertices[vert_count].id   = 0x0000FFFF & (uint32_t)vert_index;
				model.vertices[vert_count].id_h = 0xFFFF0000 & (uint32_t)vert_index;
				model.indices_32[vert_count] = (uint32_t)vert_index;

				// Check for min/max x,y,z for AABB
				if (vx < minX)			minX = vx;
				else if (vx >= maxX)	maxX = vx;
				if (vy < minY)			minY = vy;
				else if (vy >= maxY)	maxY = vy;
				if (vz < minZ)			minZ = vz;
				else if (vz >= maxZ)	maxZ = vz;

				int normal_index = idx.normal_index;
				// Check if `normal_index` is zero or positive. negative = no normal data
				if (normal_index >= 0) {
					tinyobj::real_t nx = attrib.normals[3 * size_t(normal_index) + 0];
					tinyobj::real_t ny = attrib.normals[3 * size_t(normal_index) + 1];
					tinyobj::real_t nz = attrib.normals[3 * size_t(normal_index) + 2];
					model.vertices[vert_index].normal.x = nx;
					model.vertices[vert_index].normal.y = ny;
					model.vertices[vert_index].normal.z = nz;
				}

				int texcoord_index = idx.texcoord_index;
				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
				if (texcoord_index >= 0) {
					tinyobj::real_t tx = attrib.texcoords[2 * size_t(texcoord_index) + 0];
					tinyobj::real_t ty = attrib.texcoords[2 * size_t(texcoord_index) + 1];
					model.vertices[vert_index].uv.x = tx;
					model.vertices[vert_index].uv.y = 1.0f - ty;
				}

				// Optional: vertex colors
				// tinyobj::real_t red   = attrib.colors[3*size_t(idx.vertex_index)+0];
				// tinyobj::real_t green = attrib.colors[3*size_t(idx.vertex_index)+1];
				// tinyobj::real_t blue  = attrib.colors[3*size_t(idx.vertex_index)+2];

				tri.indices[v] = (uint16_t)vert_index; // TODO: How to store u32 Tris?

				vert_count++;
			}
			model.tris.push_back(tri);
			index_offset += fv;

			// per-face material
			//shapes[s].mesh.material_ids[f];
		}
	}

	model.bounding_box.min_xyz = glm::vec4(minX, minY, minZ, 1.0f);
	model.bounding_box.max_xyz = glm::vec4(maxX, maxY, maxZ, 1.0f);

	return model;
}

void assign_texture_to_model(Model * model, VkalTexture texture, uint32_t id, TextureType texture_type)
{
	assert( (texture_type < MAX_TEXTURE_TYPES) && "Unknown TextureType!" );
	if (texture_type == TEXTURE_TYPE_DIFFUSE) {
		model->material.texture_id = id;
		model->texture = texture;
		model->material.is_textured = 1;
	}
	else if (texture_type == TEXTURE_TYPE_NORMAL) {
		model->material.normal_id = id;
		model->normal_map = texture;
		model->material.has_normal_map = 1;
	}
}

std::vector<Tri> get_adjacent_triangles(Model* model, Vertex v)
{
	std::vector<Tri> result{};

	for (Tri& tri : model->tris) {

		Vertex v1 = model->vertices[tri.a];
		Vertex v2 = model->vertices[tri.b];
		Vertex v3 = model->vertices[tri.c];

		if (v1.id == v.id || v2.id == v.id || v3.id == v.id) {
			result.push_back({ tri.a, tri.b, tri.c });
		}
	}

	return result;
}
