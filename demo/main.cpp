/**
 * @file		main.cpp
 * @brief	Engine usage example
 * 
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */



//////////////
// #INCLUDE //
//////////////

   // Main engine header:
   #include "engine.h"

   // C/C++:
   #include <iostream>



//////////   
// VARS //
//////////

   // Mouse status:
   double oldMouseX, oldMouseY;
   float rotX, rotY;
   bool mouseBR, mouseBL;
   float transZ = -50.0f;

   // Main rendering pipeline:
   Eng::PipelineDefault dfltPipe;



///////////////
// CALLBACKS //
///////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Mouse cursor callback.
 * @param mouseX updated mouse X coordinate
 * @param mouseY updated mouse Y coordinate
 */
void mouseCursorCallback(double mouseX, double mouseY)
{
   // ENG_LOG_DEBUG("x: %.1f, y: %.1f", mouseX, mouseY);
   float deltaY = (float) (mouseX - oldMouseX);
   oldMouseX = mouseX;
   if (mouseBR)
      rotY += deltaY;

   float deltaX = (float) (mouseY - oldMouseY);
   oldMouseY = mouseY;
   if (mouseBR)
      rotX += deltaX;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Mouse button callback.
 * @param button mouse button id
 * @param action action
 * @param mods modifiers
 */
void mouseButtonCallback(int button, int action, int mods)
{
   // ENG_LOG_DEBUG("button: %d, action: %d, mods: %d", button, action, mods);
   switch (button)
   {
      case 0: mouseBL = (bool) action; break;
      case 1: mouseBR = (bool) action; break;
   }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Mouse scroll callback.
 * @param scrollX updated mouse scroll X coordinate
 * @param scrollY updated mouse scroll Y coordinate
 */
void mouseScrollCallback(double scrollX, double scrollY)
{
   // ENG_LOG_DEBUG("x: %.1f, y: %.1f", scrollX, scrollY);
   transZ += (float) scrollY;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Keyboard callback.
 * @param key key code
 * @param scancode key scan code
 * @param action action
 * @param mods modifiers
 */
void keyboardCallback(int key, int scancode, int action, int mods)
{
   // ENG_LOG_DEBUG("key: %d, scancode: %d, action: %d, mods: %d", key, scancode, action, mods);
   switch (key)
   {
      case 'W': if (action == 0) dfltPipe.setWireframe(!dfltPipe.isWireframe()); break;         
   }
}



//////////
// MAIN //
//////////

/**
 * Application entry point.
 * @param argc number of command-line arguments passed
 * @param argv array containing up to argc passed arguments
 * @return error code (0 on success, error code otherwise)
 */
int main(int argc, char *argv[])
{
   // Credits:
   std::cout << "Engine demo, A. Peternier (C) SUPSI" << std::endl;
   std::cout << std::endl;

   // Init engine:
   Eng::Base &eng = Eng::Base::getInstance();
   eng.init();

   // Register callbacks:
   eng.setMouseCursorCallback(mouseCursorCallback);
   eng.setMouseButtonCallback(mouseButtonCallback);
   eng.setMouseScrollCallback(mouseScrollCallback);
   eng.setKeyboardCallback(keyboardCallback);
     

   /////////////////
   // Loading model:   
   Eng::Ovo ovo; 
   std::reference_wrapper<Eng::Node> root = ovo.load("simple3dScene.ovo");
   std::cout << "Scene graph:\n" << root.get().getTreeAsString() << std::endl;
   
   // Get light ref:
   std::reference_wrapper<Eng::Light> light1 = dynamic_cast<Eng::Light &>(Eng::Container::getInstance().find("Omni001"));   
   // light1.get().setMatrix(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 10.0f, 0.0f)));

   std::reference_wrapper<Eng::Light> light2 = dynamic_cast<Eng::Light &>(Eng::Container::getInstance().find("Omni002"));
   light2.get().setColor({ 1.0f, 0.0f, 0.0f });   

   std::reference_wrapper<Eng::Light> light3 = dynamic_cast<Eng::Light &>(Eng::Container::getInstance().find("Omni003"));
   light3.get().setColor({ 0.0f, 0.0f, 1.0f });   

   // Get a material and set its emission:
   std::reference_wrapper<Eng::Material> mtl = dynamic_cast<Eng::Material &>(Eng::Container::getInstance().find("01 - Default"));
   mtl.get().setEmission({ 1.0f, 0.0f, 0.0f });

   // Rendering elements:
   Eng::List list;
   glm::mat4 proj = glm::perspective(glm::radians(45.0f), 1.0f, 1.0f, 1000.0f);      
   
  
   /////////////
   // Main loop:
   std::cout << "Entering main loop..." << std::endl;   
   while (eng.processEvents())
   {      
      // Update viewpoint:
      glm::mat4 camera = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -10.0f, transZ));
      root.get().setMatrix(glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(rotX), { 1.0f, 0.0f, 0.0f }), glm::radians(rotY), { 0.0f, 1.0f, 0.0f }));
      
      // Update list:
      list.reset();
      list.process(root);
      
      // Main rendering:
      eng.clear();      
         dfltPipe.render(camera, proj, list);
      eng.swap();    
   }
   std::cout << "Leaving main loop..." << std::endl;

   // Release engine:
   eng.free();

   // Done:
   std::cout << "[application terminated]" << std::endl;
   return 0;
}
