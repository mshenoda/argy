#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "argy.hpp"
#include <doctest.h>
#include <vector>
#include <string>

using namespace Argy;

TEST_CASE("Template: Basic positional and optional arguments") {
    const char* argv[] = {"prog", "input.txt", "42", "--count", "7"};
    int argc = 5;
    Parser parser(argc, const_cast<char**>(argv));
    parser.add<std::string>("filename", "Input file");
    parser.add<int>("number", "A number");
    parser.add<int>("-c", "count", "Count", 10);
    parser.parse();
    CHECK(parser.get<std::string>("filename") == "input.txt");
    CHECK(parser.get<int>("number") == 42);
    CHECK(parser.get<int>("count") == 7);
}

TEST_CASE("Template: Default values and required arguments") {
    const char* argv[] = {"prog", "foo.txt"};
    int argc = 2;
    Parser parser(argc, const_cast<char**>(argv));
    parser.add<std::string>("filename", "Input file");
    parser.add<int>("-c", "count", "Count", 99);
    parser.parse();
    CHECK(parser.get<std::string>("filename") == "foo.txt");
    CHECK(parser.get<int>("count") == 99);
}

TEST_CASE("Template: Bool arguments") {
    const char* argv[] = {"prog", "input.txt", "--flag"};
    int argc = 3;
    Parser parser(argc, const_cast<char**>(argv));
    parser.add<std::string>("filename", "Input file");
    parser.add<bool>("-f", "flag", "A flag", false);
    parser.parse();
    CHECK(parser.get<bool>("flag") == true);
}

TEST_CASE("Template: Vector arguments") {
    const char* argv[] = {"prog", "--names", "Alice", "Bob", "Charlie"};
    int argc = 5;
    Parser parser(argc, const_cast<char**>(argv));
    parser.add<std::vector<std::string>>("-n", "names", "List of names");
    parser.parse();
    auto names = parser.get<std::vector<std::string>>("names");
    CHECK(names.size() == 3);
    CHECK(names[0] == "Alice");
    CHECK(names[1] == "Bob");
    CHECK(names[2] == "Charlie");
}

TEST_CASE("Basic positional and optional arguments") {
    const char* argv[] = {"prog", "input.txt", "42", "--count", "7"};
    int argc = 5;
    Parser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addInt("number", "A number");
    parser.addInt("-c", "count", "Count", 10);
    parser.parse();
    CHECK(parser.getString("filename") == "input.txt");
    CHECK(parser.getInt("number") == 42);
    CHECK(parser.getInt("count") == 7);
}

TEST_CASE("Default values and required arguments") {
    const char* argv[] = {"prog", "foo.txt"};
    int argc = 2;
    Parser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addInt("-c", "count", "Count", 99);
    parser.parse();
    CHECK(parser.getString("filename") == "foo.txt");
    CHECK(parser.getInt("count") == 99);
}

TEST_CASE("Bool arguments") {
    const char* argv[] = {"prog", "input.txt", "--flag"};
    int argc = 3;
    Parser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addBool("-f", "flag", "A flag", false);
    parser.parse();
    CHECK(parser.getBool("flag") == true);
}

TEST_CASE("Vector arguments") {
    const char* argv[] = {"prog", "--names", "Alice", "Bob", "Charlie"};
    int argc = 5;
    Parser parser(argc, const_cast<char**>(argv));
    parser.addStrings("-n", "names", "List of names");
    parser.parse();
    auto names = parser.getStrings("names");
    CHECK(names.size() == 3);
    CHECK(names[0] == "Alice");
    CHECK(names[1] == "Bob");
    CHECK(names[2] == "Charlie");
}

TEST_CASE("Help handler") {
    const char* argv[] = {"prog", "--help"};
    int argc = 2;
    Parser parser(argc, const_cast<char**>(argv));
    bool helpCalled = false;
    parser.setHelpHandler([&](std::string){ helpCalled = true; });
    parser.addString("filename", "Input file");
    parser.parse();
    CHECK(helpCalled == true);
}

TEST_CASE("Missing required argument throws") {
    const char* argv[] = {"prog"};
    int argc = 1;
    Parser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    CHECK_THROWS_AS(parser.parse(), std::runtime_error);
}

TEST_CASE("Unknown argument throws") {
    const char* argv[] = {"prog", "input.txt", "--unknown"};
    int argc = 3;
    Parser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    CHECK_THROWS_AS(parser.parse(), std::runtime_error);
}

TEST_CASE("Type validation") {
    const char* argv[] = {"prog", "input.txt", "notanint"};
    int argc = 3;
    Parser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addInt("number", "A number");
    CHECK_THROWS_AS(parser.parse(), std::invalid_argument);
}
