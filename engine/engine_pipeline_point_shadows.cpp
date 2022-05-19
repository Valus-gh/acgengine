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

void main()
{   
   gl_Position = modelviewMat * vec4(a_vertex, 1.0f);
})";


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline fragment shader.
 */
static const std::string pipeline_fs = R"(
#version 460 core

void main()
{
})";


/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief PipelinePointShadows reserved structure.
 */
struct Eng::PipelinePointShadows::Reserved
{
    Eng::Shader vs;
    Eng::Shader fs;
    Eng::Program program;
    Eng::Texture depthCubeMap;
    Eng::Fbo fbo;


    /**
     * Constructor.
     */
    Reserved()
    {}
};


/////////////////////////////////////////
// BODY OF CLASS PipelinePointShadows //
/////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::PipelinePointShadows::PipelinePointShadows() : reserved(std::make_unique<Eng::PipelinePointShadows::Reserved>())
{
    ENG_LOG_DETAIL("[+]");
    this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name node name
 */
ENG_API Eng::PipelinePointShadows::PipelinePointShadows(const std::string& name) : Eng::Pipeline(name), reserved(std::make_unique<Eng::PipelinePointShadows::Reserved>())
{
    ENG_LOG_DETAIL("[+]");
    this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor.
 */
ENG_API Eng::PipelinePointShadows::PipelinePointShadows(PipelinePointShadows&& other) : Eng::Pipeline(std::move(other)), reserved(std::move(other.reserved))
{
    ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::PipelinePointShadows::~PipelinePointShadows()
{
    ENG_LOG_DETAIL("[-]");
    if (this->isInitialized())
        free();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets shadow map texture reference.
 * @return shadow map texture reference
 */
const Eng::Texture ENG_API& Eng::PipelinePointShadows::getShadowMap() const
{
    return reserved->depthCubeMap;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Initializes this pipeline.
 * @return TF
 */
bool ENG_API Eng::PipelinePointShadows::init()
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
        ENG_LOG_ERROR("Unable to build shadow mapping program");
        return false;
    }
    this->setProgram(reserved->program);

    // Depth map:
    if (reserved->depthCubeMap.create(depthTextureSize, depthTextureSize, Eng::Texture::Format::depth, true) == false)
    {
        ENG_LOG_ERROR("Unable to init depth map");
        return false;
    }

    // Depth FBO:
    reserved->fbo.attachTexture(reserved->depthCubeMap);
    if (reserved->fbo.validate() == false)
    {
        ENG_LOG_ERROR("Unable to init depth FBO");
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
bool ENG_API Eng::PipelinePointShadows::free()
{
    if (this->Eng::Managed::free() == false)
        return false;

    // Done:   
    return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Main rendering method for the pipeline.
 * @param lightRe light renderable element
 * @param list list of renderables
 * @return TF
 */
bool ENG_API Eng::PipelinePointShadows::render(const Eng::List::RenderableElem& lightRe, const Eng::List& list)
{
    // Safety net:
    if (list == Eng::List::empty || !dynamic_cast<const Eng::Light*>(&lightRe.reference.get()))
    {
        ENG_LOG_ERROR("Invalid params");
        return false;
    }
  

    // Done:   
    return true;
}
