#pragma once

#include <base/ArgumentParser.h>
#include <framework/BenchmarkableTest.h>

#include <memory>
#include <string>
#include <vector>

namespace framework {
struct TestResult {
    int testNum;
    std::string api;
    bool multithreaded;
    std::string testName;
    BenchmarkableTest::Statistics stats;
};

class TestRunner
{
  public:
    TestRunner(base::ArgumentParser argumentParser);

    int run();
    std::vector<TestResult> runAllTests();

  private:
    int run_gl(int testNumber, bool multithreaded, bool benchmarkMode, float benchmarkTime, double testStartTime);
    int run_vk(int testNumber, bool multithreaded, bool benchmarkMode, float benchmarkTime, double testStartTime);
    int run_any(std::unique_ptr<BenchmarkableTest> test, double testStartTime);

    base::ArgumentParser arguments;
};
}
