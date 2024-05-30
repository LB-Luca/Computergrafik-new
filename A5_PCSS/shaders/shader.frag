#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_EXT_nonuniform_qualifier    : enable

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 1) uniform UBO
{
	mat4 view;
	mat4 projection;
	mat4 light_view_mat;
	mat4 light_proj_mat;

	vec4 light_pos;
	vec2 light_size;
	float light_camera_near;
	float light_camera_far;
	uint filter_method;
	uint poisson_disk_size;
	uint  use_textures;
	float light_radius_bias;

	vec3 model_pos;

} ubo;


layout (set = 0, binding = 0) uniform Material_t
{
	vec4 emissive;
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	uint texture_id;
	uint is_textured;
	uint normal_id;
	uint has_normal_map;
} Material;

layout(set = 0, binding = 2) uniform sampler2D textures[];

layout(set = 0, binding = 3) uniform sampler2D shadow_sampler;

layout (location = 1) in vec4 in_world_pos;
layout (location = 2) in vec4 in_light_pos;
layout (location = 3) in vec4 in_viewspace_normal;
layout (location = 4) in vec2 in_uv;
layout (location = 5) in vec4 in_color;



#define c_ambient 0.0
#define BIAS      0.00001
#define PI        3.14159
#define Z_NEAR    0.1
#define Z_FAR     1000.0

const mat4 bias_mat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );


const vec2 Poisson25[25] = vec2[](
    vec2(-0.978698, -0.0884121),
    vec2(-0.841121, 0.521165),
    vec2(-0.71746, -0.50322),
    vec2(-0.702933, 0.903134),
    vec2(-0.663198, 0.15482),
    vec2(-0.495102, -0.232887),
    vec2(-0.364238, -0.961791),
    vec2(-0.345866, -0.564379),
    vec2(-0.325663, 0.64037),
    vec2(-0.182714, 0.321329),
    vec2(-0.142613, -0.0227363),
    vec2(-0.0564287, -0.36729),
    vec2(-0.0185858, 0.918882),
    vec2(0.0381787, -0.728996),
    vec2(0.16599, 0.093112),
    vec2(0.253639, 0.719535),
    vec2(0.369549, -0.655019),
    vec2(0.423627, 0.429975),
    vec2(0.530747, -0.364971),
    vec2(0.566027, -0.940489),
    vec2(0.639332, 0.0284127),
    vec2(0.652089, 0.669668),
    vec2(0.773797, 0.345012),
    vec2(0.968871, 0.840449),
    vec2(0.991882, -0.657338)
);

const vec2 Poisson32[32] = vec2[](
    vec2(-0.975402, -0.0711386),
    vec2(-0.920347, -0.41142),
    vec2(-0.883908, 0.217872),
    vec2(-0.884518, 0.568041),
    vec2(-0.811945, 0.90521),
    vec2(-0.792474, -0.779962),
    vec2(-0.614856, 0.386578),
    vec2(-0.580859, -0.208777),
    vec2(-0.53795, 0.716666),
    vec2(-0.515427, 0.0899991),
    vec2(-0.454634, -0.707938),
    vec2(-0.420942, 0.991272),
    vec2(-0.261147, 0.588488),
    vec2(-0.211219, 0.114841),
    vec2(-0.146336, -0.259194),
    vec2(-0.139439, -0.888668),
    vec2(0.0116886, 0.326395),
    vec2(0.0380566, 0.625477),
    vec2(0.0625935, -0.50853),
    vec2(0.125584, 0.0469069),
    vec2(0.169469, -0.997253),
    vec2(0.320597, 0.291055),
    vec2(0.359172, -0.633717),
    vec2(0.435713, -0.250832),
    vec2(0.507797, -0.916562),
    vec2(0.545763, 0.730216),
    vec2(0.56859, 0.11655),
    vec2(0.743156, -0.505173),
    vec2(0.736442, -0.189734),
    vec2(0.843562, 0.357036),
    vec2(0.865413, 0.763726),
    vec2(0.872005, -0.927)
);

const vec2 Poisson64[64] = vec2[](
    vec2(-0.934812, 0.366741),
    vec2(-0.918943, -0.0941496),
    vec2(-0.873226, 0.62389),
    vec2(-0.8352, 0.937803),
    vec2(-0.822138, -0.281655),
    vec2(-0.812983, 0.10416),
    vec2(-0.786126, -0.767632),
    vec2(-0.739494, -0.535813),
    vec2(-0.681692, 0.284707),
    vec2(-0.61742, -0.234535),
    vec2(-0.601184, 0.562426),
    vec2(-0.607105, 0.847591),
    vec2(-0.581835, -0.00485244),
    vec2(-0.554247, -0.771111),
    vec2(-0.483383, -0.976928),
    vec2(-0.476669, -0.395672),
    vec2(-0.439802, 0.362407),
    vec2(-0.409772, -0.175695),
    vec2(-0.367534, 0.102451),
    vec2(-0.35313, 0.58153),
    vec2(-0.341594, -0.737541),
    vec2(-0.275979, 0.981567),
    vec2(-0.230811, 0.305094),
    vec2(-0.221656, 0.751152),
    vec2(-0.214393, -0.0592364),
    vec2(-0.204932, -0.483566),
    vec2(-0.183569, -0.266274),
    vec2(-0.123936, -0.754448),
    vec2(-0.0859096, 0.118625),
    vec2(-0.0610675, 0.460555),
    vec2(-0.0234687, -0.962523),
    vec2(-0.00485244, -0.373394),
    vec2(0.0213324, 0.760247),
    vec2(0.0359813, -0.0834071),
    vec2(0.0877407, -0.730766),
    vec2(0.14597, 0.281045),
    vec2(0.18186, -0.529649),
    vec2(0.188208, -0.289529),
    vec2(0.212928, 0.063509),
    vec2(0.23661, 0.566027),
    vec2(0.266579, 0.867061),
    vec2(0.320597, -0.883358),
    vec2(0.353557, 0.322733),
    vec2(0.404157, -0.651479),
    vec2(0.410443, -0.413068),
    vec2(0.413556, 0.123325),
    vec2(0.46556, -0.176183),
    vec2(0.49266, 0.55388),
    vec2(0.506333, 0.876888),
    vec2(0.535875, -0.885556),
    vec2(0.615894, 0.0703452),
    vec2(0.637135, -0.637623),
    vec2(0.677236, -0.174291),
    vec2(0.67626, 0.7116),
    vec2(0.686331, -0.389935),
    vec2(0.691031, 0.330729),
    vec2(0.715629, 0.999939),
    vec2(0.8493, -0.0485549),
    vec2(0.863582, -0.85229),
    vec2(0.890622, 0.850581),
    vec2(0.898068, 0.633778),
    vec2(0.92053, -0.355693),
    vec2(0.933348, -0.62981),
    vec2(0.95294, 0.156896)
);




