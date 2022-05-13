#pragma once

/**
 * @brief Simple forward-rendering pipeline.
 */
class ENG_API PipelineDeferred : public Eng::Pipeline
{
	//////////
public: //
//////////

   // Const/dest:
	PipelineDeferred();
	PipelineDeferred(PipelineDeferred&& other);
	PipelineDeferred(PipelineDeferred const&) = delete;
	virtual ~PipelineDeferred();

	// Get/set:
	const Eng::PipelineShadowMapping& getShadowMappingPipeline() const;
	const Eng::PipelineGeometry& getGeometryPipeline() const;
	void setWireframe(bool flag);
	bool isWireframe() const;

	// Rendering methods:
	// bool render(uint32_t value = 0, void *data = nullptr) const = delete;
	bool render(const Eng::Camera& camera, const Eng::List& list);

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
	PipelineDeferred(const std::string& name);
};