#define CATCH_CONFIG_MAIN  // This tells Catch to provide a main() - only do this in one cpp file
#include <catch2/catch.hpp>

#include <entry.hpp>
#include <entry_parser.hpp>
#include <read_files.hpp>
#include <read_paths.hpp>
#include <to_step_map.hpp>
#include <write_entries.hpp>

#include <algorithm>
#include <sstream>

std::string make_entry(const entry& e) {
  std::stringstream stream;
  stream  << "{\n" 
          << "\tindex: " << e.index 
          << "\n\ttime_step: " << e.time_step
          << "\n\tDof: " << e.Dof 
          << "\n\tValue: ";
  
  for(const auto& val : e.values) {
    stream << val << " ";
  }
  stream << "\n}\n";
  return stream.str();
}

std::string make_entries(const std::vector<entry>& entries) {
  std::stringstream stream;
  for(const auto& e : entries) { 
    stream << make_entry(e);
  }
  return stream.str();
};

bool equals(const entry& lhs, const entry& rhs) {

  return  lhs.index == rhs.index 
          && lhs.time_step == rhs.time_step
          && lhs.Dof == rhs.Dof
          && lhs.values.size() == rhs.values.size()
          && std::equal(lhs.values.begin(), lhs.values.end(), rhs.values.begin());

}


TEST_CASE(
  "Parser works"
  "[parser]"
) {

  entry_parser<std::string::const_iterator> parser;

  entry one_value {
    42,
    43,
    44,
    { 45 }
  };

  entry multiple_values {
    42, 
    43,
    44,
    { 45, 46 }
  };

  SECTION("reads entry") {

    auto entries = make_entries({one_value});
    auto result = parser.parse_entries(entries.cbegin(), entries.cend()); 
    REQUIRE(
      equals(result[0], one_value)
    );

  }

  SECTION("reads entry with multiple values") {

    auto entries = make_entries({multiple_values});
    auto result = parser.parse_entries(entries.cbegin(), entries.cend()); 
    REQUIRE(
      equals(result[0], multiple_values)
    );

  }

  SECTION("reads entries") {

    auto entries = make_entries({one_value, multiple_values});
    auto result = parser.parse_entries(entries.cbegin(), entries.cend()); 
    REQUIRE(
      equals(result[0], one_value)
    );

    REQUIRE(
      equals(result[1], multiple_values)
    );

  }

  SECTION("throws when reading bad input") {

    std::string bad_entry = "wrong";
    std::string bad_entries = "{ wrong }";

    REQUIRE_THROWS(parser.parse_entries(bad_entry.cbegin(), bad_entry.cend()));
    REQUIRE_THROWS(parser.parse_entries(bad_entries.cbegin(), bad_entries.cend()));

  }

}

TEST_CASE("read_paths works", "[i/o]") {

  const char * argv[4] = { "program", "path0", "path1", "path2" };
  auto paths = read_paths(4, argv);
  
  REQUIRE(paths[0] == "path0");
  REQUIRE(paths[1] == "path1");
  REQUIRE(paths[2] == "path2");

}

TEST_CASE("read_files works", "[i/o]") {

  for(int i = 0; i < 3; ++i) {
    std::ofstream out( std::string{"test_data_"} + std::to_string(i));
    out << i;
  }
 
  SECTION("works for one file") {
    
    std::vector<std::string_view> path { "test_data_0" };
    auto file = read_files(path);

    REQUIRE(file == "0");

  }

  SECTION("works for multiple files") {
    
    std::vector<std::string_view> paths { "test_data_0", "test_data_1", "test_data_2" };
    auto file = read_files(paths);

    REQUIRE(file == "012");

  }

  SECTION("throws for incorrect path") {

    std::vector<std::string_view> paths { "wroooong" };
    REQUIRE_THROWS(read_files(paths));

  }

}

TEST_CASE("write_entries works", "[i/o]") {

  entry one_value {
    42,
    43,
    44,
    { 45 }
  };

  entry multiple_values {
    42, 
    43,
    44,
    { 45, 46 }
  };

  SECTION("one entry / one value") {
    std::vector<entry> entries { one_value };
    std::stringstream result;

    write_entries(result, entries);

    REQUIRE(result.str() == "45 ");

  }

  SECTION("multiple entries / one value") {
    std::vector<entry> entries { one_value, one_value };
    std::stringstream result;

    write_entries(result, entries);

    REQUIRE(result.str() == "45 45 ");

  }

  SECTION("one entry / multiple values") {
    std::vector<entry> entries { multiple_values };
    std::stringstream result;

    write_entries(result, entries);

    REQUIRE(result.str() == "45 46 ");

  }

  SECTION("multiple entries / multiple values") {
    std::vector<entry> entries { multiple_values, multiple_values };
    std::stringstream result;

    write_entries(result, entries);

    REQUIRE(result.str() == "45 46 45 46 ");

  }

}

TEST_CASE("to_step_map works", "[to_step_map]") {
  //id, time_step
  std::vector<entry> entries {
    { 1, 0 }, 
    { 2, 0 },
    { 1, 1 },
    { 1, 2 }
  };

  auto mapped = to_step_map(std::move(entries));

  REQUIRE(
    mapped[0].size() == 2
  );
  REQUIRE(
    mapped[0][0].index == 1
  );
  REQUIRE(
    mapped[0][1].index == 2
  );

  REQUIRE(
    mapped[1].size() == 1
  );
  REQUIRE(
    mapped[1][0].index == 1
  );

  REQUIRE(
    mapped[2].size() == 1
  );
  REQUIRE(
    mapped[2][0].index == 1
  );

}