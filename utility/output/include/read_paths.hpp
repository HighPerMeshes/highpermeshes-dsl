#ifndef READ_PATHS_HPP
#define READ_PATHS_HPP

#include <vector>
#include <string_view>

std::vector<std::string_view> read_paths(int argc, const char* argv[]) {
    std::vector<std::string_view> paths(argc - 1);
    for(int i =1; i < argc; ++i) {
        paths[i-1] = argv[i];
    }
    return paths;
}

#endif /* READ_PATHS_HPP */
