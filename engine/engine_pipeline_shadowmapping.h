/**
 * @file		engine_pipeline_shadowmapping.h
 * @brief	A pipeline for generating planar shadow maps
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */
#pragma once



/**
 * @brief Planar shadow mapping pipeline.
 */
class ENG_API PipelineShadowMapping : public Eng::Pipeline
{
//////////
public: //
//////////

   // Special values:
   constexpr static uint32_t depthTextureSize = 1024;     ///< Size of the depth map

   // Const/dest:
   PipelineShadowMapping();      
   PipelineShadowMapping(PipelineShadowMapping &&other);
   PipelineShadowMapping(PipelineShadowMapping const&) = delete;   
   virtual ~PipelineShadowMapping(); 

   // Get/set:
   const Eng::Texture &getShadowMap() const;

   // Rendering methods:
   // bool render(uint32_t value = 0, void *data = nullptr) const = delete;
   bool render(const Eng::List::RenderableElem &lightRe, const Eng::List &list);
   
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
   PipelineShadowMapping(const std::string &name);
};






