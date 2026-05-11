#pragma once

#include <base/vkx/ShaderModule.h>
#include <framework/VKTest.h>
#include <tests/test4/BaseInitializationTest.h>

namespace tests {
namespace test_vk {
class MultithreadedInitializationTest : public BaseInitializationTest, public framework::VKTest
{
  public:
    MultithreadedInitializationTest();

    void setup() override;
    void run() override;
    void teardown() override;

  private:
    void createCommandBuffers();
    void createVbo();
    void createSemaphores();
    void createFences();
    void createRenderPass();
    void createFramebuffers();
    void createShaders();
    void createPipelineLayout();
    void createPipelineCache();
    void createPipeline();

    void destroyPipeline();
    void destroyPipelineCache();
    void destroyPipelineLayout();
    void destroyShaders();
    void destroyFramebuffers();
    void destroyRenderPass();
    void destroyFences();
    void destroySemaphores();
    void destroyVbo();
    void destroyCommandBuffers();

    std::vector<vk::PipelineShaderStageCreateInfo> getShaderStages() const;
    uint32_t getNextFrameIndex() const;
    void prepareCommandBuffer(std::size_t frameIndex) const;
    void submitCommandBuffer(std::size_t frameIndex) const;
    void presentFrame(std::size_t frameIndex) const;

    vk::CommandPool _cmdPool;
    std::vector<vk::CommandBuffer> _cmdBuffers;
    vk::CommandBuffer _uploadCmdBuffer;

    vk::Semaphore _acquireSemaphore;
    vk::Semaphore _renderSemaphore;
    vk::Semaphore _uploadSemaphore;

    std::vector<vk::Fence> _fences;

    base::vkx::Buffer _vbo;
    base::vkx::Buffer _stagingBuffer;

    vk::RenderPass _renderPass;
    std::vector<vk::Framebuffer> _framebuffers;

    base::vkx::ShaderModule _vertexModule;
    base::vkx::ShaderModule _fragmentModule;

    vk::DescriptorSetLayout _setLayout;
    vk::PipelineLayout _pipelineLayout;

    vk::PipelineCache _pipelineCache;
    vk::Pipeline _pipeline;
};
}
}
