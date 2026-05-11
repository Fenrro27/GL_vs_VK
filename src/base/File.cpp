#include <base/File.h>

#include <fstream>
#include <iostream>
#include <stdexcept>

namespace base {

bool File::exists(const std::string& path)
{
    bool result;
    std::ifstream file(path.c_str());

    result = file.good();

    return result;
}

static bool openFileStream(std::ifstream& file, const std::string& path)
{
    file.open(path, std::ios::in);
    if (file.is_open())
        return true;

    if (path.rfind("resources/", 0) == 0) {
        file.open(std::string("bin/") + path, std::ios::in);
        return file.is_open();
    }

    return false;
}

static bool openBinaryFileStream(std::ifstream& file, const std::string& path)
{
    file.open(path, std::ios::in | std::ios::binary);
    if (file)
        return true;

    if (path.rfind("resources/", 0) == 0) {
        file.open(std::string("bin/") + path, std::ios::in | std::ios::binary);
        return static_cast<bool>(file);
    }

    return false;
}

std::string File::readText(const std::string& path, bool throwException)
{
    std::string result;
    std::ifstream fileStream;
    if (openFileStream(fileStream, path)) {
        std::string line = "";

        while (std::getline(fileStream, line)) {
            result += line;

            if (fileStream.eof() == false) {
                result += "\n";
            }
        }

        fileStream.close();
    } else {
        std::cerr << "base::File::readText > Couldn't open file: " << path << std::endl;

        if (throwException) {
            std::string errorMsg = "Could not open file: '" + path + "'";
            throw std::runtime_error(errorMsg);
        }
    }
    return result;
}

std::string File::readBinary(const std::string& path, bool throwException)
{
    std::string result;
    std::ifstream file;
    if (openBinaryFileStream(file, path)) {
        file.seekg(0, std::ios::end);
        result.resize(static_cast<std::size_t>(file.tellg()));
        file.seekg(0, std::ios::beg);
        file.read(&result.front(), result.size());
        file.close();
    } else {
        std::cerr << "base::File::readBinary > Couldn't open file: " << path << std::endl;

        if (throwException) {
            throw std::runtime_error("Couldn't open file: '" + path + "'");
        }
    }
    return result;
}

std::vector<uint8_t> File::readBinaryBytes(const std::string& path, bool throwException)
{
    std::vector<uint8_t> result;
    std::ifstream file;
    if (openBinaryFileStream(file, path)) {
        file.seekg(0, std::ios::end);
        result.resize(static_cast<std::size_t>(file.tellg()));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(&result.front()), result.size());
        file.close();
    } else {
        std::cerr << "base::File::readBinaryBytes > Couldn't open file: " << path << std::endl;

        if (throwException) {
            throw std::runtime_error("Couldn't open file: '" + path + "'");
        }
    }
    return result;
}

bool File::writeBinaryBytes(const std::string& path, std::vector<uint8_t> data, bool throwException)
{
    std::ofstream file(path, std::ios::out | std::ios::binary);
    if (!file && path.rfind("resources/", 0) == 0) {
        file.open(std::string("bin/") + path, std::ios::out | std::ios::binary);
    }

    if (file) {
        file.write(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(data.front()));
    } else {
        std::cerr << "base::File::writeBinaryBytes > Couldn't open file: " << path << std::endl;
        if (throwException) {
            throw std::runtime_error("Couldn't open file: '" + path + "'");
        }
        return false;
    }

    return true;
}

std::string File::getPath(const std::string& path)
{
    return path.substr(0, path.find_last_of("/\\") + 1);
}

std::string File::getFilename(const std::string& path)
{
    return path.substr(path.find_last_of("/\\") + 1);
}

std::string File::getFilenameExtensionless(const std::string& path)
{
    std::string fileName = File::getFilename(path);
    return fileName.substr(0, fileName.find_last_of("."));
}

std::string File::getExtension(const std::string& filePath)
{
    return filePath.substr(filePath.find_last_of(".") + 1);
}
}
