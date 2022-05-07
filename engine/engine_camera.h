/**
 * @file		engine_camera.h
 * @brief	A camera class
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */
#pragma once



 /**
  * @brief Class for modeling a camera.
  */
class ENG_API Camera : public Eng::Node
{
//////////
public: //
//////////

   // Special values:
   static Camera empty;
   

   // Const/dest:
   Camera();
   Camera(Camera &&other);
   Camera(Camera const &) = delete;
   ~Camera();   

   // Operators:
   void operator=(Camera const&) = delete;

   // Get/set:   
   void setProjMatrix(const glm::mat4 &projMatrix);
   const glm::mat4 &getProjMatrix() const;
   
   // Rendering methods:   
   bool render(uint32_t value = 0, void *data = nullptr) const;   

   // Cache:
   static Camera &getCached();
      

///////////
private: //
///////////

   // Cache:
   static std::reference_wrapper<Eng::Camera> cache;

   // Reserved:
   struct Reserved;
   std::unique_ptr<Reserved> reserved;

   // Const/dest:
   Camera(const std::string &name);
};





