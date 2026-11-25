#ifndef PIPELINES_H
#define PIPELINES_H

#include "../vk_context/vk_context.hpp"
#include "graphics_pipeline.hpp"
#include "compute_pipeline.hpp"

typedef struct Pipeline {
    VkPipeline pipeline{VK_NULL_HANDLE};
    VkPipelineLayout layout{VK_NULL_HANDLE};
} Pipeline;

class Pipelines {
private:
    VulkanContext *vko;
    void setVertexBindingDescription();
	void setVertexAttributeDescriptions();
public:
    VkVertexInputBindingDescription bindingDescs[1];
	VkVertexInputAttributeDescription attrDescs[1];
    Pipeline graphicsPipeline;
    Pipeline raymarchDensityPipeline;
    Pipeline initializationComputePipeline;
    Pipeline applyExternalForcesPipeline;
    Pipeline advectVelocitiesPipeline;
    Pipeline solvePressurePipeline;
    Pipeline applyPressureForcePipeline;
    Pipeline advectSmokePipeline;
    Pipeline persistDataPipeline;

    void init(VulkanContext *vko);

    void createGraphicsPipeline(
        Descriptors *descriptors,
        RenderPass *renderPass,
        VkPipeline *destPipeline,
        VkPipelineLayout *destPipelineLayout,
        const char *vert_path,
        const char *frag_path
    );
    void createComputePipeline(
        Descriptors* descriptors,
        VkPipeline *destPipeline,
        VkPipelineLayout *destPipelineLayout,
        const char *compute_path
    );
    void destroyPipeline(Pipeline &pipeline);
    void destroy();
};
#endif