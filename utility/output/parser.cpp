
#include <boost/spirit/include/phoenix.hpp>

#include <boost/spirit/include/karma.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_expect.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <algorithm>
#include <map>

#include <entry_parser.hpp>
#include <read_files.hpp>
#include <read_paths.hpp>
#include <to_step_map.hpp>
#include <write_entries.hpp>



int main(int argc, const char* argv[])
{
    using namespace std;

    auto file = read_files(read_paths(argc, argv));

    auto file_begin = file.cbegin();
    entry_parser<decltype(file_begin)> parser;

    auto all_entries = parser.parse_entries(file_begin, file.cend());

    sort(begin(all_entries), end(all_entries), [](const entry& lhs, const entry& rhs) { return tie(lhs.index, lhs.Dof) < tie(rhs.index, rhs.Dof); });

    auto mapped_entries = to_step_map(std::move(all_entries));
    
    for(const auto& [time_step, entries] : mapped_entries) {
        std::cout << "time_step: " << time_step << "\n";
        write_entries(std::cout, entries);
        std::cout << "\n";
    }

}