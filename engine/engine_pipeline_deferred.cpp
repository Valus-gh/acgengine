
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

// Varying:
out vec4 fragPosition;
out vec4 fragPositionLightSpace;
out vec3 normal;
out vec2 uv;

void main()
{
   normal = normalMat * a_normal.xyz;
   uv = a_uv;

   fragPosition = modelviewMat * vec4(a_vertex, 1.0f);
   fragPositionLightSpace = lightMatrix * fragPosition;
   gl_Position = projectionMat * fragPosition;
})";


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline fragment shader.
 */
static const std::string pipeline_fs = R"(
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
 
// Output to the framebuffer:
out vec4 outFragment;

float ShadowCalculation(vec4 fragPosLightSpace)
{
   // Perform perspective divide:
   vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    
   // Transform to [0,1] range:
   projCoords = projCoords * 0.5 + 0.5;
   
   // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords):
   float closestDepth = texture(texture4, projCoords.xy).r; 
   
   // Get depth of current fragment from light's perspective:
   float currentDepth = projCoords.z;
   
   // Check whether current frag pos is in shadow:
   float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
   return shadow;
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

   // Material props:
   justUseIt += mtlEmission.r + mtlAlbedo.r + mtlOpacity + mtlRoughness + mtlMetalness;

   vec3 fragColor = mtlEmission + lightAmbient;   
   
   vec3 N = normalize(normal);   
   vec3 V = normalize(-fragPosition.xyz);   
   vec3 L = normalize(lightPosition - fragPosition.xyz);      

   // Light only front faces:
   if (dot(N, V) > 0.0f)
   {
      float shadow = 1.0f - ShadowCalculation(fragPositionLightSpace);     
      
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
 * @brief PipelineDeferred reserved structure.
 */
struct Eng::PipelineDeferred::Reserved
{
    Eng::Shader vs;
    Eng::Shader fs;
    Eng::Program program;

    bool wireframe;

    PipelineShadowMapping shadowMapping;
    PipelineFullscreenLighting fullscreenLighting;
    PipelineGeometry geometryBuffer;

    /**
     * Constructor.
     */
    Reserved() : wireframe{ false }
    {}
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
ENG_API Eng::PipelineDeferred::PipelineDeferred(const std::string& name) : Eng::Pipeline(name), reserved(std::make_unique<Eng::PipelineDeferred::Reserved>())
{
    ENG_LOG_DETAIL("[+]");
    this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor.
 */
ENG_API Eng::PipelineDeferred::PipelineDeferred(PipelineDeferred&& other) : Eng::Pipeline(std::move(other)), reserved(std::move(other.reserved))
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

    // Build:
    reserved->vs.load(Eng::Shader::Type::vertex, pipeline_vs);
    reserved->fs.load(Eng::Shader::Type::fragment, pipeline_fs);
    if (reserved->program.build({ reserved->vs, reserved->fs }) == false)
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
bool ENG_API Eng::PipelineDeferred::free()
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
const Eng::PipelineShadowMapping ENG_API& Eng::PipelineDeferred::getShadowMappingPipeline() const
{
    return reserved->shadowMapping;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets a reference to the geometry pipeline.
 * @return geometry pipeline reference
 */
const Eng::PipelineGeometry ENG_API& Eng::PipelineDeferred::getGeometryPipeline() const
{
    return reserved->geometryBuffer;
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
 * Main rendering method for the pipeline.
 * @param camera view camera
 * @param list list of renderables
 * @return TF
 */
bool ENG_API Eng::PipelineDeferred::render(const Eng::Camera& camera, const Eng::List& list)
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
            ENG_LOG_ERROR("Unable to render (initialization failed)");
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

    // Render one light at time:
    const Eng::List::RenderableElem& lightRe = list.getRenderableElem(0);
    const Eng::Light& light = dynamic_cast<const Eng::Light&>(lightRe.reference.get());
    // Render shadow map:
    reserved->shadowMapping.render(lightRe, list);

    program.render();

    // Apply camera:   
    camera.render();
    glm::mat4 viewMatrix = glm::inverse(camera.getWorldMatrix());

    // Wireframe is on?
    if (isWireframe())
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    reserved->geometryBuffer.render(viewMatrix, list);

    reserved->fullscreenLighting.render(reserved->geometryBuffer, reserved->shadowMapping, const_cast<Light&>(light), list);

    // Wireframe is on?
    if (isWireframe())
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Done:   
    return true;
}
