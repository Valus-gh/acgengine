/**
 * @file		engine_pipeline_raytracing.h
 * @brief	A pipeline for doing simple ray tracing on GPU
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */
#pragma once


/**
 * @brief Basic ray tracing.
 */
class ENG_API PipelineRayTracing : public Eng::Pipeline
{
	//////////
public: //
	//////////

	/**
	 * Per-triangle data. This struct must be aligned for OpenGL std430.
	 */
	__declspec(align(16)) struct TriangleStruct
	{
		glm::vec4 v[3];
		glm::vec4 n[3];
		uint32_t matId;
		uint32_t _pad[3];
	};


	/**
	 * Per-light data. This struct must be aligned for OpenGL std430.
	 */
	__declspec(align(16)) struct LightStruct
	{
		glm::vec4 position;
		glm::vec4 color;
	};


	/**
	 * Bounding sphere data. This struct must be aligned for OpenGL std430.
	 */
	__declspec(align(16)) struct BSphereStruct
	{
		glm::vec4 position;
		float radius;
		uint32_t firstTriangle;
		uint32_t nrOfTriangles;
		uint32_t _pad;
	};


	// Const/dest:
	PipelineRayTracing();
	PipelineRayTracing(PipelineRayTracing&& other);
	PipelineRayTracing(PipelineRayTracing const&) = delete;
	virtual ~PipelineRayTracing();

	// Get/set:
	const Eng::Texture& getColorBuffer() const;

	// Data preparation:
	bool migrate(const Eng::List& list);

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
	PipelineRayTracing(const std::string& name);
};
