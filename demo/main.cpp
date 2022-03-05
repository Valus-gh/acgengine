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
#include <chrono>
#include <thread>


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
	ENG_LOG_DEBUG("x: %.1f, y: %.1f", mouseX, mouseY);

	static auto R = glm::identity<glm::mat4>();
	static auto cOld = mouseX;

	const auto cCurrent = mouseX;

	if (cCurrent != cOld)
	{

		const auto degrees = (cCurrent - cOld > 0.0) ? 1.0f : -1.0f;
		const auto rotate = glm::rotate(
			glm::mat4{ 1.0f },
			glm::radians(degrees),
			glm::vec3{ 0.0f, 1.0f, 0.0f });

		R *= rotate;

		// to_string does not work with ENG_LOG
		std::cout << glm::to_string(R);

		cOld = cCurrent;

	}
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
	ENG_LOG_DEBUG("button: %d, action: %d, mods: %d", button, action, mods);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Mouse scroll callback.
 * @param scrollX updated mouse scroll X coordinate
 * @param scrollY updated mouse scroll Y coordinate
 */
void mouseScrollCallback(double scrollX, double scrollY)
{
	ENG_LOG_DEBUG("x: %.1f, y: %.1f", scrollX, scrollY);
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
	ENG_LOG_DEBUG("key: %d, scancode: %d, action: %d, mods: %d", key, scancode, action, mods);

	static auto T = glm::identity<glm::mat4>();

	if (key >= 262 && key <= 265)
	{
		auto translate = glm::mat4{};

		switch (key)
		{
		case 262:
			translate = glm::translate(glm::mat4{1.0f}, glm::vec3{1.0f, 0.0f, 0.0f});
			T *= translate;
			break;

		case 263:
			translate = glm::translate(glm::mat4{1.0f}, glm::vec3{-1.0f, 0.0f, 0.0f});
			T *= translate;
			break;

		case 264:
			translate = glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 0.0f, -1.0f});
			T *= translate;
			break;

		case 265:
			translate = glm::translate(glm::mat4{1.0f}, glm::vec3{0.0f, 0.0f, 1.0f});
			T *= translate;
			break;
		default: ;
		}

		// to_string does not work with ENG_LOG
		std::cout << glm::to_string(T);
	}
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Function to display FPS value to debug console every >3 seconds
 */
void displayFPS()
{
	static auto tStart = Eng::Timer::getInstance().getCounter();
	static auto frames = Eng::Base::getInstance().getFrameNr();

	auto tCurrent = Eng::Timer::getInstance().getCounter();
	auto tDiff = Eng::Timer::getInstance().getCounterDiff(tStart, tCurrent);

	if (tDiff >= 3000.0)
	{
		tStart = tCurrent;
		auto fDiff = Eng::Base::getInstance().getFrameNr() - frames;
		frames = Eng::Base::getInstance().getFrameNr();

		ENG_LOG_DEBUG("FPS: %.3f", fDiff / (tDiff / 1000.0));
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
int main(int argc, char* argv[])
{
	// Credits:
	std::cout << "Engine demo, A. Peternier (C) SUPSI" << std::endl;
	std::cout << std::endl;

	// Init engine:
	Eng::Base& eng = Eng::Base::getInstance();
	eng.init();

	// Time measurement example:
	Eng::Timer& timer = Eng::Timer::getInstance();
	uint64_t startTime = timer.getCounter();
	std::this_thread::sleep_for(std::chrono::seconds(1));
	uint64_t endTime = timer.getCounter();
	std::cout << "Time elapsed: " << timer.getCounterDiff(startTime, endTime) << std::endl;

	// Register callbacks:
	eng.setMouseCursorCallback(mouseCursorCallback);
	eng.setMouseButtonCallback(mouseButtonCallback);
	eng.setMouseScrollCallback(mouseScrollCallback);
	eng.setKeyboardCallback(keyboardCallback);

	// Main loop:

	std::cout << "Entering main loop..." << std::endl;
	while (eng.processEvents())
	{
		eng.clear();
		eng.swap();

		displayFPS();
	}

	std::cout << "Leaving main loop..." << std::endl;

	// Release engine:
	eng.free();

	// Done:
	std::cout << "[application terminated]" << std::endl;
	return 0;
}
