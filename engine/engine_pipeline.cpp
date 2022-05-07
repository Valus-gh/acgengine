/**
 * @file		engine_pipeline.cpp
 * @brief	Generic rendering pipeline
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */


//////////////
// #INCLUDE //
//////////////

// Main include:
#include "engine.h"


////////////
// STATIC //
////////////

   // Special values:
Eng::Pipeline Eng::Pipeline::empty("[empty]");

// Cache:
std::reference_wrapper<Eng::Pipeline> Eng::Pipeline::cache = Eng::Pipeline::empty;

/////////////////////////
// RESERVED STRUCTURES //
/////////////////////////

/**
 * @brief Pipeline reserved structure.
 */
struct Eng::Pipeline::Reserved
{
	std::reference_wrapper<Eng::Program> program; ///< Program of the pipeline


	/**
	 * Constructor. 
	 */
	Reserved() : program{Eng::Program::empty} {}
};


////////////////////////////
// BODY OF CLASS Pipeline //
////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor.
 */
ENG_API Eng::Pipeline::Pipeline() : reserved(std::make_unique<Eng::Pipeline::Reserved>())
{
	ENG_LOG_DETAIL("[+]");
	this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Constructor with name.
 * @param name node name
 */
ENG_API Eng::Pipeline::Pipeline(const std::string& name) : Eng::Object(name),
                                                           reserved(std::make_unique<Eng::Pipeline::Reserved>())
{
	ENG_LOG_DETAIL("[+]");
	this->setProgram(reserved->program);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Move constructor. 
 */
ENG_API Eng::Pipeline::Pipeline(Pipeline&& other) : Eng::Object(std::move(other)), reserved(std::move(other.reserved))
{
	ENG_LOG_DETAIL("[M]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Destructor.
 */
ENG_API Eng::Pipeline::~Pipeline()
{
	ENG_LOG_DETAIL("[-]");
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Set pipeline program. 
 * @param program pipeline program 
 * @return TF
 */
bool ENG_API Eng::Pipeline::setProgram(Eng::Program& program)
{
	reserved->program = program;

	// Done:
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Get pipeline program.  
 * @return pipeline program
 */
Eng::Program ENG_API& Eng::Pipeline::getProgram() const
{
	return reserved->program;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Get the last rendered pipeline.
 * @return last rendered pipeline
 */
Eng::Pipeline ENG_API& Eng::Pipeline::getCached()
{
	return cache;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 * Main rendering method for the pipeline.  
 * @param list list of renderables
 * @return TF
 */
bool ENG_API Eng::Pipeline::render(const Eng::List& list)
{
	// Just update cache:
	Eng::Pipeline::cache = const_cast<Eng::Pipeline&>(*this);
	return true;
}
