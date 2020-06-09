#ifndef WRITE_ENTRIES_HPP
#define WRITE_ENTRIES_HPP

#include <vector>
#include <entry.hpp>

template<typename Stream> void write_entries(Stream&& stream, const std::vector<entry>& entries) {
    for(const auto& entry : entries) {
        for(const auto& dof_value : entry.values) {
            stream << dof_value << " ";
        }
    }
}

#endif /* WRITE_ENTRIES_HPP */
