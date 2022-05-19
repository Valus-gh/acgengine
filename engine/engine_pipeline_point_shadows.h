#pragma once

class ENG_API PipelinePointShadows : public Eng::Pipeline
{

	//////////
public: //
	//////////

	// Const/dest:
	PipelinePointShadows();
	PipelinePointShadows(PipelinePointShadows&& other);
	PipelinePointShadows(PipelinePointShadows const&) = delete;
	virtual ~PipelinePointShadows();

	// Get/set:
	const Eng::PipelineCubemap& getShadowMappingPipeline() const;
	void setWireframe(bool flag);
	bool isWireframe() const;

	// Rendering methods:
	//bool render(uint32_t value = 0, void *data = nullptr) const = delete;
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
	PipelinePointShadows(const std::string& name);

};

