/**
 * @file		engine_pipeline.cpp
 * @brief	Simple forward-rendering pipeline
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */


//////////////
// #INCLUDE //
//////////////

// Main include:
#include "engine.h"

// OGL:      
#include <GL/glew.h>
#include <GLFW/glfw3.h>


/////////////
// SHADERS //
/////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline vertex shader.
 */
static const std::string pipeline_vs = R"(
#version 460 core
 
// Per-vertex data from VBOs:
layout(location = 0) in vec3 a_vertex;
layout(location = 1) in vec4 a_normal;
layout(location = 2) in vec2 a_uv;
layout(location = 3) in vec4 a_tangent;

// Uniforms:
uniform mat4 modelviewMat;
uniform mat4 projectionMat;
uniform mat3 normalMat;

// Varying:
out vec4 fragPosition;
out vec3 normal;
out vec2 uv;
out mat3 tbn;

void main()
{
   normal = normalMat * a_normal.xyz;

   vec3 tangent = normalMat * a_tangent.xyz;
   tangent = normalize(tangent - dot(tangent, normal) * normal);

   vec3 bitangent = normalize(cross(normal, tangent));

   //tbn = transpose(mat3(tangent, bitangent, normal));
   tbn = mat3(tangent, bitangent, normal);

   uv = a_uv;

   fragPosition = modelviewMat * vec4(a_vertex, 1.0f);
   gl_Position = projectionMat * fragPosition;

})";


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline fragment shader.
 */
const std::string pipeline_fs = R"(
#version 460 core
#extension GL_ARB_bindless_texture : require

const float PI = 3.14159265359;

// Uniform (textures):
layout (bindless_sampler) uniform sampler2D texture0; // Albedo
layout (bindless_sampler) uniform sampler2D texture1; // Normal
layout (bindless_sampler) uniform sampler2D texture2; // Roughness
layout (bindless_sampler) uniform sampler2D texture3; // Metalness

// Uniform (material):
uniform vec3 mtlEmission;
uniform vec3 mtlAlbedo;
uniform float mtlOpacity;
uniform float mtlRoughness;
uniform float mtlMetalness;

// Uniform (light):
uniform vec3 lightColor;
uniform vec3 lightPosition;

// Varying:
in vec4 fragPosition;
in vec3 normal;
in vec2 uv;
in mat3 tbn;

// Output to the framebuffer:
out vec4 outFragment;

vec3 F0(vec3 dielectric, vec3 albedo, float metalness)
{

   return mix(dielectric, albedo, metalness);

}

float D_GGX(vec3 N, vec3 H, float alpha)
{

   float alpha_2 = alpha * alpha;

   float cosNH   = max(0.0f, dot(N, H));
   //float cosNH   = dot(N, H);

   float cosNH_2 = cosNH * cosNH;

   float num     = alpha_2;
   float denom   = PI *  pow(cosNH_2 * (alpha_2 - 1.0f) + 1.0f, 2.0f);

   return num / denom;

}

vec3 F_schlick(vec3 f0, vec3 H, vec3 V)
{

   float cosHV = max(0.0f, dot(H, V));
   //float cosHV = dot(H, V);

   return f0 + (1.0f - f0) * pow(1.0 - cosHV, 5.0f); 

}

float G_schlickGGX(vec3 N, vec3 V, float alpha)
{

   //float cosNV = max(0.0f, dot(N, V));
   float cosNV = dot(N, V);
   float k     = pow(alpha + 1.0f, 2.0f) / 8.0f;

   float num   = cosNV;
   float denom = cosNV * (1.0f - k) + k;

   return num / denom;

}

vec3 lambert(vec3 albedo)
{

  return albedo / PI;

}

vec3 cook_torrance(vec3 N, vec3 L, vec3 V, vec3 H, vec3 albedo, float alpha, float metal)
{
   // Fresnel base reflectivity at 0 deg incidence
   vec3 fb = F0(vec3(0.04f), albedo, metal);

   float D = D_GGX(N, H, alpha);
   vec3  F = F_schlick(fb, H, V);
   float G = G_schlickGGX(N, V, alpha);

   float cosVN = dot(V, N);
   float cosLN = dot(L, N);

   vec3 num    = D * F * G;
   float denom = 4 * cosVN * cosLN;

   return num / denom;
   
}

