#include <tests/test4/vk/MultithreadedInitializationTest.h>

#include <base/File.h>
#include <base/ScopedTimer.h>

#include <glm/vec4.hpp>
#include <vulkan/vulkan.hpp>

#include <array>
#include <stdexcept>
#include <vector>
#include <thread>

namespace {
const std::string kPipelineCacheDataPath = "resources/test4/vk_pipeline_cache.bin.tmp";
}

namespace tests {
namespace test_vk {
MultithreadedInitializationTest::MultithreadedInitializationTest()
    : BaseInitializationTest()
    , VKTest("MultithreadedInitializationTest", true, 0.0f)
{
}

void MultithreadedInitializationTest::setup()
{
    VKTest::setup();

    // In this "multithreaded" initialization, we just use a separate thread 
    // for some part of the setup to justify the test name.
    std::thread setupThread([&]() {
        createVbo();
        createShaders();
        createPipelineCache();
    });

    createCommandBuffers();
    createSemaphores();
    createFences();
    createRenderPass();
    createFramebuffers();
    createPipelineLayout();

    setupThread.join();

    createPipeline();
}

void MultithreadedInitializationTest::run()
{
    TIME_RESET("Frame time:");

    auto frameIndex = getNextFrameIndex();
    prepareCommandBuffer(frameIndex);
    submitCommandBuffer(frameIndex);
    presentFrame(frameIndex);

    window().update();

    queues().queue().waitIdle();
    processFrameTime();
}

void MultithreadedInitializationTest::teardown()
{
    device().waitIdle();

    destroyPipeline();
    destroyPipelineCache();
    destroyPipelineLayout();
    destroyShaders();
    destroyFramebuffers();
    destroyRenderPass();
    destroyFences();
    destroySemaphores();
    destroyVbo();
    destroyCommandBuffers();

    VKTest::teardown();
}

void MultithreadedInitializationTest::createCommandBuffers()
{
    vk::CommandPoolCreateFlags cmdPoolFlags = vk::CommandPoolCreateFlagBits::eResetCommandBuffer;
    _cmdPool = device().createCommandPool({cmdPoolFlags, queues().familyIndex()});
    _cmdBuffers = device().allocateCommandBuffers(
        {_cmdPool, vk::CommandBufferLevel::ePrimary, static_cast<uint32_t>(window().swapchainImages().size())});
    _uploadCmdBuffer = device().allocateCommandBuffers({_cmdPool, vk::CommandBufferLevel::ePrimary, 1}).front();
}

void MultithreadedInitializationTest::createVbo()
{
    vk::DeviceSize size = vertices().size() * sizeof(vertices().front());
    vk::BufferUsageFlags usage = vk::BufferUsageFlagBits::eVertexBuffer;

    _stagingBuffer = memory().createStagingBuffer(size);
    {
        auto vboMemory = device().mapMemory(_stagingBuffer.memory, _stagingBuffer.offset, _stagingBuffer.size, {});
        std::memcpy(vboMemory, vertices().data(), static_cast<std::size_t>(_stagingBuffer.size));
        device().unmapMemory(_stagingBuffer.memory);
    }
    _vbo = memory().copyToDeviceLocalMemoryAsync(_stagingBuffer, usage, _uploadCmdBuffer, queues().queue(),
                                                 _uploadSemaphore);
}

void MultithreadedInitializationTest::createSemaphores()
{
    _acquireSemaphore = device().createSemaphore({});
    _renderSemaphore = device().createSemaphore({});
    _uploadSemaphore = device().createSemaphore({});
}

void MultithreadedInitializationTest::createFences()
{
    _fences.resize(_cmdBuffers.size());
    for (vk::Fence& fence : _fences) {
        fence = device().createFence({vk::FenceCreateFlagBits::eSignaled});
    }
}

void MultithreadedInitializationTest::createRenderPass()
{
    vk::AttachmentDescription attachment{{},
                                         window().swapchainImageFormat(),
                                         vk::SampleCountFlagBits::e1,
                                         vk::AttachmentLoadOp::eClear,
                                         vk::AttachmentStoreOp::eStore,
                                         vk::AttachmentLoadOp::eDontCare,
                                         vk::AttachmentStoreOp::eDontCare,
                                         vk::ImageLayout::eUndefined,
                                         vk::ImageLayout::ePresentSrcKHR};
    vk::AttachmentReference colorAttachment{0, vk::ImageLayout::eColorAttachmentOptimal};
    vk::SubpassDescription subpassDesc{
        {}, vk::PipelineBindPoint::eGraphics, 0, nullptr, 1, &colorAttachment, nullptr, nullptr, 0, nullptr};

    vk::RenderPassCreateInfo renderPassInfo{{}, 1, &attachment, 1, &subpassDesc, 0, nullptr};
    _renderPass = device().createRenderPass(renderPassInfo);
}

void MultithreadedInitializationTest::createFramebuffers()
{
    _framebuffers.reserve(window().swapchainImages().size());
    for (const vk::ImageView& imageView : window().swapchainImageViews()) {
        vk::FramebufferCreateInfo framebufferInfo{{}, _renderPass, 1, &imageView, window().size().x, window().size().y,
                                                  1};
        _framebuffers.push_back(device().createFramebuffer(framebufferInfo));
    }
}

void MultithreadedInitializationTest::createShaders()
{
    _vertexModule = base::vkx::ShaderModule{device(), "resources/test4/shaders/vk_shader.vert.spv"};
    _fragmentModule = base::vkx::ShaderModule{device(), "resources/test4/shaders/vk_shader.frag.spv"};
}

void MultithreadedInitializationTest::createPipelineLayout()
{
    std::vector<vk::DescriptorSetLayoutBinding> bindings;
    vk::DescriptorSetLayoutCreateInfo setLayoutInfo{{}, static_cast<uint32_t>(bindings.size()), bindings.data()};
    _setLayout = device().createDescriptorSetLayout(setLayoutInfo);

    vk::PushConstantRange pushConstantRanges[] = {
        {vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::vec4)}, // position
        {vk::ShaderStageFlagBits::eFragment, sizeof(glm::vec4), sizeof(glm::vec4)} // color
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{{}, 1, &_setLayout, 2, &pushConstantRanges[0]};
    _pipelineLayout = device().createPipelineLayout(pipelineLayoutInfo);
}

void MultithreadedInitializationTest::createPipelineCache()
{
    std::vector<uint8_t> initialData = base::File::readBinaryBytes(kPipelineCacheDataPath, false);
    vk::PipelineCacheCreateInfo pipelineCacheInfo{{}, initialData.size(), initialData.data()};
    _pipelineCache = device().createPipelineCache(pipelineCacheInfo);
}

void MultithreadedInitializationTest::createPipeline()
{
    TIME_IT("Pipeline creation");

    std::vector<vk::PipelineShaderStageCreateInfo> shaderStages = getShaderStages();

    std::vector<vk::VertexInputBindingDescription> vertexBindingDescriptions{
        {0, sizeof(glm::vec4), vk::VertexInputRate::eVertex}
    };
    std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescription{
        {0, 0, vk::Format::eR32G32B32A32Sfloat, 0}
    };
    vk::PipelineVertexInputStateCreateInfo vertexInputState{{},
                                                            static_cast<uint32_t>(vertexBindingDescriptions.size()),
                                                            vertexBindingDescriptions.data(),
                                                            static_cast<uint32_t>(vertexAttributeDescription.size()),
                                                            vertexAttributeDescription.data()};

    vk::PipelineInputAssemblyStateCreateInfo inputAssemblyState{{}, vk::PrimitiveTopology::eTriangleList, VK_FALSE};

    vk::Viewport viewport{0.0f, 0.0f, static_cast<float>(window().size().x), static_cast<float>(window().size().y),
                          0.0f, 1.0f};
    vk::Rect2D scissor{{0, 0}, {static_cast<uint32_t>(window().size().x), static_cast<uint32_t>(window().size().y)}};
    vk::PipelineViewportStateCreateInfo viewportState{{}, 1, &viewport, 1, &scissor};

    vk::PipelineRasterizationStateCreateInfo rasterizationState{{},
                                                                VK_FALSE,
                                                                VK_FALSE,
                                                                vk::PolygonMode::eFill,
                                                                vk::CullModeFlagBits::eNone,
                                                                vk::FrontFace::eCounterClockwise,
                                                                VK_FALSE,
                                                                0.0f,
                                                                0.0f,
                                                                0.0f,
                                                                1.0f};

    vk::PipelineMultisampleStateCreateInfo multisampleState{
        {}, vk::SampleCountFlagBits::e1, VK_FALSE, 0.0f, nullptr, VK_FALSE, VK_FALSE};

    vk::PipelineColorBlendAttachmentState colorBlendAttachmentState{VK_FALSE};
    colorBlendAttachmentState.colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG |
                                               vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;
    vk::PipelineColorBlendStateCreateInfo colorBlendState{
        {}, VK_FALSE, vk::LogicOp::eClear, 1, &colorBlendAttachmentState};

    vk::GraphicsPipelineCreateInfo pipelineInfo{{},
                                                 static_cast<uint32_t>(shaderStages.size()),
                                                 shaderStages.data(),
                                                 &vertexInputState,
                                                 &inputAssemblyState,
                                                 nullptr,
                                                 &viewportState,
                                                 &rasterizationState,
                                                 &multisampleState,
                                                 nullptr,
                                                 &colorBlendState,
                                                 nullptr,
                                                 _pipelineLayout,
                                                 _renderPass,
                                                 0};
    _pipeline = device().createGraphicsPipeline(_pipelineCache, pipelineInfo).value;
}

void MultithreadedInitializationTest::destroyPipeline()
{
    device().destroyPipeline(_pipeline);
}

void MultithreadedInitializationTest::destroyPipelineCache()
{
    std::vector<uint8_t> pipelineCacheData = device().getPipelineCacheData(_pipelineCache);
    base::File::writeBinaryBytes(kPipelineCacheDataPath, pipelineCacheData, false);
    device().destroyPipelineCache(_pipelineCache);
}

void MultithreadedInitializationTest::destroyPipelineLayout()
{
    device().destroyPipelineLayout(_pipelineLayout);
    device().destroyDescriptorSetLayout(_setLayout);
}

void MultithreadedInitializationTest::destroyShaders()
{
    _vertexModule = {};
    _fragmentModule = {};
}

void MultithreadedInitializationTest::destroyFramebuffers()
{
    for (const vk::Framebuffer& framebuffer : _framebuffers) {
        device().destroyFramebuffer(framebuffer);
    }
}

void MultithreadedInitializationTest::destroyRenderPass()
{
    device().destroyRenderPass(_renderPass);
}

void MultithreadedInitializationTest::destroyFences()
{
    for (const vk::Fence& fence : _fences) {
        device().destroyFence(fence);
    }
    _fences.clear();
}

void MultithreadedInitializationTest::destroySemaphores()
{
    device().destroySemaphore(_uploadSemaphore);
    device().destroySemaphore(_acquireSemaphore);
    device().destroySemaphore(_renderSemaphore);
}

void MultithreadedInitializationTest::destroyVbo()
{
    memory().destroyBuffer(_stagingBuffer);
    memory().destroyBuffer(_vbo);
}

void MultithreadedInitializationTest::destroyCommandBuffers()
{
    device().destroyCommandPool(_cmdPool);
    _cmdBuffers.clear();
    _uploadCmdBuffer = vk::CommandBuffer{};
}

std::vector<vk::PipelineShaderStageCreateInfo> MultithreadedInitializationTest::getShaderStages() const
{
    std::vector<vk::PipelineShaderStageCreateInfo> stages;
    vk::PipelineShaderStageCreateInfo vertexStage{{}, vk::ShaderStageFlagBits::eVertex, _vertexModule, "main", nullptr};
    stages.push_back(vertexStage);
    vk::PipelineShaderStageCreateInfo fragmentStage{{}, vk::ShaderStageFlagBits::eFragment, _fragmentModule, "main", nullptr};
    stages.push_back(fragmentStage);
    return stages;
}

uint32_t MultithreadedInitializationTest::getNextFrameIndex() const
{
    auto nextFrameAcquireStatus = device().acquireNextImageKHR(window().swapchain(), UINT64_MAX, _acquireSemaphore, {});
    return nextFrameAcquireStatus.value;
}

void MultithreadedInitializationTest::prepareCommandBuffer(std::size_t frameIndex) const
{
    static const vk::ClearValue clearValue = vk::ClearColorValue{std::array<float, 4>{{0.0f, 0.0f, 0.0f, 1.0f}}};
    const vk::CommandBuffer& cmdBuffer = _cmdBuffers[frameIndex];
    vk::RenderPassBeginInfo renderPassInfo{_renderPass, _framebuffers[frameIndex], {{}, {window().size().x, window().size().y}}, 1, &clearValue};
    device().waitForFences(1, &_fences[frameIndex], VK_FALSE, UINT64_MAX);
    device().resetFences(1, &_fences[frameIndex]);
    cmdBuffer.reset({});
    cmdBuffer.begin({vk::CommandBufferUsageFlagBits::eOneTimeSubmit, nullptr});
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, _pipeline);
    cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
    cmdBuffer.bindVertexBuffers(0, {{_vbo.buffer}}, {{0}});
    cmdBuffer.draw(static_cast<uint32_t>(vertices().size()), 1, 0, 0);
    cmdBuffer.endRenderPass();
    cmdBuffer.end();
}

void MultithreadedInitializationTest::submitCommandBuffer(std::size_t frameIndex) const
{
    vk::PipelineStageFlags waitStages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eVertexInput};
    static bool first_sent = false;
    if (!first_sent) {
        vk::Semaphore waitSemaphores[2] = {_acquireSemaphore, _uploadSemaphore};
        vk::SubmitInfo submits{2, waitSemaphores, waitStages, 1, &_cmdBuffers[frameIndex], 1, &_renderSemaphore};
        queues().queue().submit(submits, _fences[frameIndex]);
        first_sent = true;
    } else {
        vk::SubmitInfo submits{1, &_acquireSemaphore, waitStages, 1, &_cmdBuffers[frameIndex], 1, &_renderSemaphore};
        queues().queue().submit(submits, _fences[frameIndex]);
    }
}

void MultithreadedInitializationTest::presentFrame(std::size_t frameIndex) const
{
    uint32_t imageIndex = static_cast<uint32_t>(frameIndex);
    vk::PresentInfoKHR presentInfo{1, &_renderSemaphore, 1, &window().swapchain(), &imageIndex, nullptr};
    queues().queue().presentKHR(presentInfo);
}
}
}
