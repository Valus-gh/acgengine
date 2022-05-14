/**
 * @file		engine_pipeline_shadowmapping.cpp 
 * @brief	A pipeline for generating planar shadow maps
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
uniform mat4 modelMat;        // Transformation per obejct
uniform mat4 viewMat;         // Transformation into camera space
uniform mat4 projectionMat;   // Projection
uniform mat3 normalMat;

// Varying:
out vec4 fragPosition;
out mat3 tangentSpace;
out vec2 uv;


void main()
{
   // ToDo: create tangent space matrix here

   uv             = a_uv;
   fragPosition   = modelMat * vec4(a_vertex, 1.0f);
   vec4 tmp       = viewMat * fragPosition;
   gl_Position    = projectionMat * tmp;
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

// Varying:
in vec4 fragPosition;
in mat3 tangentSpace;
in vec2 uv;

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

   normal_texel.xyz = tangentSpace * getNormal(normal_texel);

   positionOut = fragPosition;
   normalOut   = vec4(normal_texel.xyz, metalness_texel.x);
   albedoOut   = vec4(albedo_texel.xyz, roughness_texel.x);
})";



/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief PipelineShadowMapping reserved structure.
 */
struct Eng::PipelineGeometry::Reserved
{  
   Eng::Shader vs;
   Eng::Shader fs;
   Eng::Program program;
   Eng::Texture posTex;       // xyz in world
   Eng::Texture normalTex;    // xyz in world, w metalness
   Eng::Texture matTex;       // albedo rgb, alpha roughness
   Eng::Texture depthTex;     // albedo rgb, alpha roughness
   Eng::Fbo fbo;


   /**
    * Constructor. 
    */
   Reserved()
   {}
};



/////////////////////////////////////////
// BODY OF CLASS PipelineShadowMapping //
/////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::PipelineGeometry::PipelineGeometry() : reserved(std::make_unique<Eng::PipelineGeometry::Reserved>())
{	
   ENG_LOG_DETAIL("[+]");      
   this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name node name
 */
ENG_API Eng::PipelineGeometry::PipelineGeometry(const std::string &name) : Eng::Pipeline(name), reserved(std::make_unique<Eng::PipelineGeometry::Reserved>())
{	   
   ENG_LOG_DETAIL("[+]");   
   this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor. 
 */
ENG_API Eng::PipelineGeometry::PipelineGeometry(PipelineGeometry&&other) : Eng::Pipeline(std::move(other)), reserved(std::move(other.reserved))
{  
   ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::PipelineGeometry::~PipelineGeometry()
{	
   ENG_LOG_DETAIL("[-]");
   if (this->isInitialized())
      free();
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets position texture reference.
 * @return position texture reference
 */
const Eng::Texture ENG_API &Eng::PipelineGeometry::getPositionBuffer() const
{	
   return reserved->posTex;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets normal texture reference.
 * @return normal texture reference
 */
const Eng::Texture ENG_API& Eng::PipelineGeometry::getNormalBuffer() const
{
   return reserved->normalTex;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets material texture reference.
 * @return material texture reference
 */
const Eng::Texture ENG_API& Eng::PipelineGeometry::getMaterialBuffer() const
{
   return reserved->matTex;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets depth texture.
 * @return material texture reference
 */
const Eng::Texture ENG_API& Eng::PipelineGeometry::getDepthBuffer() const
{
   return reserved->depthTex;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Initializes this pipeline. 
 * @return TF
 */
bool ENG_API Eng::PipelineGeometry::init()
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

   // Depth FBO:
   reserved->fbo.attachTexture(reserved->posTex);
   reserved->fbo.attachTexture(reserved->normalTex);
   reserved->fbo.attachTexture(reserved->matTex);
   reserved->fbo.attachTexture(reserved->depthTex);
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
bool ENG_API Eng::PipelineGeometry::free()
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
bool ENG_API Eng::PipelineGeometry::render(glm::mat4& viewMatrix, const Eng::List &list)
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


   // Apply program:
   Eng::Program &program = getProgram();
   if (program == Eng::Program::empty)
   {
      ENG_LOG_ERROR("Invalid program");
      return false;
   }   
   program.render();    
   program.setMat4("projectionMat", Eng::Camera::getCached().getProjMatrix());
   // program.setMat4("modelviewMat", viewMatrix);
   
   // Bind FBO and change OpenGL settings:
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   reserved->fbo.render();
   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
   

   // Render meshes:   
   list.render(viewMatrix, Eng::List::Pass::meshes);         

   // Redo OpenGL settings:
   glCullFace(GL_BACK);
   
   Eng::Base &eng = Eng::Base::getInstance();
   Eng::Fbo::reset(eng.getWindowSize().x, eng.getWindowSize().y);   
  
   // Done:   
   return true;
}
