#include <tests/test1/gl/MultithreadedBallsSceneTest.h>

#include <base/ScopedTimer.h>

#include <iostream>
#include <GL/glew.h>
#include <glm/vec4.hpp>

#include <thread>
#include <vector>

namespace tests {
namespace test_gl {
MultithreadedBallsSceneTest::MultithreadedBallsSceneTest(bool benchmarkMode, float benchmarkTime)
    : BaseBallsSceneTest()
    , GLTest("MultithreadedBallsSceneTest", benchmarkMode, benchmarkTime)
{
}

void MultithreadedBallsSceneTest::setup()
{
    std::cout << "[MultithreadedBallsSceneTest] setup start" << std::endl;
    GLTest::setup();
    initTestState();

    initApplication();
    initProgram();
    initVBO();
    initVAO();
    std::cout << "[MultithreadedBallsSceneTest] setup complete" << std::endl;
}

void MultithreadedBallsSceneTest::run()
{
    std::cout << "[MultithreadedBallsSceneTest] run start" << std::endl;
    while (!window_.shouldClose()) {
        glClear(GL_COLOR_BUFFER_BIT);

        updateStateMultithreaded();

        program_.use();
        vao_.bind();

        for (const auto& ball : balls()) {
            program_["objPosition"] = ball.position;
            program_["objColor"] = ball.color;

            vao_.drawArrays();
        }

        vao_.unbind();
        program_.unbind();

        window_.update();

        if (processFrameTime(window_.getFrameTime())) {
            break; // Benchmarking is complete
        }
    }
}

void MultithreadedBallsSceneTest::teardown()
{
    destroyTestState();
    GLTest::teardown();
}

void MultithreadedBallsSceneTest::initApplication()
{
    window_.setDisplayingFPS(true);
    window_.setFPSRefreshRate(1.0);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void MultithreadedBallsSceneTest::initProgram()
{
    program_.load({"resources/test1/shaders/gl_shader.vert", base::gl::Shader::Type::VertexShader},
                  {"resources/test1/shaders/gl_shader.frag", base::gl::Shader::Type::FragmentShader});
}

void MultithreadedBallsSceneTest::initVBO()
{
    // VertexData
    base::gl::VertexBuffer::Data vertexData;
    vertexData.data = (GLvoid*)(vertices().data());
    vertexData.size = sizeof(glm::vec4) * vertices().size();
    vertexData.pointers.push_back(base::gl::VertexAttrib(0, 4, GL_FLOAT, 0, nullptr));

    // VBO settings
    vbo_.bind();
    vbo_.setData(vertexData);
    vbo_.unbind();
}

void MultithreadedBallsSceneTest::initVAO()
{
    vao_.setDrawCount(vertices().size());
    vao_.setDrawTarget(base::gl::VertexArray::DrawTarget::Triangles);

    vao_.bind();
    vao_.attachVBO(&vbo_);
    vao_.setAttribPointers();
    vao_.unbind();
}

void MultithreadedBallsSceneTest::updateStateMultithreaded()
{
    std::size_t threadCount = std::thread::hardware_concurrency();
    if (threadCount == 0) {
        threadCount = 1u;
    }

    std::vector<std::thread> threads(threadCount);
    for (std::size_t threadIndex = 0; threadIndex < threadCount; ++threadIndex) {
        std::size_t k = balls().size() / threadCount;
        std::size_t rangeFrom = threadIndex * k;
        std::size_t rangeTo = (threadIndex + 1) * k;

        if (threadIndex + 1 == threadCount) {
            rangeTo = balls().size();
        }

        threads[threadIndex] =
            std::move(std::thread(&MultithreadedBallsSceneTest::updatePartialState, this, rangeFrom, rangeTo));
    }

    for (auto& thread : threads) {
        thread.join();
    }
}

void MultithreadedBallsSceneTest::updatePartialState(std::size_t rangeFrom, std::size_t rangeTo)
{
    TIME_IT("Partial state update");

    updateTestState(static_cast<float>(window_.getFrameTime()), rangeFrom, rangeTo);
}
}
}
