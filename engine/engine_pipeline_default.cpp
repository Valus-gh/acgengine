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
uniform mat4 lightMatrix;

uniform vec3 lightPosition;

// Varying:
out vec4 fragPosition;
out vec4 fragPositionLightSpace;
out vec3 normal;
out vec2 uv;

out vec3 V;
out vec3 L;

void main()
{
   normal = normalMat * a_normal.xyz;

   vec3 tangent = normalMat * a_tangent.xyz;
   tangent = normalize(tangent - dot(tangent, normal) * normal);

   vec3 bitangent = normalize(cross(normal, tangent));

   mat3 tbn = transpose(mat3(tangent, bitangent, normal));
   //tbn = mat3(tangent, bitangent, normal);

   uv = a_uv;

   fragPosition = modelviewMat * vec4(a_vertex, 1.0f);
   fragPositionLightSpace = lightMatrix * fragPosition;
   gl_Position = projectionMat * fragPosition;

   V = tbn * normalize(-fragPosition.xyz);  
   L = tbn * normalize(lightPosition - fragPosition.xyz);   

})";


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline fragment shader.
 */
const std::string pipeline_fs = R"(
#version 460 core
#extension GL_ARB_bindless_texture : require

// Uniform (textures):
layout (bindless_sampler) uniform sampler2D texture0; // Albedo
layout (bindless_sampler) uniform sampler2D texture1; // Normal
layout (bindless_sampler) uniform sampler2D texture2; // Roughness
layout (bindless_sampler) uniform sampler2D texture3; // Metalness
layout (bindless_sampler) uniform sampler2D texture4; // Shadow map

// Uniform (material):
uniform vec3 mtlEmission;
uniform vec3 mtlAlbedo;
uniform float mtlOpacity;
uniform float mtlRoughness;
uniform float mtlMetalness;

// Uniform (light):
uniform vec3 lightColor;
uniform vec3 lightAmbient;
uniform vec3 lightPosition;

// Varying:
in vec4 fragPosition;
in vec4 fragPositionLightSpace;
in vec3 normal;
in vec2 uv;
in vec3 V;
in vec3 L;

// Output to the framebuffer:
out vec4 outFragment;

/**
 * Computes the amount of shadow for a given fragment.
 * @param fragPosLightSpace frament coords in light space
 * @return shadow intensity
 */
float shadowAmount(vec4 fragPosLightSpace)
{
   // From "clip" to "ndc" coords:
   vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
   // Transform to the [0,1] range:
   projCoords = projCoords * 0.5f + 0.5f;
   
   // Get closest depth in the shadow map:
   float closestDepth = texture(texture4, projCoords.xy).r;    
   
   // Check whether current fragment is in shadow:
   return projCoords.z > closestDepth  ? 1.0f : 0.0f;   
} 

void main()
{
   // Texture lookup:
   vec4 albedo_texel = texture(texture0, uv);
   vec4 normal_texel = texture(texture1, uv);
   vec4 roughness_texel = texture(texture2, uv);
   vec4 metalness_texel = texture(texture3, uv);
   float shadow_texel = texture(texture4, uv).r;
   float justUseIt = albedo_texel.r + normal_texel.r + roughness_texel.r + metalness_texel.r + shadow_texel;

   // Calculate z parameter and normalize into [-1,1]
   vec3 normal3d = normal_texel.xyz;
   normal3d.z = sqrt(1.0 - pow(normal3d.x, 2.0) - pow(normal3d.y, 2.0));
   normal3d = normalize(normal3d * 2.0 - 1.0);
   
   // Material props:
   justUseIt += mtlEmission.r + mtlAlbedo.r + mtlOpacity + mtlRoughness + mtlMetalness;

   vec3 fragColor = mtlEmission + lightAmbient;   
   
   vec3 N = normalize(normal3d);   

   // Light only front faces:
   if (dot(N, V) > 0.0f)
   {

      float shadow = 1.0f - shadowAmount(fragPositionLightSpace);

      // Diffuse term:   
      float nDotL = max(0.0f, dot(N, L));      
      fragColor += nDotL * lightColor * shadow;
      
      // Specular term:     
      vec3 H = normalize(L + V);                     
      float nDotH = max(0.0f, dot(N, H));         
      fragColor += pow(nDotH, 70.0f) * lightColor * shadow;         
   }
   
   outFragment = vec4(fragColor * albedo_texel.xyz, justUseIt);   
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

	PipelineShadowMapping shadowMapping;


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
 * Gets a reference to the shadow mapping pipeline.
 * @return shadow mapping pipeline reference
 */
const Eng::PipelineShadowMapping ENG_API& Eng::PipelineDefault::getShadowMappingPipeline() const
{
	return reserved->shadowMapping;
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
bool ENG_API Eng::PipelineDefault::render(const Eng::Camera& camera, const Eng::List& list)
{
	// Safety net:
	if (camera == Eng::Camera::empty || list == Eng::List::empty)
	{
		ENG_LOG_ERROR("Invalid params");
		return false;
	}

	// Just to update the cache:
	this->Eng::Pipeline::render(list);

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

	// Apply camera:   
	camera.render();
	glm::mat4 viewMatrix = glm::inverse(camera.getWorldMatrix());

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
		const Eng::Light& light = dynamic_cast<const Eng::Light&>(lightRe.reference.get());

		// Render shadow map:
		reserved->shadowMapping.render(lightRe, list);

		// Re-enable this pipeline's program:
		program.render();
		glm::mat4 lightFinalMatrix = viewMatrix * lightRe.matrix; // Light position in eye coords
		light.render(0, &lightFinalMatrix);

		lightFinalMatrix = light.getProjMatrix() * glm::inverse(lightRe.matrix) * glm::inverse(viewMatrix); // To convert from eye coords into light space    
		program.setMat4("lightMatrix", lightFinalMatrix);
		reserved->shadowMapping.getShadowMap().render(4);

		// Render meshes:
		list.render(viewMatrix, Eng::List::Pass::meshes);

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
