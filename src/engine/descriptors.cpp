#include "descriptors.hpp"

void Descriptors::destroy() {
    vkDestroyDescriptorPool(vko->device.device, descriptorPool, NULL);
    vkDestroyDescriptorSetLayout(vko->device.device, descriptorSetLayout, NULL);
}

void Descriptors::init(VulkanContext *vko, Buffers *buffers, Images *images, Resources *resources) {
    this->vko = vko;
    this->buffers = buffers;
    this->images = images;
    this->resources = resources;
    createUniformBuffer();
    createDescriptorSetResources();
    createDescriptorSets();
}

void Descriptors::addDescriptorSetLayoutBinding(
    std::vector<VkDescriptorSetLayoutBinding> &bindings,
    std::vector<VkDescriptorPoolSize> &poolSizes,
    uint32_t binding,
    VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags,
    const VkSampler *pImmutableSampler
) {
    VkDescriptorSetLayoutBinding layoutBinding{
        .binding = binding,
        .descriptorType = descriptorType,
        .descriptorCount = 1, // for now its always 1
        .stageFlags = stageFlags,
        .pImmutableSamplers = pImmutableSampler
    };

    VkDescriptorPoolSize poolSize{
        .type = descriptorType,
        .descriptorCount = 2 // for now its always 2
    };

    bindings.push_back(layoutBinding);
    poolSizes.push_back(poolSize);
}

void Descriptors::createDescriptorSetResources() {
    std::vector<VkDescriptorSetLayoutBinding> bindings;
    std::vector<VkDescriptorPoolSize> poolSizes;

    // uniform buffer
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        0,
        VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // storage image
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        1,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        NULL
    );

    // density image READ
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        2,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // density image WRITE
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        3,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // x velocities READ
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        4,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // y velocities READ
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        5,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // z velocities READ
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        6,
        VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // x velocities WRITE
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        7,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // y velocities WRITE
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        8,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // z velocities WRITE
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        9,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // linear sampler
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        10,
        VK_DESCRIPTOR_TYPE_SAMPLER,
        VK_SHADER_STAGE_COMPUTE_BIT,
        &resources->linearSampler
    );

    // solid map
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        11,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // pressure map READ
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        12,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // pressure map WRITE
    addDescriptorSetLayoutBinding(
        bindings,
        poolSizes,
        13,
        VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        VK_SHADER_STAGE_COMPUTE_BIT,
        NULL
    );

    // create descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pBindings = bindings.data();
    layoutInfo.bindingCount = bindings.size();

    // create descriptor pool
    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.maxSets = 2;

    if (vkCreateDescriptorSetLayout(vko->device.device, &layoutInfo, NULL, &descriptorSetLayout) != VK_SUCCESS) {
        printf("Failed to create descriptor set layout\n");
        exit(1);
    }

    if (vkCreateDescriptorPool(vko->device.device, &poolInfo, NULL, &descriptorPool) != VK_SUCCESS) {
        printf("failed to create descriptor pool\n");
        exit(1);
    }
}

void Descriptors::createUniformBuffer() {
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);
    
    // create one for each image in flight (ie uniform buffer count)
    for (int i = 0; i < 2; i++) {
        buffers->createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &buffers->uniformBuffers[i],
            &buffers->uniformBufferMemories[i]
        );
        vkMapMemory(vko->device.device, buffers->uniformBufferMemories[i], 0, bufferSize, 0, &buffers->uniformBuffersMapped[i]);
    }
}

