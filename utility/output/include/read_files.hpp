#ifndef READ_FILES_HPP
#define READ_FILES_HPP

#include <string>
#include <vector>
#include <fstream>
#include <string_view>

std::string read_files(const std::vector<std::string_view>& paths) {

    std::string res;
    for(const auto& path : paths) {
        std::ifstream in (path.data(), std::ios::in);
        if (in)
        {
            auto res_size = res.size();
            in.seekg(0, std::ios::end);
            auto file_size = in.tellg();
            res.resize(res_size + file_size);
            in.seekg(0, std::ios::beg);
            in.read(&res[res_size], file_size);
        }
        else {
            throw std::invalid_argument(std::string { "could not open file: " }.append(path));
        }
    }
    return res;
} 



#endif /* READ_FILES_HPP */
