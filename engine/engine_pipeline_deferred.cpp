#include "engine.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

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
uniform mat4 modelMat;        // Transformation per obejct
uniform mat4 viewMat;         // Transformation into camera space
uniform mat4 projectionMat;   // Projection
uniform mat3 normalMat;

// Varying:
out vec4 fragPosition;
out mat3 tbn;
out vec2 uv;

void main()
{
	// TBN matrix for normal mapping

	vec3 normal    = normalMat * a_normal.xyz;
	vec3 tangent   = normalMat * a_tangent.xyz;
    tangent	       = normalize(tangent - dot(tangent,normal) * normal);
	vec3 bitangent = normalize(cross(normal, tangent));

	tbn = mat3(tangent, bitangent, normal);            // Tangent to world
  //tbn = transpose(mat3(tangent, bitangent, normal)); // World to tangent

    uv = a_uv;

	// Fragment position in world coordinates, will be saved in the first channel of the GBuffer
	fragPosition = modelMat * vec4(a_vertex, 1.0f);
	gl_Position  = projectionMat * viewMat * fragPosition;

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

// Varying:
in vec4 fragPosition;
in mat3 tbn;
in vec2 uv;

// Output to the framebuffer:
layout(location=0) out vec4 positionOut;
layout(location=1) out vec4 normalOut;
layout(location=2) out vec4 albedoOut;

/**
 * Uncompresses the normal and brings it into [-1, 1]^3
 * @param texNormal  normal read from texture
 * @return normal in [-1,1]^3
 */
vec3 getNormal(vec4 texNorm)
{
   vec3 tmp = texNorm.xyz * 2.0f - 1.0f;
   tmp.z    = sqrt(1.0f - tmp.x * tmp.x - tmp.y * tmp.y);
   return tmp;   
}

void main()
{
	vec4 albedo_texel    = texture(texture0, uv);
	vec4 normal_texel    = texture(texture1, uv);
	vec4 roughness_texel = texture(texture2, uv);
	vec4 metalness_texel = texture(texture3, uv);

	normal_texel.xyz = tbn * getNormal(normal_texel);

	positionOut = fragPosition;
	normalOut   = vec4(normal_texel.xyz, metalness_texel.x);
	albedoOut   = vec4(albedo_texel.xyz, roughness_texel.x);

})";

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline vertex shader.
 */
static const std::string pipeline_vs_lighting = R"(
#version 460 core
 
// Per-vertex data from VBOs:

// Uniforms:

// Varying:
out vec2 uv;

void main()
{
   float x = -1.0f + float((gl_VertexID & 1) << 2);
   float y = -1.0f + float((gl_VertexID & 2) << 1);
   
   uv.x = (x + 1.0f) * 0.5f;
   uv.y = (y + 1.0f) * 0.5f;
   
   gl_Position = vec4(x, y, 1.0f, 1.0f);

})";


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline fragment shader.
 */
const std::string pipeline_fs_lighting = R"(
#version 460 core
#extension GL_ARB_bindless_texture : require

const float PI = 3.14159265359;

// Uniform (textures):
layout (bindless_sampler) uniform sampler2D texture0;
layout (bindless_sampler) uniform sampler2D texture1;
layout (bindless_sampler) uniform sampler2D texture2;
layout (bindless_sampler) uniform sampler2D texture3;

// Uniform (material):


// Uniform (light and camera):
uniform vec3 camPosition;     // Camera position in World-Space
uniform vec3 lightPosition;   // Light position in World-Space
uniform vec3 lightColor;      // Light color
uniform mat4 lightMatrix;     // Transformation into light space

// Varying:
in vec2 uv;

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
   float closestDepth = texture(texture3, projCoords.xy).r;    

   // Check whether current fragment is in shadow:
   return projCoords.z > closestDepth  ? 1.0f : 0.0f;   
}


void main()
{


   // Texture lookup:
   vec4 pixWorldPos     = texture(texture0, uv);
   vec4 pixWorldNormal  = texture(texture1, uv);
   vec4 pixMaterial     = texture(texture2, uv);

   float metalness      = pixWorldNormal.w;
   float roughness      = pixMaterial.w;

   float a = camPosition.x + lightPosition.x + lightColor.x + metalness + roughness;

   outFragment = vec4(pixWorldPos.xyz, a);

})";

/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief PipelineDefault reserved structure.
 */