void Descriptors::createDescriptorSets() {
    VkDescriptorSetAllocateInfo allocInfo{};

    // need identical layouts for both descriptor sets
    VkDescriptorSetLayout layouts[2] = {0};
    for (int i = 0; i < 2; i++) layouts[i] = descriptorSetLayout;

    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = (uint32_t) 2;
    allocInfo.pSetLayouts = layouts;

    if (vkAllocateDescriptorSets(vko->device.device, &allocInfo, descriptorSets) != VK_SUCCESS) {
        fprintf(stderr, "Failed to create descriptor sets\n");
        exit(1);
    }

    // populate descriptors
    for (int ping = 0; ping < 2; ping++) {
        // add buffer to descriptor set
        VkDescriptorBufferInfo bufferInfo = {0};
        bufferInfo.buffer = buffers->uniformBuffers[ping];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkDescriptorImageInfo uniformImageInfo = resources->storageImage.imageInfo();

        int READ = ping;
        int WRITE = 1 - ping;

        VkDescriptorImageInfo densityImageREADInfo = resources->densityImages[READ].imageInfo();
        VkDescriptorImageInfo densityImageWRITEInfo = resources->densityImages[WRITE].imageInfo();

        VkDescriptorImageInfo vxREADInfo = resources->xVelocitiesImages[READ].imageInfo();
        VkDescriptorImageInfo vyREADInfo = resources->yVelocitiesImages[READ].imageInfo();
        VkDescriptorImageInfo vzREADInfo = resources->zVelocitiesImages[READ].imageInfo();

        VkDescriptorImageInfo vxWRITEInfo = resources->xVelocitiesImages[WRITE].imageInfo();
        VkDescriptorImageInfo vyWRITEInfo = resources->yVelocitiesImages[WRITE].imageInfo();
        VkDescriptorImageInfo vzWRITEInfo = resources->zVelocitiesImages[WRITE].imageInfo();

        VkDescriptorImageInfo solidMapImageInfo = resources->solidMap.imageInfo();

        VkDescriptorImageInfo pressureMapImageREADInfo = resources->pressureMapImages[READ].imageInfo();
        VkDescriptorImageInfo pressureMapImageWRITEInfo = resources->pressureMapImages[WRITE].imageInfo();

        // write to descriptor set
        std::vector<VkWriteDescriptorSet> descriptorWrites;

        // uniform buffer
        addWriteBufferDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            0,
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            &bufferInfo
        );

        // storage image
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            1,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            &uniformImageInfo
        );

        // density image READ
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            2,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            &densityImageREADInfo
        );

        // density image WRITE
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            3,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            &densityImageWRITEInfo
        );

        // x velocities READ
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            4,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            &vxREADInfo
        );

        // y velocities READ
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            5,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            &vyREADInfo
        );
        
        // z velocities READ
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            6,
            VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            &vzREADInfo
        );

        // x velocities WRITE
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            7,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            &vxWRITEInfo
        );

        // y velocities WRITE
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            8,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            &vyWRITEInfo
        );
        
        // z velocities WRITE
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            9,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            &vzWRITEInfo
        );

        // binding 10 is reserved for the linear sampler

        // solid map
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            11,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            &solidMapImageInfo
        );

        // pressure map READ
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            12,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            &pressureMapImageREADInfo
        );

        // pressure map WRITE
        addWriteImageDescriptorSet(
            descriptorWrites,
            descriptorSets[ping],
            13,
            VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
            &pressureMapImageWRITEInfo
        );

        vkUpdateDescriptorSets(vko->device.device, descriptorWrites.size(), descriptorWrites.data(), 0, NULL);
    }
}

void Descriptors::addWriteBufferDescriptorSet(
    std::vector<VkWriteDescriptorSet> &descriptorWrites,
    VkDescriptorSet descriptorSet,
    uint32_t dstBinding,
    VkDescriptorType descriptorType,
    VkDescriptorBufferInfo *pBufferInfo
) {
    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = dstBinding,
        .descriptorCount = 1,
        .descriptorType = descriptorType,
        .pBufferInfo = pBufferInfo
    };

    descriptorWrites.push_back(write);
}

void Descriptors::addWriteImageDescriptorSet(
    std::vector<VkWriteDescriptorSet> &descriptorWrites,
    VkDescriptorSet descriptorSet,
    uint32_t dstBinding,
    VkDescriptorType descriptorType,
    VkDescriptorImageInfo *pImageInfo
) {
    VkWriteDescriptorSet write{
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = descriptorSet,
        .dstBinding = dstBinding,
        .descriptorCount = 1,
        .descriptorType = descriptorType,
        .pImageInfo = pImageInfo
    };

    descriptorWrites.push_back(write);
}
