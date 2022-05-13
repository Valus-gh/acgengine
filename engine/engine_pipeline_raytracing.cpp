/**
 * @file		engine_pipeline_raytracing.cpp 
 * @brief	A pipeline for doing simple ray tracing on GPU
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
static const std::string pipeline_cs = R"(
#version 460 core

// This is the (hard-coded) workgroup size:
layout (local_size_x = 8, local_size_y = 8) in;



/////////////
// #DEFINE //
/////////////

   #define K_EPSILON     1e-4f                // Tolerance around zero   
   #define FLT_MAX       3.402823466e+38f     // Max float value
   #define NR_OF_BOUNCES 4                    // Number of bounces
   // #define CULLING                            // Back face culling enabled when defined
   // #define SHOW_BOUNCES_AS_COLORS             // When defined, each pixel has a different color according to the bounce nr.



///////////////
// TRIANGLES //
///////////////

struct TriangleStruct 
{    
   vec4 v[3];
   vec4 n[3];
   uint matId;
   uint _pad[3];
};
   
layout(std430, binding=0) buffer SceneData
{     
   TriangleStruct triangle[];     
};   



////////////
// LIGHTS //
////////////
   
struct LightStruct 
{    
   vec4 position;     
   vec4 color;
};
   
layout(std430, binding=1) buffer LightData
{     
   LightStruct light[];     
};   



//////////////////////
// BOUNDING SPHERES //
//////////////////////

struct BSphereStruct 
{    
   vec4 position;
   float radius;
   uint firstTriangle;
   uint nrOfTriangles;
   uint _pad;
};
   
layout(std430, binding=2) buffer BSphereData
{     
   BSphereStruct bsphere[];     
};   

/////////////////////////
// MATERIAL PROPERTIES //
/////////////////////////



///////////////////
// LOCAL STRUCTS //
///////////////////

/**
 * Structure for modeling a ray. 
 */
struct Ray
{ 
   vec3 origin;    // Ray origin point
   vec3 dir;       // Normalized ray direction  
};


/**
 * Structure reporting information about the collision. 
 */
struct HitInfo 
{  
   uint triangle;  // Triangle index (within the triangle[] array)
   float t, u, v;          // Triangle barycentric coords
   vec3 color;             // Triangle color
   vec3 ambient;           // Triangle ambient
   vec3 diffuse;           // Triangle diffuse term
   vec3 specular;          // Triangle specular term
   float shininess;        // Triangle shininess
   vec3 collisionPoint;    // Triangle's coords at collision point
   vec3 normal;            // Triangle's normal at collision point
   vec3 faceNormal;        // Triangle's face normal
};



////////////
// IN/OUT //
////////////

   // Uniforms:
   uniform uint nrOfTriangles;
   uniform uint nrOfLights;
   uniform uint nrOfBSpheres;
   uniform vec4 eyePosition;
   uniform vec4 ray00;
   uniform vec4 ray01;
   uniform vec4 ray10;
   uniform vec4 ray11;

   // Output framebuffers:
   layout(binding = 0, rgba8) uniform image2D colorBuffer;             // Current frame and/or accumulation buffer   



///////////////
// FUNCTIONS //
///////////////

/**
 * Ray-sphere intersection.
 * param ray input ray
 * param center sphere center coords
 * param radius sphere radius size
 * param t output collision distance
 * return true on collision, false otherwise
 */
bool intersectSphere(const Ray ray, 
                     const vec3 center, const float radius,
                     out float t)
{ 
   float t0, t1; // solutions for t if the ray intersects 

   // Geometric solution:
   vec3 L = center - ray.origin; 
   float tca = dot(L, ray.dir); 
   //if (tca < 0.0f) return false; // the sphere is behind the ray origin
   float d2 = dot(L, L) - tca * tca; 
   if (d2 > (radius * radius)) 
      return false; 
   float thc = sqrt((radius * radius) - d2); 
   t0 = tca - thc; 
   t1 = tca + thc; 

   if (t0 > t1) 
   {
      float _t = t0;
      t0 = t1;
      t1 = _t;
   }
 
   if (t0 < 0.0f) 
   { 
      t0 = t1; // if t0 is negative, let's use t1 instead 
      if (t0 < 0.0f) 
         return false; // both t0 and t1 are negative 
   } 
 
   t = t0;  
   return true; 
} 


