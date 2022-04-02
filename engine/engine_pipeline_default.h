/**
 * @file		engine_pipeline_default.h
 * @brief	Simple forward-rendering pipeline
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */
#pragma once


/**
 * @brief Simple forward-rendering pipeline.
 */
class ENG_API PipelineDefault : public Eng::Pipeline
{
	//////////
public: //
	//////////

	// Const/dest:
	PipelineDefault();
	PipelineDefault(PipelineDefault&& other);
	PipelineDefault(PipelineDefault const&) = delete;
	virtual ~PipelineDefault();

	// Get/set:
	void setWireframe(bool flag);
	bool isWireframe() const;

	// Rendering methods:
	//bool render(uint32_t value = 0, void *data = nullptr) const = delete;
	bool render(const glm::mat4& camera, const glm::mat4& proj, const Eng::List& list);

	// Managed:
	bool init() override;
	bool free() override;


	/////////////
protected: //
	/////////////

	// Reserved:
	struct Reserved;
	std::unique_ptr<Reserved> reserved;

	// Const/dest:
	PipelineDefault(const std::string& name);
};
