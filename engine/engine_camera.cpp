/**
 * @file		engine_camera.cpp
 * @brief	A camera class
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main include:
   #include "engine.h"

   // GLM:
   #include <glm/gtc/packing.hpp>  

   // OGL:      
   #include <GL/glew.h>
   #include <GLFW/glfw3.h>
   


////////////
// STATIC //
////////////

   // Special values:
   Eng::Camera Eng::Camera::empty("[empty]");

   // Cache:
   std::reference_wrapper<Eng::Camera> Eng::Camera::cache = Eng::Camera::empty;



/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief Camera class reserved structure.
 */
struct Eng::Camera::Reserved
{  
   glm::mat4 projMatrix;                         ///< Projection matrix


   /**
    * Constructor
    */
   Reserved() : projMatrix{ 1.0f }
   {}
};



//////////////////////////
// BODY OF CLASS Camera //
//////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::Camera::Camera() : reserved(std::make_unique<Eng::Camera::Reserved>())
{	
   ENG_LOG_DETAIL("[+]");   
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name mesh name
 */
ENG_API Eng::Camera::Camera(const std::string &name) : Eng::Node(name), reserved(std::make_unique<Eng::Camera::Reserved>())
{	
   ENG_LOG_DETAIL("[+]");   
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor. 
 */
ENG_API Eng::Camera::Camera(Camera &&other) : Eng::Node(std::move(other)), reserved(std::move(other.reserved))
{  
   ENG_LOG_DETAIL("[M]");   
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::Camera::~Camera()
{
   ENG_LOG_DETAIL("[-]");   
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Set camera projection matrix.
 * @param projMatrix projection matrix as glm mat4x4 
 */
void ENG_API Eng::Camera::setProjMatrix(const glm::mat4 &projMatrix) 
{		
   reserved->projMatrix = projMatrix;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Get camera projection matrix. 
 * @return projection matrix as glm 4x4 matrix
 */
const glm::mat4 ENG_API &Eng::Camera::getProjMatrix() const
{	
   return reserved->projMatrix;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Get the last rendered camera.
 * @return last rendered camera
 */
Eng::Camera ENG_API &Eng::Camera::getCached()
{
   return cache;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Rendering method. 
 * @param value generic value
 * @param data generic pointer to any kind of data
 * @return TF
 */
bool ENG_API Eng::Camera::render(uint32_t value, void *data) const
{	
   Eng::Program &program = dynamic_cast<Eng::Program &>(Eng::Program::getCached());
   program.setMat4("projectionMat", reserved->projMatrix);
   
   // Done:
   Eng::Camera::cache = const_cast<Eng::Camera &>(*this);
   return true;
}
