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

// Uniforms:
uniform mat4 modelMat;

void main()
{   
   gl_Position = modelMat * vec4(a_vertex, 1.0f);
})";


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline geometry shader.
 */
static const std::string pipeline_gs = R"(
#version 460 core
 
// Per-vertex data from verted shader:
layout(triangles) in;

// Newly generated vertices, a maximum of 18 = 3 * 6
layout(triangle_strip, max_vertices=18) out;

// Uniforms:
uniform mat4 projections[6];

// Varying
out vec4 worldSpaceFragment;

void main()
{  

	for(int side = 0; side < 6; side++)
	{

		gl_Layer = side;

		for(int v = 0; v < 3; v++)
		{

			worldSpaceFragment = gl_in[v].gl_Position;
			gl_Position = projections[side] * worldSpaceFragment;
			EmitVertex();
		}

		EndPrimitive();

	}

})";


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Default pipeline fragment shader.
 */
static const std::string pipeline_fs = R"(
#version 460 core

uniform vec3 lightPosition;
uniform float farPlane;

in vec4 worldSpaceFragment;

void main()
{

	// Map depth to [0,1] for easier shadow map calcs

	float distance = length(worldSpaceFragment.xyz - lightPosition);
	distance = distance / farPlane;

	gl_FragDepth = distance;

})";




/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief PipelinePointShadows reserved structure.
 */
struct Eng::PipelineCubemap::Reserved
{
    Eng::Shader vs;
    Eng::Shader gs;
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
ENG_API Eng::PipelineCubemap::PipelineCubemap() : reserved(std::make_unique<Eng::PipelineCubemap::Reserved>())
{
    ENG_LOG_DETAIL("[+]");
    this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name node name
 */
ENG_API Eng::PipelineCubemap::PipelineCubemap(const std::string& name) : Eng::Pipeline(name), reserved(std::make_unique<Eng::PipelineCubemap::Reserved>())
{
    ENG_LOG_DETAIL("[+]");
    this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor.
 */
ENG_API Eng::PipelineCubemap::PipelineCubemap(PipelineCubemap&& other) : Eng::Pipeline(std::move(other)), reserved(std::move(other.reserved))
{
    ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::PipelineCubemap::~PipelineCubemap()
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
const Eng::Texture ENG_API& Eng::PipelineCubemap::getShadowMap() const
{
    return reserved->depthCubeMap;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Initializes this pipeline.
 * @return TF
 */
bool ENG_API Eng::PipelineCubemap::init()
{
    // Already initialized?
    if (this->Eng::Managed::init() == false)
        return false;
    if (!this->isDirty())
        return false;

    // Build:
    reserved->vs.load(Eng::Shader::Type::vertex, pipeline_vs);
    reserved->gs.load(Eng::Shader::Type::geometry, pipeline_gs);
    reserved->fs.load(Eng::Shader::Type::fragment, pipeline_fs);
    if (reserved->program.build({ reserved->vs, reserved->gs, reserved->fs }) == false)
    {
        ENG_LOG_ERROR("Unable to build shadow mapping program");
        return false;
    }
    this->setProgram(reserved->program);

    // Depth cubemap:
    if (reserved->depthCubeMap.create(depthTextureSize, depthTextureSize, Eng::Texture::Format::depth, true) == false)
    {
        ENG_LOG_ERROR("Unable to init depth cubemap");
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
bool ENG_API Eng::PipelineCubemap::free()
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
bool ENG_API Eng::PipelineCubemap::render(const Eng::List::RenderableElem& lightRe, const Eng::List& list)
{
    // Safety net:
    if (list == Eng::List::empty || !dynamic_cast<const Eng::Light*>(&lightRe.reference.get()))
    {
        ENG_LOG_ERROR("Invalid params");
        return false;
    }
  
    const Eng::Light& light = dynamic_cast<const Eng::Light&>(lightRe.reference.get());


    // Just to update the cache
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

    // Compute surrounding projection matrices:

    Eng::Fbo &fbo = reserved->fbo;

    auto shadowProjection = glm::perspective(glm::radians(90.0f), static_cast<float>(fbo.getSizeX()) / static_cast<float>(fbo.getSizeY()), 1.0f, 1000.0f);
    auto lightPosition = glm::vec3{ light.getWorldMatrix()[3] };

	std::vector<glm::mat4> lightSpaceProjections;

    // Right, left, top, bottom, near, far

    lightSpaceProjections.push_back(shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3{  1.0f,  0.0f,  0.0f }, glm::vec3{ 0.0f, -1.0f,  0.0f }));
    lightSpaceProjections.push_back(shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3{ -1.0f,  0.0f,  0.0f }, glm::vec3{ 0.0f, -1.0f,  0.0f }));
    lightSpaceProjections.push_back(shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3{  0.0f,  1.0f,  0.0f }, glm::vec3{ 0.0f,  0.0f,  1.0f }));
    lightSpaceProjections.push_back(shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3{  0.0f, -1.0f,  0.0f }, glm::vec3{ 0.0f,  0.0f, -1.0f }));
    lightSpaceProjections.push_back(shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3{  0.0f,  0.0f,  1.0f }, glm::vec3{ 0.0f, -1.0f,  0.0f }));
    lightSpaceProjections.push_back(shadowProjection * glm::lookAt(lightPosition, lightPosition + glm::vec3{  0.0f,  0.0f, -1.0f }, glm::vec3{ 0.0f, -1.0f,  0.0f }));

    program.setMat4v("projections", lightSpaceProjections[0], 6);
    program.setFloat("farPlane", 1000.0f);

    glm::mat4 lightWorldPosition = light.getWorldMatrix();

    light.render(0, &lightWorldPosition);

    fbo.render();

    glClear(GL_DEPTH_BUFFER_BIT);
    glColorMask(0, 0, 0, 0);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    list.render(glm::mat4{ 1.0f }, Eng::List::Pass::meshes);

    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);
    glColorMask(1, 1, 1, 1);

    Eng::Base& eng = Eng::Base::getInstance();
    Eng::Fbo::reset(eng.getWindowSize().x, eng.getWindowSize().y);

    // Done:   
    return true;
}
