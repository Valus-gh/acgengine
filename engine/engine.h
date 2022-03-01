/**
 * @file		engine.h
 * @brief	Simple 3D engine main include
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */
#pragma once



//////////////
// #INCLUDE //
//////////////
   
   // C/C++:      
   #include <string>
   #include <vector>
   #include <list>   
   #include <memory> 

   // GLM:
#ifndef _DEBUG
   #define GLM_FORCE_INLINE
   #define GLM_FORCE_SSE4
#endif
   #include <glm/glm.hpp>   
   #include <glm/detail/type_half.hpp>
   #include <glm/gtc/matrix_transform.hpp>
   #include <glm/gtc/matrix_inverse.hpp>
   #include <glm/gtc/type_ptr.hpp>
   #include <glm/gtx/string_cast.hpp>   
   #include <glm/gtx/quaternion.hpp>   



/////////////
// VERSION //
/////////////

   // Export API:
#ifdef _WINDOWS
   // Specifies i/o linkage (VC++ spec):
   #ifdef ENGINE_EXPORTS
      #define ENG_API __declspec(dllexport)
   #else
      #define ENG_API __declspec(dllimport)
   #endif      

   // Get rid of annoying warnings:
   #pragma warning(disable : 4251) 
#endif



///////////////
// NAMESPACE //
///////////////

namespace Eng {



//////////////
// #INCLUDE //
//////////////   

   // Logging:
   #include "engine_log.h"

   // Extra stuff:
   #include "engine_timer.h"



///////////////////////
// MAIN ENGINE CLASS //
///////////////////////
   
/**
 * @brief Base engine main class. This class is a singleton.
 */
class ENG_API Base
{
//////////
public: //
//////////	   

   // Callback signatures:
   typedef void (* KeyboardCallback)   (int key, int scancode, int action, int mods);
   typedef void (* MouseCursorCallback)(double mouseX, double mouseY);
   typedef void (* MouseButtonCallback)(int button, int action, int mods);
   typedef void (* MouseScrollCallback)(double scrollX, double scrollY);

   // Const/dest:
   Base(Base const &) = delete;
   virtual ~Base();

   // Operators:
   void operator=(Base const &) = delete;

   // Singleton:
   static Base &getInstance();

   // Init/free:
   bool init();
   bool free();

   // Get/set:
   uint64_t getFrameNr() const;

   // Management:
   bool processEvents();
   bool clear();
   bool swap();
   bool setKeyboardCallback(KeyboardCallback cb);
   bool setMouseCursorCallback(MouseCursorCallback cb);
   bool setMouseButtonCallback(MouseButtonCallback cb);
   bool setMouseScrollCallback(MouseScrollCallback cb);


/////////////
protected: //
/////////////	

   // Reserved:
   struct Reserved;
   std::unique_ptr<Reserved> reserved;

   // Const/dest:
   Base();
};

}; // end of namespace Eng::
