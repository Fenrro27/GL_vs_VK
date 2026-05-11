#include <tests/test2/gl/MultithreadedTerrainSceneTest.h>

#include <GL/glew.h>
#include <glm/vec4.hpp>

namespace tests {
namespace test_gl {
MultithreadedTerrainSceneTest::MultithreadedTerrainSceneTest(bool benchmarkMode, float benchmarkTime)
    : BaseTerrainSceneTest()
    , GLTest("MultithreadedTerrainSceneTest", benchmarkMode, benchmarkTime)
    , _ibo(base::gl::Buffer::Target::ElementArray, base::gl::Buffer::Usage::StaticDraw)
{
}

void MultithreadedTerrainSceneTest::setup()
{
    GLTest::setup();

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    initApplication();
    initProgram();
    initVBO();
    initIBO();
    initVAO();
}

void MultithreadedTerrainSceneTest::run()
{
    while (!window_.shouldClose()) {
        glClear(GL_COLOR_BUFFER_BIT);
        _program.use();
        _vao.bind();
        _ibo.bind(base::gl::Buffer::Target::ElementArray);

        _program["MVP"] = currentMVP();
        {
            auto renderChunk = [](std::size_t count, std::ptrdiff_t offset) {
                glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, (const GLvoid*)offset);
            };
            terrain().executeLoD(currentPosition(), renderChunk);
        }

        _ibo.unbind();
        _vao.unbind();
        _program.unbind();

        window_.update();
        updateTestState(window_.getFrameTime());

        if (processFrameTime(window_.getFrameTime())) {
            break;
        }
    }
}

void MultithreadedTerrainSceneTest::teardown()
{
    GLTest::teardown();
}

void MultithreadedTerrainSceneTest::initApplication()
{
    window_.setDisplayingFPS(true);
    window_.setFPSRefreshRate(1.0);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void MultithreadedTerrainSceneTest::initProgram()
{
    _program.load({"resources/test2/shaders/gl_shader.vert", base::gl::Shader::Type::VertexShader},
                  {"resources/test2/shaders/gl_shader.frag", base::gl::Shader::Type::FragmentShader});
}

void MultithreadedTerrainSceneTest::initVBO()
{
    base::gl::VertexBuffer::Data vertexData;
    vertexData.data = (GLvoid*)(terrain().vertices().data());
    vertexData.size = sizeof(glm::vec4) * terrain().vertices().size();
    vertexData.pointers.push_back(base::gl::VertexAttrib(0, 4, GL_FLOAT, 0, nullptr));

    _vbo.bind();
    _vbo.setData(vertexData);
    _vbo.unbind();
}

void MultithreadedTerrainSceneTest::initIBO()
{
    _ibo.bind(base::gl::Buffer::Target::ElementArray);
    _ibo.setData(terrain().indices());
    _ibo.unbind();
}

void MultithreadedTerrainSceneTest::initVAO()
{
    _vao.setDrawTarget(base::gl::VertexArray::DrawTarget::Triangles);

    _vao.bind();
    _vao.attachVBO(&_vbo);
    _vao.setAttribPointers();
    _vao.unbind();
}
}
}
