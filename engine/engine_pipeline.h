/**
 * @file		engine_pipeline.h
 * @brief	Generic rendering pipeline
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */
#pragma once


/**
 * @brief Class for setting up a rendering pipeline: derive and customize at will.
 */
class ENG_API Pipeline : public Eng::Object, public Eng::Managed
{
	//////////
public: //
	//////////

	// Special values:
	static Pipeline empty;

	// Const/dest:
	Pipeline();
	Pipeline(Pipeline&& other);
	Pipeline(Pipeline const&) = delete;
	virtual ~Pipeline();

	// Management:
	bool setProgram(Eng::Program& program);
	Eng::Program& getProgram() const;

	// Rendering methods:
	virtual bool render(const Eng::List& list);


	/////////////
protected: //
	/////////////

	// Reserved:
	struct Reserved;
	std::unique_ptr<Reserved> reserved;

	// Const/dest:
	Pipeline(const std::string& name);
};
