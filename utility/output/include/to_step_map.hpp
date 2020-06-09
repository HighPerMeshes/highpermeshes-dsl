#ifndef TO_STEP_MAP_HPP
#define TO_STEP_MAP_HPP

#include <entry.hpp>

#include <vector>
#include <map>

auto to_step_map(std::vector<entry>&& entries) {
    using namespace std;
    map<size_t, vector<entry>> mapped_entries;
    for(auto&& entry : entries) {
        mapped_entries[entry.time_step].emplace_back(move(entry));
    }
    return mapped_entries; 
}

#endif /* TO_STEP_MAP_HPP */
