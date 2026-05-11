#include <base/ArgumentParser.h>
#include <framework/TestRunner.h>

#include <iostream>
#include <fstream>

int main(int argc, char* argv[])
{
    try {
        base::ArgumentParser argParser{argc, argv};

        if (argParser.hasArgument("all")) {
            framework::TestRunner runner{std::move(argParser)};
            auto results = runner.runAllTests();
            std::ofstream csv("results.csv");
            if (!csv) {
                std::cerr << "Failed to open results.csv" << std::endl;
                return -1;
            }
            csv << "Test,API,Multithreaded,TestName,MinFrameTime,MaxFrameTime,AvgFrameTime,MaxFPS,MinFPS,AvgFPS\n";
            for (const auto& r : results) {
                csv << r.testNum << "," << r.api << "," << (r.multithreaded ? "Yes" : "No") << ",\"" << r.testName << "\","
                    << r.stats.minFrameTime << "," << r.stats.maxFrameTime << "," << r.stats.avgFrameTime << ","
                    << r.stats.maxFPS << "," << r.stats.minFPS << "," << r.stats.avgFPS << "\n";
            }
            std::cout << "Results written to results.csv" << std::endl;
        } else {
            framework::TestRunner testRunner{std::move(argParser)};
            testRunner.run();
        }

    } catch (const std::system_error& systemError) {
        std::cerr << "Caught std::system_error!" << std::endl;
        std::cerr << "  Code:    " << systemError.code() << std::endl;
        std::cerr << "  Message: " << systemError.what() << std::endl;

    } catch (const std::exception& exception) {
        std::cerr << "Caught std::exception!" << std::endl;
        std::cerr << "  Message: " << exception.what() << std::endl;
        return -1;

    } catch (...) {
        std::cerr << "Caught unknown exception!" << std::endl;
        return -1;
    }

    return 0;
}