vec2 g_dz_duv;

/* Nvidia's code! */
// Derivatives of light-space depth with respect to texture2D coordinates
vec2 depthGradient(vec2 uv, float z)
{
    vec2 dz_duv = vec2(0.0, 0.0);

    vec3 duvdist_dx = dFdx(vec3(uv,z));
    vec3 duvdist_dy = dFdy(vec3(uv,z));

    dz_duv.x = duvdist_dy.y * duvdist_dx.z;
    dz_duv.x -= duvdist_dx.y * duvdist_dy.z;

    dz_duv.y = duvdist_dx.x * duvdist_dy.z;
    dz_duv.y -= duvdist_dy.x * duvdist_dx.z;

    float det = (duvdist_dx.x * duvdist_dy.y) - (duvdist_dx.y * duvdist_dy.x);
    dz_duv /= det;

    return dz_duv;
}

float biasedZ(float z0, vec2 dz_duv, vec2 offset)
{
    return z0 + dot(dz_duv, offset);
}

float pcf(vec3 shadowCoord)
{
	float smDepth = texture(shadow_sampler, shadowCoord.xy).x;
	if(shadowCoord.z < smDepth + BIAS){
		return 1.0f;
	}
	return 0.0f;
}

float pcss(vec3 shadowCoord) {
	float zNear = ubo.light_camera_near;
	float zFar = ubo.light_camera_far;

	/* STEP 1: blocker search */

	/* Z from the light's point of view */
    
	/* Light-Radius-Bias necessary since we provide the light size within texture coordinates which is mostly <0! Also
	   the bias enables to tweak the blocker  search a bit and can be useful depending on the distance of the light
	   to the receiver. 
	*/

	switch (ubo.poisson_disk_size) {
		case 0:
		{
			float smDepth = texture(shadow_sampler, shadowCoord.xy).x;
			if(shadowCoord.z < smDepth){
				return 1.0f;
			}
			return 0.0f;
			break;
		} 
		case 1:
		{
			float smDepth = texture(shadow_sampler, shadowCoord.xy).x;
			if(shadowCoord.z < smDepth){
				return 1.0f;
			}
			return 0.0f;
			break;
		} 
		case 2:
		{
			float smDepth = texture(shadow_sampler, shadowCoord.xy).x;
			if(shadowCoord.z < smDepth){
				return 1.0f;
			}
			return 0.0f;
			break;
		}
	}
	
	/* No Blockers found: Fragment is fully lit -> early out. */

	/* STEP 2: Approximate penumbra size.
	   We found the blocker depths in NDC, that is a value from 0 to 1. We have to project those blocker distances back
	   to the light's viewspace to do further computations.
	*/

	/* Use similar triangles to compute penumbra_size. */

	/* STEP 3: filtering */
	//return pcf(shadowCoord, pcf_kernel_size);
}

void main() 
{
	/* Convert this fragments position to Light's NDC which is suitable for texture sampling.
	   The implicit perspective division is only performed if we use gl_Position in the
	   Vertex Shader to pass vertices on to the next stage!
	*/
	vec4 light_pos_uvz = in_light_pos / in_light_pos.w;

	/* depth gradient for dynamic z-biasing */
	g_dz_duv = depthGradient(light_pos_uvz.xy, light_pos_uvz.z);

	float shadow = 1.00;
	if (ubo.filter_method == 0) {
		shadow = pcf(light_pos_uvz.xyz);
	}
	else if (ubo.filter_method == 1) {
		shadow = pcss(light_pos_uvz.xyz);
	}
	else { /* no shadows */
	}


	/* Do a basic diffuse shading */
	vec4 light_pos_viewspace = ubo.view * ubo.light_pos;
	vec4 pos_viewspace = ubo.view * in_world_pos;
	vec3 light_dir = normalize(light_pos_viewspace.xyz - pos_viewspace.xyz);
	vec3 normal = normalize(in_viewspace_normal.xyz);
	float diffuse_strength = max(dot(light_dir, normal), 0.0);
	vec4 diffuse_texture = texture(textures[Material.texture_id], in_uv);
	vec3 diffuse_color;
	if (ubo.use_textures == 1) 	diffuse_color = diffuse_strength * diffuse_texture.xyz;
	else 				diffuse_color = diffuse_strength * Material.diffuse.xyz;

	// Fragments in umbra will be 80% darkend
	const float dim = 0.8;
	vec3 final_color;
	final_color = Material.emissive.xyz + diffuse_color * (1.0 - dim * (1.0 - shadow));

	outColor = vec4(final_color, 1.0);
}