void main()
{
   // Texture lookup:
   vec4 albedo_texel = texture(texture0, uv);
   vec4 normal_texel = texture(texture1, uv);
   vec4 roughness_texel = texture(texture2, uv);
   vec4 metalness_texel = texture(texture3, uv);
   float justUseIt = albedo_texel.r + normal_texel.r + roughness_texel.r + metalness_texel.r + mtlAlbedo.x + mtlRoughness.x + mtlEmission.x + mtlMetalness.x + mtlOpacity.x;

   // Calculate z parameter and normalize into [-1,1]
   vec3 normal3d = normal_texel.xyz;
   normal3d.z = sqrt(1.0 - pow(normal3d.x, 2.0) - pow(normal3d.y, 2.0));
   normal3d = normalize(normal3d * 2.0 - 1.0);
   
   vec3 N = tbn * normalize(normal3d);   
   vec3 V = normalize(-fragPosition.xyz);  
   vec3 L = normalize(lightPosition - fragPosition.xyz);

   // Half vector between view vector and light vector
   vec3 H = normalize(V + L);

// PBR //

   // Lambert
   vec3 fLB = lambert(albedo_texel.xyz);

   // Cook-torrance
   vec3 fCT = cook_torrance(N, L, V, H, albedo_texel.xyz, roughness_texel.r, metalness_texel.r);

   // Reflection and Refraction Coefficients
   vec3 fb = F0(vec3(0.04f), albedo_texel.xyz, metalness_texel.r);

   vec3 ks = F_schlick(fb, H, V);
   vec3 kd = (vec3(1.0f) - ks) * (1 - metalness_texel.r);

   // Final result

   //float cosNLdir = max(0.0f, dot(N, lightDirection));

   vec3 fr = kd * fLB + ks * fCT;

// PBR //

   outFragment = vec4(fr * lightColor.xyz, justUseIt);

})";

/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief PipelineDefault reserved structure.
 */
struct Eng::PipelineDefault::Reserved
{
	Eng::Shader vs;
	Eng::Shader fs;
	Eng::Program program;

	bool wireframe;


	/**
	 * Constructor. 
	 */
	Reserved() : wireframe{false} {}
};


///////////////////////////////////
// BODY OF CLASS PipelineDefault //
///////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::PipelineDefault::PipelineDefault() : reserved(std::make_unique<Eng::PipelineDefault::Reserved>())
{
	ENG_LOG_DETAIL("[+]");
	this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name node name
 */
ENG_API Eng::PipelineDefault::PipelineDefault(const std::string& name) : Eng::Pipeline(name),
                                                                         reserved(
	                                                                         std::make_unique<
		                                                                         Eng::PipelineDefault::Reserved>())
{
	ENG_LOG_DETAIL("[+]");
	this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor. 
 */
ENG_API Eng::PipelineDefault::PipelineDefault(PipelineDefault&& other) : Eng::Pipeline(std::move(other)),
                                                                         reserved(std::move(other.reserved))
{
	ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::PipelineDefault::~PipelineDefault()
{
	ENG_LOG_DETAIL("[-]");
	if (this->isInitialized())
		free();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Initializes this pipeline. 
 * @return TF
 */
bool ENG_API Eng::PipelineDefault::init()
{
	// Already initialized?
	if (this->Eng::Managed::init() == false)
		return false;
	if (!this->isDirty())
		return false;

	// Build:
	reserved->vs.load(Eng::Shader::Type::vertex, pipeline_vs);
	reserved->fs.load(Eng::Shader::Type::fragment, pipeline_fs);
	if (reserved->program.build({reserved->vs, reserved->fs}) == false)
	{
		ENG_LOG_ERROR("Unable to build default program");
		return false;
	}
	this->setProgram(reserved->program);

	// Done: 
	this->setDirty(false);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Releases this pipeline.
 * @return TF
 */
bool ENG_API Eng::PipelineDefault::free()
{
	if (this->Eng::Managed::free() == false)
		return false;

	// Done:   
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the status of the wireframe status.
 * @return wireframe status
 */
bool ENG_API Eng::PipelineDefault::isWireframe() const
{
	return reserved->wireframe;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the status of the wireframe flag.
 * @param flag wireframe flag
 */
void ENG_API Eng::PipelineDefault::setWireframe(bool flag)
{
	reserved->wireframe = flag;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Main rendering method for the pipeline.  
 * @param list list of renderables
 * @return TF
 */
bool ENG_API Eng::PipelineDefault::render(const glm::mat4& camera, const glm::mat4& proj, const Eng::List& list)
{
	// Safety net:
	if (list == Eng::List::empty)
	{
		ENG_LOG_ERROR("Invalid params");
		return false;
	}

	// Lazy-loading:
	if (this->isDirty())
		if (!this->init())
		{
			ENG_LOG_ERROR("Unable to render (initalization failed)");
			return false;
		}

	// Apply program:
	Eng::Program& program = getProgram();
	if (program == Eng::Program::empty)
	{
		ENG_LOG_ERROR("Invalid program");
		return false;
	}
	program.render();
	program.setMat4("projectionMat", proj);

	// Wireframe is on?
	if (isWireframe())
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Multipass rendering:
	for (uint32_t l = 0; l < list.getNrOfLights(); l++)
	{
		// Enable addictive blending from light 1 on:
		if (l == 1)
		{
			glEnable(GL_BLEND);
			glBlendFunc(GL_ONE, GL_ONE);
		}

		// Render one light at time:
		const Eng::List::RenderableElem& lightRe = list.getRenderableElem(l);
		glm::mat4 lightFinalMatrix = camera * lightRe.matrix;
		lightRe.reference.get().render(0, &lightFinalMatrix);

		// Render meshes:
		list.render(camera, Eng::List::Pass::meshes);

		//break;

	}

	// Disable blending, in case we used it:
	if (list.getNrOfLights() > 1)
		glDisable(GL_BLEND);

	// Wireframe is on?
	if (isWireframe())
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// Done:   
	return true;
}