/**
 * Ray-triangle intersection.
 * param ray current ray
 * param v0 first triangle vertex
 * param v1 second triangle vertex
 * param v2 third triangle vertex
 * param t output collision distance
 * param u output barycentric coordinate u
 * param v output barycentric coordinate v
 */
bool intersectTriangle(const Ray ray, 
                       const vec3 v0, const vec3 v1, const vec3 v2, 
                       out float t, out float u, out float v) 
{ 
   vec3 v0v1 = v1 - v0; 
   vec3 v0v2 = v2 - v0; 
   vec3 pvec = cross(ray.dir, v0v2); 
   float det = dot(v0v1, pvec);    

#ifdef CULLING 
    // if the determinant is negative the triangle is backfacing
    // if the determinant is close to 0, the ray misses the triangle    
    if (det < K_EPSILON) 
      return false; 
#else 
    // ray and triangle are parallel if det is close to 0    
    if (abs(det) < K_EPSILON)
      return false;     
#endif 
    float invDet = 1.0f / det; 
 
    vec3 tvec = ray.origin - v0; 
    u = dot(tvec, pvec) * invDet; 
    if (u < 0.0f || u > 1.0f)     
      return false; 
 
    vec3 qvec = cross(tvec, v0v1); 
    v = dot(ray.dir, qvec) * invDet; 
    if (v < 0.0f || ((u + v) > 1.0f))     
      return false; 
 
    t = dot(v0v2, qvec) * invDet;  
    return (t > 0.0f) ? true : false;     
} 


/**
 * Main intersection method
 * param ray current ray  
 * param info collision information (output)
 * return true when the ray intersects a triangle, false otherwise
 */
bool intersect(const Ray ray, out HitInfo info)
{  
   float dist;
   info.triangle = 999999; // Special value for "no triangle"
   info.t = FLT_MAX;         

   for (uint b = 0; b < nrOfBSpheres; b++)
      if (intersectSphere(ray, bsphere[b].position.xyz, bsphere[b].radius, dist)) 
      {
         float t, u, v;
         for (uint i = bsphere[b].firstTriangle; i < bsphere[b].firstTriangle + bsphere[b].nrOfTriangles; i++) 
            if (intersectTriangle(ray, triangle[i].v[0].xyz, triangle[i].v[1].xyz, triangle[i].v[2].xyz, t, u, v)) 
               if (t < info.t && i != info.triangle)
               {  
                  info.triangle = i;   
                  info.t = t;
                  info.u = u;
                  info.v = v;
                  info.color = vec3(1.0f,0.2f,0.2f);
               }
      }

   // Compute final values:
   if (info.triangle != 999999)
   {  
      info.collisionPoint = ray.origin + info.t * ray.dir;      
      info.normal = normalize(info.u * triangle[info.triangle].n[1].xyz + info.v * triangle[info.triangle].n[2].xyz + (1.0f - info.u - info.v) * triangle[info.triangle].n[0].xyz);      
      if (dot(info.normal, -ray.dir.xyz) < 0.0f) // Coll. from inside
         info.normal = -info.normal;
      
      // Compute face normal:
      vec3 v0v1 = triangle[info.triangle].v[1].xyz - triangle[info.triangle].v[0].xyz;
      vec3 v0v2 = triangle[info.triangle].v[2].xyz - triangle[info.triangle].v[0].xyz;
      info.faceNormal = normalize(cross(v0v1, v0v2));
   }
            
   // Done:
   return info.triangle != 999999;
}


/**
 * Ray casting function for tracing a (recursive) ray within the scene.
 * param ray primary ray
 * return color of the pixel's ray
 */