struct Eng::PipelineDeferred::Reserved
{
	// GBuffer pass

	Eng::Shader vs;
	Eng::Shader fs;
	Eng::Program program;

	Eng::Texture posTex;       // xyz in world
	Eng::Texture normalTex;    // xyz in world, w metalness
	Eng::Texture matTex;       // albedo rgb, alpha roughness
	Eng::Texture depthTex;     // albedo rgb, alpha roughness
	Eng::Fbo fbo;

	// Lighting Pass

	Eng::Shader vs_lighting;
	Eng::Shader fs_lighting;
	Eng::Program program_lighting;

	Eng::Vao vao_lighting;

	bool wireframe;

	PipelineShadowMapping shadowMapping;


	/**
	 * Constructor.
	 */
	Reserved() : wireframe{ false } {}
};

///////////////////////////////////
// BODY OF CLASS PipelineDeferred //
///////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::PipelineDeferred::PipelineDeferred() : reserved(std::make_unique<Eng::PipelineDeferred::Reserved>())
{
	ENG_LOG_DETAIL("[+]");
	this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name node name
 */
ENG_API Eng::PipelineDeferred::PipelineDeferred(const std::string& name) : Eng::Pipeline(name),
reserved(
	std::make_unique<
	Eng::PipelineDeferred::Reserved>())
{
	ENG_LOG_DETAIL("[+]");
	this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor.
 */
ENG_API Eng::PipelineDeferred::PipelineDeferred(PipelineDeferred&& other) : Eng::Pipeline(std::move(other)),
reserved(std::move(other.reserved))
{
	ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::PipelineDeferred::~PipelineDeferred()
{
	ENG_LOG_DETAIL("[-]");
	if (this->isInitialized())
		free();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets a reference to the shadow mapping pipeline.
 * @return shadow mapping pipeline reference
 */
const Eng::PipelineShadowMapping ENG_API& Eng::PipelineDeferred::getShadowMappingPipeline() const
{
	return reserved->shadowMapping;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the status of the wireframe status.
 * @return wireframe status
 */
bool ENG_API Eng::PipelineDeferred::isWireframe() const
{
	return reserved->wireframe;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Sets the status of the wireframe flag.
 * @param flag wireframe flag
 */
void ENG_API Eng::PipelineDeferred::setWireframe(bool flag)
{
	reserved->wireframe = flag;
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets position texture reference.
 * @return position texture reference
 */
const Eng::Texture ENG_API& Eng::PipelineDeferred::getPositionBuffer() const
{
	return reserved->posTex;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets normal texture reference.
 * @return normal texture reference
 */
const Eng::Texture ENG_API& Eng::PipelineDeferred::getNormalBuffer() const
{
	return reserved->normalTex;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets material texture reference.
 * @return material texture reference
 */
const Eng::Texture ENG_API& Eng::PipelineDeferred::getMaterialBuffer() const
{
	return reserved->matTex;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets depth texture.
 * @return material texture reference
 */
const Eng::Texture ENG_API& Eng::PipelineDeferred::getDepthBuffer() const
{
	return reserved->depthTex;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Initializes this pipeline.
 * @return TF
 */
bool ENG_API Eng::PipelineDeferred::init()
{
	// Already initialized?
	if (this->Eng::Managed::init() == false)
		return false;
	if (!this->isDirty())
		return false;

	// Build gbuffer pass:
	reserved->vs.load(Eng::Shader::Type::vertex, pipeline_vs);
	reserved->fs.load(Eng::Shader::Type::fragment, pipeline_fs);
	if (reserved->program.build({ reserved->vs, reserved->fs }) == false)
	{
		ENG_LOG_ERROR("Unable to build gbuffer program");
		return false;
	}
	this->setProgram(reserved->program);

    // Build gbuffer textures
    // Positions:
    // world.xyz in rgb
	Eng::Base& eng = Eng::Base::getInstance();
	int width = eng.getWindowSize().x;
	int height = eng.getWindowSize().y;
	if (reserved->posTex.create(width, height, Eng::Texture::Format::rgb_float) == false)
	{
		ENG_LOG_ERROR("Unable to init position texture");
		return false;
	}

	// Normals:
	// world.xyz in rgb, metalness in alpha
	if (reserved->normalTex.create(width, height, Eng::Texture::Format::rgba_float) == false)
	{
		ENG_LOG_ERROR("Unable to init normal texture");
		return false;
	}

	// Material properties:
	// albedo in rgb, roughness in alpha 
	if (reserved->matTex.create(width, height, Eng::Texture::Format::rgba_float) == false)
	{
		ENG_LOG_ERROR("Unable to init material texture");
		return false;
	}

	// Depth texture:
	// Necessary for depth testing to work
	if (reserved->depthTex.create(width, height, Eng::Texture::Format::depth) == false)
	{
		ENG_LOG_ERROR("Unable to init depth texture");
		return false;
	}

	// GBuffer FBO:
	reserved->fbo.attachTexture(reserved->posTex);
	reserved->fbo.attachTexture(reserved->normalTex);
	reserved->fbo.attachTexture(reserved->matTex);
	reserved->fbo.attachTexture(reserved->depthTex);
	if (reserved->fbo.validate() == false)
	{
		ENG_LOG_ERROR("Unable to init depth FBO");
		return false;
	}

	// Build lighting pass:
	reserved->vs_lighting.load(Eng::Shader::Type::vertex, pipeline_vs_lighting);
	reserved->fs_lighting.load(Eng::Shader::Type::fragment, pipeline_fs_lighting);
	if (reserved->program_lighting.build({ reserved->vs_lighting, reserved->fs_lighting }) == false)
	{
		ENG_LOG_ERROR("Unable to build lighting program");
		return false;
	}

	// Init dummy VAO:
	if (reserved->vao_lighting.init() == false)
	{
		ENG_LOG_ERROR("Unable to init VAO for fullscreen2D");
		return false;
	}

	// Done: 
	this->setDirty(false);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Releases this pipeline.
 * @return TF
 */
bool ENG_API Eng::PipelineDeferred::free()
{
	if (this->Eng::Managed::free() == false)
		return false;

	// Done:   
	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Main rendering method for the pipeline.
 * @param list list of renderables
 * @return TF
 */
bool ENG_API Eng::PipelineDeferred::render(const Eng::Camera& camera, const Eng::List& list)
{

	// Safety net:
	if (list == Eng::List::empty)
	{
		ENG_LOG_ERROR("Invalid params");
		return false;
	}

	// Just to update the cache
	this->Eng::Pipeline::render(list);

	// Lazy-loading:
	if (this->isDirty())
		if (!this->init())
		{
			ENG_LOG_ERROR("Unable to render (initialization failed)");
			return false;
		}

	// Shadowmapping

	const Eng::List::RenderableElem& lightRe = list.getRenderableElem(0);
	reserved->shadowMapping.render(lightRe, list);

	// Apply GBuffer program:
	Eng::Program& program = reserved->program;
	if (program == Eng::Program::empty)
	{
		ENG_LOG_ERROR("GBuffer program is invalid");
		return false;
	}

	// Render scene data to geometry buffer

	program.render();
	camera.render();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	reserved->fbo.render();
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	list.render(glm::inverse(camera.getWorldMatrix()), Eng::List::Pass::meshes);

	glCullFace(GL_BACK);

	// Unbind GBuffer FBO

	Eng::Base& eng = Eng::Base::getInstance();
	Eng::Fbo::reset(eng.getWindowSize().x, eng.getWindowSize().y);

	// Apply Lighting program:

	reserved->program_lighting.render();

	reserved->posTex.render(0);
	reserved->normalTex.render(1);
	reserved->matTex.render(2);
	reserved->shadowMapping.getShadowMap().render(3);

	// Pass camera position and light position and color parameters

	glm::mat4 camMat = Eng::Camera::getCached().getMatrix();
	float x = camMat[3][0];
	float y = camMat[3][1];
	float z = camMat[3][2];
	glm::vec3 camPos = glm::vec3(x, y, z);

	reserved->program_lighting.setVec3("camPosition", camPos);

	const Eng::Light& light = dynamic_cast<const Eng::Light&>(lightRe.reference.get());

	// Points are in world coordinates, no need to go back from eye coordinates
	glm::mat4 lightMatrix = light.getProjMatrix() * glm::inverse(light.getWorldMatrix());
	light.render(0, &lightMatrix);
	program.setMat4("lightMatrix", lightMatrix);

	// Render scene via GBuffer content with lighting

	reserved->vao_lighting.render();
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Done:   
	return true;

}
