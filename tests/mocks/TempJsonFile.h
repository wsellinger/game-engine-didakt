#pragma once

#include <filesystem>
#include <fstream>
#include <string>

class TempJsonFile
{
public:
    TempJsonFile(const std::string& content, const std::string& filename = "didakt_test.json")
    {
        _path = std::filesystem::temp_directory_path() / filename;
        std::ofstream file(_path);
        file << content;
    }

    ~TempJsonFile()
    {
        std::filesystem::remove(_path);
    }

    std::string PathString() const { return _path.string(); }

private:
    std::filesystem::path _path;
};