vec4 rayCasting(Ray ray, ivec2 pix) 
{
   HitInfo hit;   
   vec4 outputColor = vec4(0.0f);
   vec4 throughput = vec4(1.0f);

   for (uint c = 0; c < NR_OF_BOUNCES; c++)   
      if (intersect(ray, hit))    
      {  
         // Compute illumination:
         vec4 illum = vec4(0.0f);
         for (uint l = 0; l < nrOfLights; l++)
         {
            vec3 L = normalize(light[l].position.xyz - hit.collisionPoint);
            float lightDist = distance(light[l].position.xyz, hit.collisionPoint);

            // Shadow ray:
            Ray shadowRay;
            HitInfo shadowHit;

            shadowRay.origin = hit.collisionPoint.xyz + hit.faceNormal.xyz * (2.0f * K_EPSILON);
            shadowRay.dir = L;
         
            float shadowDimmer = 1.0f;
            if (intersect(shadowRay, shadowHit))                  
               if (distance(hit.collisionPoint.xyz, shadowHit.collisionPoint.xyz) < lightDist)
                  shadowDimmer = 0.0f;      
            float attenuation = max(0.0f, 1.0f - lightDist / 300.0f); // 300.0f is the light influence radius
            shadowDimmer = shadowDimmer * attenuation; 
            
            // Diffuse:                        
            illum += shadowDimmer * throughput * light[l].color * vec4(clamp(dot(hit.normal.xyz, L), 0.0f, 1.0f));            

            // Specular:                        
            const vec3 V = normalize(ray.origin.xyz - hit.collisionPoint.xyz);
            vec3 H = normalize(L + V);
            illum += shadowDimmer * throughput * light[l].color * vec4(pow(clamp(dot(hit.normal.xyz, H), 0.0f, 1.0f), 1000.0f));            
            
            // Material absorption:
            throughput *= 0.5f;
         }      
         
         //outputColor = mix(outputColor, illum, 1.0f / float(c + 1));
         outputColor += illum;
         
         // Update next ray:
         ray.origin = hit.collisionPoint.xyz + hit.faceNormal.xyz * (2.0f * K_EPSILON);
         ray.dir = reflect(ray.dir, hit.normal.xyz);

#ifdef SHOW_BOUNCES_AS_COLORS         
         // Store nr. of bounces:
         switch (c)
         {
            case 0: outputColor = vec4(1.0f, 0.0f, 0.0f, 0.0f); break;
            case 1: outputColor = vec4(0.0f, 1.0f, 0.0f, 0.0f); break;
            case 2: outputColor = vec4(0.0f, 0.0f, 1.0f, 0.0f); break;
            case 3: outputColor = vec4(1.0f, 1.0f, 0.0f, 0.0f); break;
            case 4: outputColor = vec4(0.0f, 1.0f, 1.0f, 0.0f); break;               
            case 5: outputColor = vec4(1.0f, 1.0f, 1.0f, 0.0f); break;               
         }
#endif
         // outputColor = vec4(hit.normal, 1.0f) / 2.0f + 0.5f; // Uncomment to show normal vectors
         // outputColor = vec4(hit.faceNormal, 1.0f) / 2.0f + 0.5f; // Uncomment to show face normal vectors
      }   
      else
         return outputColor;
   
   // Done:
   return outputColor;
}



//////////
// MAIN //
//////////

void main()
{   
   // Pixel coordinates:
   ivec2 pix = ivec2(gl_GlobalInvocationID.xy);
   ivec2 size = imageSize(colorBuffer);

   // Avoid out of range values:
   if (pix.x >= size.x || pix.y >= size.y)   
      return;  

   // Primary ray tracing:
   Ray ray;   
   vec2 pos = (vec2(pix) / vec2(size.x, size.y)) - 0.5f;
   const float sceneExt = 50.0f;
   ray.origin = vec3(pos.x * sceneExt, pos.y * sceneExt, 100.0f);
   ray.dir = vec3(0.0f, 0.0f, -1.0f);   

   // Perspective:
   pos = (vec2(pix) / vec2(size.x, size.y));
   ray.origin = eyePosition.xyz;
   ray.dir = normalize(mix(mix(ray00.xyz, ray01.xyz, pos.y), mix(ray10.xyz, ray11.xyz, pos.y), pos.x));   

   // Ray casting:
   vec4 color = rayCasting(ray, pix);     

   // Write output:   
   imageStore(colorBuffer, pix, color);
})";


