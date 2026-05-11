#include <tests/test3/gl/MultithreadedShadowMappingSceneTest.h>

#include <GL/glew.h>
#include <glm/vec4.hpp>

#include <stdexcept>

namespace tests {
namespace test_gl {
MultithreadedShadowMappingSceneTest::MultithreadedShadowMappingSceneTest(bool benchmarkMode, float benchmarkTime)
    : BaseShadowMappingSceneTest()
    , GLTest("MultithreadedShadowMappingSceneTest", benchmarkMode, benchmarkTime)
{
}

void MultithreadedShadowMappingSceneTest::setup()
{
    GLTest::setup();

    glEnable(GL_DEPTH_TEST);

    initApplication();
    initShadowmapObjects();
    initPrograms();
    initRenderObjects();
}

void MultithreadedShadowMappingSceneTest::run()
{
    while (!window_.shouldClose()) {
        updateTestState(window_.getFrameTime());

        {
            setupShadowStage();
            render(_shadowProgram, shadowMatrix());

            setupRenderStage();
            render(_renderProgram, renderMatrix());
        }

        window_.update();

        if (processFrameTime(window_.getFrameTime())) {
            break;
        }
    }
}

void MultithreadedShadowMappingSceneTest::teardown()
{
    GLTest::teardown();
}

void MultithreadedShadowMappingSceneTest::initApplication()
{
    window_.setDisplayingFPS(true);
    window_.setFPSRefreshRate(1.0);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
}

void MultithreadedShadowMappingSceneTest::initShadowmapObjects()
{
    glGenFramebuffers(1, &_shadowmapFramebuffer);

    glGenTextures(1, &_shadowmapTexture);
    glBindTexture(GL_TEXTURE_2D, _shadowmapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, shadowmapSize().x, shadowmapSize().y, 0, GL_DEPTH_COMPONENT,
                 GL_FLOAT, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, _shadowmapFramebuffer);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _shadowmapTexture, 0);
    glDrawBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error{"Shadowmap framebuffer initialization failed!"};
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void MultithreadedShadowMappingSceneTest::initPrograms()
{
    _shadowProgram.load({"resources/test3/shaders/gl/shadow.vert", base::gl::Shader::Type::VertexShader},
                        {"resources/test3/shaders/gl/shadow.frag", base::gl::Shader::Type::FragmentShader});

    _renderProgram.load({"resources/test3/shaders/gl/render.vert", base::gl::Shader::Type::VertexShader},
                        {"resources/test3/shaders/gl/render.frag", base::gl::Shader::Type::FragmentShader});
}

void MultithreadedShadowMappingSceneTest::initRenderObjects()
{
    _glRenderObjects.resize(renderObjects().size());

    for (std::size_t index = 0; index < renderObjects().size(); ++index) {
        const common::RenderObject& renderObject = renderObjects()[index];
        GLRenderObject& glRenderObject = _glRenderObjects[index];

        glRenderObject.modelMatrix = renderObject.modelMatrix;
        initVBO(renderObject, glRenderObject);
        initVAO(renderObject, glRenderObject);
    }
}

void MultithreadedShadowMappingSceneTest::initVBO(const common::RenderObject& renderObject, GLRenderObject& glRenderObject)
{
    std::vector<glm::vec4> vboBuffer = renderObject.generateCombinedData();

    base::gl::VertexBuffer::Data vertexData;
    vertexData.data = (GLvoid*)(vboBuffer.data());
    vertexData.size = sizeof(glm::vec4) * vboBuffer.size();
    vertexData.pointers.push_back(base::gl::VertexAttrib{0, 4, GL_FLOAT, 3 * sizeof(glm::vec4), nullptr});
    vertexData.pointers.push_back(base::gl::VertexAttrib{1, 4, GL_FLOAT, 3 * sizeof(glm::vec4), sizeof(glm::vec4)});
    vertexData.pointers.push_back(base::gl::VertexAttrib{2, 4, GL_FLOAT, 3 * sizeof(glm::vec4), 2 * sizeof(glm::vec4)});

    glRenderObject.vbo.bind();
    glRenderObject.vbo.setData(vertexData);
    glRenderObject.vbo.unbind();
}

void MultithreadedShadowMappingSceneTest::initVAO(const common::RenderObject& renderObject, GLRenderObject& glRenderObject)
{
    glRenderObject.vao.setDrawTarget(base::gl::VertexArray::DrawTarget::Triangles);
    glRenderObject.vao.setDrawCount(renderObject.vertices.size());

    glRenderObject.vao.bind();
    glRenderObject.vao.attachVBO(&glRenderObject.vbo);
    glRenderObject.vao.setAttribPointers();
    glRenderObject.vao.unbind();
}

void MultithreadedShadowMappingSceneTest::setupShadowStage()
{
    glBindFramebuffer(GL_FRAMEBUFFER, _shadowmapFramebuffer);
    glViewport(0, 0, shadowmapSize().x, shadowmapSize().y);
    glClear(GL_DEPTH_BUFFER_BIT);

    _shadowProgram.use();
}

void MultithreadedShadowMappingSceneTest::setupRenderStage()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, 800, 600);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glBindTexture(GL_TEXTURE_2D, _shadowmapTexture);

    _renderProgram.use();
}

void MultithreadedShadowMappingSceneTest::render(const base::gl::Program& currentProgram, const glm::mat4& viewProjectionmatrix)
{
    for (const auto& renderObject : _glRenderObjects) {
        glm::mat4 MVP = viewProjectionmatrix * renderObject.modelMatrix;
        currentProgram["MVP"] = MVP;

        try {
            glm::mat4 depthMVP = convertProjectionToImage(shadowMatrix() * renderObject.modelMatrix);
            currentProgram["depthMVP"] = depthMVP;
        } catch (...) {
        }

        renderObject.vao.bind();
        renderObject.vao.drawArrays();
    }
}

glm::mat4 MultithreadedShadowMappingSceneTest::convertProjectionToImage(const glm::mat4& matrix) const
{
    static const glm::mat4 bias = {0.5, 0.0, 0.0, 0.0,
                                   0.0, 0.5, 0.0, 0.0,
                                   0.0, 0.0, 0.5, 0.0,
                                   0.5, 0.5, 0.5, 1.0};

    return bias * matrix;
}
}
}
