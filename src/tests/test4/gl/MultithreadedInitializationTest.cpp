#include <tests/test4/gl/MultithreadedInitializationTest.h>

#include <GL/glew.h>
#include <glm/vec4.hpp>

namespace tests {
namespace test_gl {
MultithreadedInitializationTest::MultithreadedInitializationTest()
    : BaseInitializationTest()
    , GLTest("MultithreadedInitializationTest", true, 0.0f)
{
}

void MultithreadedInitializationTest::setup()
{
    GLTest::setup();

    initApplication();
    initProgram();
    initVBO();
    initVAO();
}

void MultithreadedInitializationTest::run()
{
    glClear(GL_COLOR_BUFFER_BIT);

    program_.use();
    vao_.bind();
    vao_.drawArrays();
    vao_.unbind();
    program_.unbind();

    window_.update();

    glFinish();
    processFrameTime();
}

void MultithreadedInitializationTest::teardown()
{
    GLTest::teardown();
}

void MultithreadedInitializationTest::initApplication()
{
    window_.setDisplayingFPS(true);
    window_.setFPSRefreshRate(1.0);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void MultithreadedInitializationTest::initProgram()
{
    program_.load({"resources/test4/shaders/gl_shader.vert", base::gl::Shader::Type::VertexShader},
                  {"resources/test4/shaders/gl_shader.frag", base::gl::Shader::Type::FragmentShader});
}

void MultithreadedInitializationTest::initVBO()
{
    base::gl::VertexBuffer::Data vertexData;
    vertexData.data = (GLvoid*)(vertices().data());
    vertexData.size = sizeof(glm::vec4) * vertices().size();
    vertexData.pointers.push_back(base::gl::VertexAttrib(0, 4, GL_FLOAT, 0, nullptr));

    vbo_.bind();
    vbo_.setData(vertexData);
    vbo_.unbind();
}

void MultithreadedInitializationTest::initVAO()
{
    vao_.setDrawCount(vertices().size());
    vao_.setDrawTarget(base::gl::VertexArray::DrawTarget::Triangles);

    vao_.bind();
    vao_.attachVBO(&vbo_);
    vao_.setAttribPointers();
    vao_.unbind();
}
}
}
