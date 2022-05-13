/**
 * @file		engine_pipeline_shadowmapping.h
 * @brief	A pipeline for generating planar shadow maps
 *
 * @author	Randolf Schärfig (randolf.schaerfig@supsi.ch), (C) SUPSI
 */
#pragma once



/**
 * @brief Planar shadow mapping pipeline.
 */
class ENG_API PipelineGeometry : public Eng::Pipeline
{
//////////
public: //
//////////

   // Special values:
   constexpr static uint32_t depthTextureSize = 512;     ///< Size of the depth map

   
   // Const/dest:
   PipelineGeometry(/*unsigned short width, unsigned short height*/);
	PipelineGeometry(PipelineGeometry &&other);
   PipelineGeometry(PipelineGeometry const&) = delete;   
   virtual ~PipelineGeometry();

   // Get/set:
   const Eng::Texture& getPositionBuffer() const;
   const Eng::Texture& getNormalBuffer()   const;
   const Eng::Texture& getMaterialBuffer() const;
   const Eng::Texture& getDepthBuffer() const;

   // Rendering methods:
   // bool render(uint32_t value = 0, void *data = nullptr) const = delete;
   bool render(glm::mat4& viewMatrix, const Eng::List &list);
   
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
   PipelineGeometry(const std::string &name);
};






