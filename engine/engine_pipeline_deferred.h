#pragma once

class ENG_API PipelineDeferred : public Eng::Pipeline
{
public:

	// Const/dest:
	PipelineDeferred();
	PipelineDeferred(PipelineDeferred&& other);
	PipelineDeferred(PipelineDeferred const&) = delete;
	virtual ~PipelineDeferred();

	// Get/set:
	const Eng::Texture& getPositionBuffer() const;
	const Eng::Texture& getNormalBuffer()   const;
	const Eng::Texture& getMaterialBuffer() const;
	const Eng::Texture& getDepthBuffer() const;
	const Eng::PipelineShadowMapping& getShadowMappingPipeline() const;
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
	PipelineDeferred(const std::string& name);

};

