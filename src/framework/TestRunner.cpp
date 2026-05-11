#include <base/gl/Window.h>
#include <framework/TestRunner.h>
#include <tests/test1/BallsSceneTests.h>
#include <tests/test2/TerrainSceneTests.h>
#include <tests/test3/ShadowMappingSceneTests.h>
#include <tests/test4/InitializationTests.h>

#include <iostream>
#include <stdexcept>
#include <fstream>

namespace {
const float kDefaultTestBenchmarkTime = 15.0f; // 15 seconds
}

namespace framework {
TestRunner::TestRunner(base::ArgumentParser argumentParser)
    : arguments(std::move(argumentParser))
{
}

int TestRunner::run()
{
    const int TESTS = 4;

    auto errorCallback = [&](const std::string& msg) -> int {
        std::cerr << "Invalid usage! " << msg << std::endl;
        std::cerr << "Usage: `" << arguments.getPath() << " -t N -api API [-m] [-benchmark] [-time T] | -all`" << std::endl;
        std::cerr << "  -t N        - test number (in range [1, " << TESTS << "])" << std::endl;
        std::cerr << "  -api API    - API (`gl` or `vk`)" << std::endl;
        std::cerr << "  -m          - run multithreaded version (if exists)" << std::endl;
        std::cerr << "  -benchmark  - run in benchmark mode" << std::endl;
        std::cerr << "  -time T     - change benchmark duraton to T seconds" << std::endl;
        std::cerr << "                default value is 15 seconds" << std::endl;
        std::cerr << "  -all        - run all tests and generate CSV" << std::endl;
        return -1;
    };

    if (!arguments.hasArgument("t"))
        return errorCallback("Missing `-t` argument!");

    if (!arguments.hasArgument("api"))
        return errorCallback("Missing `-api` argument!");

    int testNum = -1;
    try {
        testNum = arguments.getIntArgument("t");
    } catch (...) {
        // ignore, will fail with proper message later
    }

    if (testNum < 1 || testNum > TESTS)
        return errorCallback("Invalid test number!");

    std::string api = arguments.getArgument("api");
    if (api != "gl" && api != "vk")
        return errorCallback("Invalid `-api` value!");

    bool multithreaded = arguments.hasArgument("m");
    bool benchmarkMode = arguments.hasArgument("benchmark");
    float benchmarkTime = kDefaultTestBenchmarkTime;

    if (benchmarkMode && arguments.hasArgument("time")) {
        try {
            benchmarkTime = arguments.getFloatArgument("time");
        } catch (...) {
            std::cerr << "Invalid value for argument '-time'!";
        }
    }

    auto testStartTime = BenchmarkableTest::getCurrentTime();
    if (api == "gl") {
        return run_gl(testNum, multithreaded, benchmarkMode, benchmarkTime, testStartTime);
    } else {
        return run_vk(testNum, multithreaded, benchmarkMode, benchmarkTime, testStartTime);
    }
}

int TestRunner::run_gl(int testNumber, bool multithreaded, bool benchmarkMode, float benchmarkTime, double testStartTime)
{
    std::unique_ptr<BenchmarkableTest> test;

    switch (testNumber) {
    case 1:
        if (multithreaded) {
            test = std::unique_ptr<BenchmarkableTest>(
                new tests::test_gl::MultithreadedBallsSceneTest(benchmarkMode, benchmarkTime));
        } else {
            test = std::unique_ptr<BenchmarkableTest>(
                new tests::test_gl::SimpleBallsSceneTest(benchmarkMode, benchmarkTime));
        }
        break;

    case 2:
        if (multithreaded) {
            test = std::unique_ptr<BenchmarkableTest>(
                new tests::test_gl::MultithreadedTerrainSceneTest(benchmarkMode, benchmarkTime));
        } else {
            test =
                std::unique_ptr<BenchmarkableTest>(new tests::test_gl::TerrainSceneTest(benchmarkMode, benchmarkTime));
        }
        break;

    case 3:
        if (multithreaded) {
            test = std::unique_ptr<BenchmarkableTest>(
                new tests::test_gl::MultithreadedShadowMappingSceneTest(benchmarkMode, benchmarkTime));
        } else {
            test = std::unique_ptr<BenchmarkableTest>(
                new tests::test_gl::ShadowMappingSceneTest(benchmarkMode, benchmarkTime));
        }
        break;
    case 4:
        if (multithreaded) {
            test = std::unique_ptr<BenchmarkableTest>(new tests::test_gl::MultithreadedInitializationTest());
        } else {
            test = std::unique_ptr<BenchmarkableTest>(new tests::test_gl::InitializationTest());
        }
    }

    if (test) {
        return run_any(std::move(test), testStartTime);
    } else {
        std::cerr << "Unknown " << (multithreaded ? "multithreaded" : "") << " OpenGL test: " << testNumber
                  << std::endl;
        return -1;
    }
}

int TestRunner::run_vk(int testNumber, bool multithreaded, bool benchmarkMode, float benchmarkTime, double testStartTime)
{
    std::unique_ptr<BenchmarkableTest> test;

    switch (testNumber) {
    case 1:
        if (multithreaded) {
            test = std::unique_ptr<BenchmarkableTest>(
                new tests::test_vk::MultithreadedBallsSceneTest(benchmarkMode, benchmarkTime));
        } else {
            test = std::unique_ptr<BenchmarkableTest>(
                new tests::test_vk::SimpleBallsSceneTest(benchmarkMode, benchmarkTime));
        }
        break;

    case 2:
        if (multithreaded) {
            test = std::unique_ptr<BenchmarkableTest>(
                new tests::test_vk::MultithreadedTerrainSceneTest(benchmarkMode, benchmarkTime));
        } else {
            test =
                std::unique_ptr<BenchmarkableTest>(new tests::test_vk::TerrainSceneTest(benchmarkMode, benchmarkTime));
        }
        break;

    case 3:
        if (multithreaded) {
            test = std::unique_ptr<BenchmarkableTest>(
                new tests::test_vk::MultithreadedShadowMappingSceneTest(benchmarkMode, benchmarkTime));
        } else {
            test = std::unique_ptr<BenchmarkableTest>(
                new tests::test_vk::ShadowMappingSceneTest(benchmarkMode, benchmarkTime));
        }
        break;
    case 4:
        if (multithreaded) {
            test = std::unique_ptr<BenchmarkableTest>(new tests::test_vk::MultithreadedInitializationTest());
        } else {
            test = std::unique_ptr<BenchmarkableTest>(new tests::test_vk::InitializationTest());
        }
    }

    if (test) {
        return run_any(std::move(test), testStartTime);
    } else {
        std::cerr << "Unknown " << (multithreaded ? "multithreaded" : "") << " Vulkan test: " << testNumber
                  << std::endl;
        return -1;
    }
}

int TestRunner::run_any(std::unique_ptr<BenchmarkableTest> test, double testStartTime)
{
    try {
        test->startMeasuring(testStartTime);
        test->setup();
        test->run();
        test->printStatistics();
        test->teardown();

    } catch (const std::runtime_error& exception) {
        std::cerr << "Caught runtime exception during test execution!" << std::endl;
        std::cerr << exception.what() << std::endl;
        return -1;

    } catch (const std::exception& exception) {
        std::cerr << "Caught exception during test execution!" << std::endl;
        std::cerr << exception.what() << std::endl;
        return -1;

    } catch (...) {
        std::cerr << "Caught unknown exception during test execution!" << std::endl;
        return -1;
    }

    return 0;
}

std::vector<TestResult> TestRunner::runAllTests() {
    std::vector<TestResult> results;

    auto runBenchmarkTest = [&](std::unique_ptr<BenchmarkableTest> test, int testNum, const std::string& api,
                                bool multithreaded, const std::string& description) {
        if (!test)
            return;

        try {
            std::cout << "[runAllTests] Running " << description << std::endl;
            test->startMeasuring(BenchmarkableTest::getCurrentTime());
            test->setup();
            test->run();
            test->teardown();
            results.push_back(TestResult{testNum, api, multithreaded, description, test->getStatistics()});
            std::cout << "[runAllTests] Completed " << description << std::endl;
        } catch (const std::exception& ex) {
            std::cerr << "[runAllTests] " << description << " failed: " << ex.what() << std::endl;
        } catch (...) {
            std::cerr << "[runAllTests] " << description << " failed with unknown exception" << std::endl;
        }
    };

    // Test 1
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_gl::SimpleBallsSceneTest(true, 5.0f)), 1, "GL", false, "[GL] SimpleBallsSceneTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_gl::MultithreadedBallsSceneTest(true, 5.0f)), 1, "GL", true, "[GL] MultithreadedBallsSceneTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_vk::SimpleBallsSceneTest(true, 5.0f)), 1, "VK", false, "[VK] SimpleBallsSceneTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_vk::MultithreadedBallsSceneTest(true, 5.0f)), 1, "VK", true, "[VK] MultithreadedBallsSceneTest");

    // Test 2
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_gl::TerrainSceneTest(true, 5.0f)), 2, "GL", false, "[GL] TerrainSceneTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_gl::MultithreadedTerrainSceneTest(true, 5.0f)), 2, "GL", true, "[GL] MultithreadedTerrainSceneTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_vk::TerrainSceneTest(true, 5.0f)), 2, "VK", false, "[VK] TerrainSceneTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_vk::MultithreadedTerrainSceneTest(true, 5.0f)), 2, "VK", true, "[VK] MultithreadedTerrainSceneTest");

    // Test 3
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_gl::ShadowMappingSceneTest(true, 5.0f)), 3, "GL", false, "[GL] ShadowMappingSceneTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_gl::MultithreadedShadowMappingSceneTest(true, 5.0f)), 3, "GL", true, "[GL] MultithreadedShadowMappingSceneTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_vk::ShadowMappingSceneTest(true, 5.0f)), 3, "VK", false, "[VK] ShadowMappingSceneTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_vk::MultithreadedShadowMappingSceneTest(true, 5.0f)), 3, "VK", true, "[VK] MultithreadedShadowMappingSceneTest");

    // Test 4
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_gl::InitializationTest()), 4, "GL", false, "[GL] InitializationTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_gl::MultithreadedInitializationTest()), 4, "GL", true, "[GL] MultithreadedInitializationTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_vk::InitializationTest()), 4, "VK", false, "[VK] InitializationTest");
    runBenchmarkTest(std::unique_ptr<BenchmarkableTest>(new tests::test_vk::MultithreadedInitializationTest()), 4, "VK", true, "[VK] MultithreadedInitializationTest");

    return results;
}
}