/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief PipelineRayTracing reserved structure.
 */
struct Eng::PipelineRayTracing::Reserved
{
	Eng::Shader cs;
	Eng::Program program;
	Eng::Ssbo triangles; ///< List of triangles in world coords
	Eng::Ssbo lights; ///< List of lights in world coords
	Eng::Ssbo bspheres; ///< List of bounding spheres in world coords
	Eng::Texture colorBuffer; ///< Output image of the ray tracer

	// Scene-specific:
	uint32_t nrOfTriangles;
	uint32_t nrOfLights;
	uint32_t nrOfBSpheres;


	/**
	 * Constructor. 
	 */
	Reserved() : nrOfTriangles{0}, nrOfLights{0}, nrOfBSpheres{0}
	{
	}
};


//////////////////////////////////////
// BODY OF CLASS PipelineRayTracing //
//////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::PipelineRayTracing::PipelineRayTracing() : reserved(std::make_unique<Eng::PipelineRayTracing::Reserved>())
{
	ENG_LOG_DETAIL("[+]");
	this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name node name
 */
ENG_API Eng::PipelineRayTracing::PipelineRayTracing(const std::string& name) : Eng::Pipeline(name),
                                                                               reserved(
	                                                                               std::make_unique<
		                                                                               Eng::PipelineRayTracing::Reserved>())
{
	ENG_LOG_DETAIL("[+]");
	this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor. 
 */
ENG_API Eng::PipelineRayTracing::PipelineRayTracing(PipelineRayTracing&& other) : Eng::Pipeline(std::move(other)),
	reserved(std::move(other.reserved))
{
	ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::PipelineRayTracing::~PipelineRayTracing()
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
bool ENG_API Eng::PipelineRayTracing::init()
{
	// Already initialized?
	if (this->Eng::Managed::init() == false)
		return false;
	if (!this->isDirty())
		return false;

	// Build:
	reserved->cs.load(Eng::Shader::Type::compute, pipeline_cs);
	if (reserved->program.build({reserved->cs}) == false)
	{
		ENG_LOG_ERROR("Unable to build RayTracing program");
		return false;
	}
	this->setProgram(reserved->program);

	// Create output image:   
	Eng::Base& eng = Eng::Base::getInstance();
	reserved->colorBuffer.create(eng.getWindowSize().x, eng.getWindowSize().y, Eng::Texture::Format::r8g8b8a8);

	// Done: 
	this->setDirty(false);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Releases this pipeline.
 * @return TF
 */
bool ENG_API Eng::PipelineRayTracing::free()
{
	if (this->Eng::Managed::free() == false)
		return false;

	// Done:   
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Gets the color buffer used as ray tracing output.
 * @return color buffer texture reference
 */
const Eng::Texture ENG_API& Eng::PipelineRayTracing::getColorBuffer() const
{
	return reserved->colorBuffer;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Migrates the data from a standard list into RT-specific structures. 
 * @param list list of renderables
 * @return TF
 */
bool ENG_API Eng::PipelineRayTracing::migrate(const Eng::List& list)
{
	// Safety net:
	if (list == Eng::List::empty)
	{
		ENG_LOG_ERROR("Invalid params");
		return false;
	}


	////////////////////////////////////////
	// 1st pass: count elems and fill lights
	uint32_t nrOfVertices = 0;
	uint32_t nrOfFaces = 0;
	uint32_t nrOfLights = list.getNrOfLights();
	uint32_t nrOfRenderables = list.getNrOfRenderableElems();
	uint32_t nrOfBSpheres = nrOfRenderables - nrOfLights;

	for (uint32_t c = nrOfLights; c < nrOfRenderables; c++)
	{
		const Eng::List::RenderableElem& re = list.getRenderableElem(c);
		const Eng::Mesh& mesh = dynamic_cast<const Eng::Mesh&>(re.reference.get());
		const Eng::Vbo& vbo = mesh.getVbo();
		const Eng::Ebo& ebo = mesh.getEbo();
		nrOfVertices += vbo.getNrOfVertices();
		nrOfFaces += ebo.getNrOfFaces();
	}

	ENG_LOG_DEBUG("Tot. nr. of faces . . :  %u", nrOfFaces);
	ENG_LOG_DEBUG("Tot. nr. of vertices  :  %u", nrOfVertices);


	/////////////////////////
	// 2nd pass: fill buffers
	std::vector<Eng::PipelineRayTracing::LightStruct> allLights(nrOfLights);
	std::vector<Eng::PipelineRayTracing::TriangleStruct> allTriangles(nrOfFaces);
	std::vector<Eng::PipelineRayTracing::BSphereStruct> allBSpheres(nrOfBSpheres);
	nrOfFaces = 0; // Reset counter

	for (uint32_t c = 0; c < nrOfRenderables; c++)
	{
		const Eng::List::RenderableElem& re = list.getRenderableElem(c);

		// Get renderable world matrices:
		glm::mat4 modelMat = re.matrix;
		glm::mat3 normalMat = glm::inverseTranspose(re.matrix);

		// Lights:
		if (c < nrOfLights)
		{
			const Eng::Light& light = dynamic_cast<const Eng::Light&>(re.reference.get());
			Eng::PipelineRayTracing::LightStruct l;

			l.color = glm::vec4(light.getColor(), 1.0f);
			l.position = modelMat[3];
			allLights[c] = l;
		}

		// Meshes (and bounding spheres):
		else
		{
			const Eng::Mesh& mesh = dynamic_cast<const Eng::Mesh&>(re.reference.get());

			// Read VBO back:
			const Eng::Vbo& vbo = mesh.getVbo();
			std::vector<Eng::Vbo::VertexData> vData(vbo.getNrOfVertices());

			glBindBuffer(GL_ARRAY_BUFFER, vbo.getOglHandle());
			glGetBufferSubData(GL_ARRAY_BUFFER, 0, vbo.getNrOfVertices() * sizeof(Eng::Vbo::VertexData), vData.data());

			// Read EBO back:
			const Eng::Ebo& ebo = mesh.getEbo();
			std::vector<Eng::Ebo::FaceData> fData(ebo.getNrOfFaces());

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo.getOglHandle());
			glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, ebo.getNrOfFaces() * sizeof(Eng::Ebo::FaceData),
			                   fData.data());

			ENG_LOG_DEBUG("Object: %s, data: %s, face: %u, %u, %u", mesh.getName().c_str(),
			              glm::to_string(vData[0].vertex).c_str(), fData[0].a, fData[0].b, fData[0].c);

			// Bounding sphere:
			Eng::PipelineRayTracing::BSphereStruct s;
			s.firstTriangle = nrOfFaces;
			s.nrOfTriangles = ebo.getNrOfFaces();
			s.radius = mesh.getRadius();
			s.position = modelMat[3];
			allBSpheres[c - nrOfLights] = s;

			// Copy faces and vertices into the std::vector:
			for (uint32_t f = 0; f < ebo.getNrOfFaces(); f++)
			{
				Eng::PipelineRayTracing::TriangleStruct t;
				t.matId = 0; // @TODO!

				// First vertex:
				t.v[0] = modelMat * glm::vec4(vData[fData[f].a].vertex, 1.0f);
				t.n[0] = glm::vec4(normalMat * glm::vec3(glm::unpackSnorm3x10_1x2(vData[fData[f].a].normal)), 1.0f);

				// Second vertex:
				t.v[1] = modelMat * glm::vec4(vData[fData[f].b].vertex, 1.0f);
				t.n[1] = glm::vec4(normalMat * glm::vec3(glm::unpackSnorm3x10_1x2(vData[fData[f].b].normal)), 1.0f);

				// Third vertex:
				t.v[2] = modelMat * glm::vec4(vData[fData[f].c].vertex, 1.0f);
				t.n[2] = glm::vec4(normalMat * glm::vec3(glm::unpackSnorm3x10_1x2(vData[fData[f].c].normal)), 1.0f);

				allTriangles[nrOfFaces] = t;
				nrOfFaces++;
			}
		}
	}


	////////////////////////////
	// 3rd: copy data into SSBOs
	reserved->lights.create(nrOfLights * sizeof(Eng::PipelineRayTracing::LightStruct), allLights.data());
	reserved->triangles.create(nrOfFaces * sizeof(Eng::PipelineRayTracing::TriangleStruct), allTriangles.data());
	reserved->bspheres.create(nrOfBSpheres * sizeof(Eng::PipelineRayTracing::BSphereStruct), allBSpheres.data());

	// Done:
	reserved->nrOfTriangles = nrOfFaces;
	reserved->nrOfLights = nrOfLights;
	reserved->nrOfBSpheres = nrOfBSpheres;
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Main rendering method for the pipeline.  
 * @param camera view camera
 * @param list list of renderables
 * @return TF
 */
bool ENG_API Eng::PipelineRayTracing::render(const Eng::Camera& camera, const Eng::List& list)
{
	// Safety net:
	if (camera == Eng::Camera::empty || list == Eng::List::empty)
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

	// Get camera rays in the corners (will then be interpolated in the compute shader):
	glm::mat4 cameraMat = camera.getWorldMatrix();
	glm::mat4 viewMat = glm::inverse(cameraMat);
	glm::mat4 invViewProjMat = glm::inverse(camera.getProjMatrix() * viewMat);
	glm::vec4 eyePosition = cameraMat[3];

	glm::vec4 ray00 = invViewProjMat * glm::vec4(-1.0f, -1.0f, 0.0f, 1.0f);
	ray00 /= ray00.w;
	ray00 -= eyePosition;
	ray00 = glm::normalize(ray00);

	glm::vec4 ray01 = invViewProjMat * glm::vec4(-1.0f, 1.0f, 0.0f, 1.0f);
	ray01 /= ray01.w;
	ray01 -= eyePosition;
	ray01 = glm::normalize(ray01);

	glm::vec4 ray10 = invViewProjMat * glm::vec4(1.0f, -1.0f, 0.0f, 1.0f);
	ray10 /= ray10.w;
	ray10 -= eyePosition;
	ray10 = glm::normalize(ray10);

	glm::vec4 ray11 = invViewProjMat * glm::vec4(1.0f, 1.0f, 0.0f, 1.0f);
	ray11 /= ray11.w;
	ray11 -= eyePosition;
	ray11 = glm::normalize(ray11);

	// Apply program:
	Eng::Program& program = getProgram();
	if (program == Eng::Program::empty)
	{
		ENG_LOG_ERROR("Invalid program");
		return false;
	}
	program.render();

	// Bindings:
	reserved->colorBuffer.bindImage();
	reserved->triangles.render(0);
	reserved->lights.render(1);
	reserved->bspheres.render(2);

	// Uniforms:
	program.setUInt("nrOfTriangles", reserved->nrOfTriangles);
	program.setUInt("nrOfLights", reserved->nrOfLights);
	program.setUInt("nrOfBSpheres", reserved->nrOfBSpheres);
	program.setVec4("eyePosition", eyePosition);
	program.setVec4("ray00", ray00);
	program.setVec4("ray01", ray01);
	program.setVec4("ray10", ray10);
	program.setVec4("ray11", ray11);

	// Execute:
	program.compute(reserved->colorBuffer.getSizeX() / 8, reserved->colorBuffer.getSizeY() / 8);
	// 8 is the hard-coded size of the workgroup
	program.wait();

	// Done:   
	return true;
}
