#pragma once

#include "../vk_context/vk_context.hpp"
#include "buffers.hpp"

class Images {
private:
	VulkanContext *vko;
public:
	void createImage(
		VkImageType imageType,
		uint32_t width,
		uint32_t length,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage *image,
		VkDeviceMemory *imageMemory
	);
	void transitionImageLayout(
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout
	);
	void createImageView(
		VkImage image,
		VkImageViewType imageViewType,
		VkFormat format,
		VkImageAspectFlags aspectFlags,
		VkImageView *imageView
	);
	// interpolate = VK_LINEAR, no interpolation = VK_NEAREST
	void createSampler(VkSampler *pSampler, VkFilter filter);

	void init(VulkanContext *vko);
	void destroy();
};
