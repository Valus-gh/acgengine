
/**
 * @file		engine_pipeline_fullscreen2d.h
 * @brief	A pipeline for rendering a texture fullscreen in 2D
 *
 * @author	Achille Peternier (achille.peternier@supsi.ch), (C) SUPSI
 */
#pragma once



/**
 * @brief Fullscreen rendering 2D.
 */
class ENG_API PipelineFullscreenLighting : public Eng::Pipeline
{
//////////
public: //
//////////
   
   // Const/dest:
   PipelineFullscreenLighting();
   PipelineFullscreenLighting(PipelineFullscreenLighting &&other);
   PipelineFullscreenLighting(PipelineFullscreenLighting const&) = delete;
   virtual ~PipelineFullscreenLighting();

   // Rendering methods:
   // bool render(uint32_t value = 0, void *data = nullptr) const = delete;
   bool render(const Eng::PipelineGeometry& geometries, const Eng::PipelineShadowMapping& shadowmap, Eng::Light& light, const Eng::List &list);
   
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
   PipelineFullscreenLighting(const std::string &name);
};
