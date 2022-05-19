#pragma once

class ENG_API PipelinePointShadows : public Eng::Pipeline
{

//////////
public: //
//////////
///
	// Special values:
	constexpr static uint32_t depthTextureSize = 1024;     ///< Size of the depth map

	// Const/dest:
	PipelinePointShadows();
	PipelinePointShadows(PipelinePointShadows&& other);
	PipelinePointShadows(PipelinePointShadows const&) = delete;
	virtual ~PipelinePointShadows();

	// Get/set:
	const Eng::Texture& getShadowMap() const;

	// Rendering methods:
	// bool render(uint32_t value = 0, void *data = nullptr) const = delete;
	bool render(const Eng::List::RenderableElem& lightRe, const Eng::List& list);

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

