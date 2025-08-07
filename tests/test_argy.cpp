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

TEST_CASE("Long name only arguments for all types") {
    // String
    {
        const char* argv[] = {"prog", "--filename", "input.txt"};
        int argc = 3;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addString("--filename", "Input file");
        parser.parse();
        CHECK(parser.getString("filename") == "input.txt");
    }
    // Int
    {
        const char* argv[] = {"prog", "--count", "42"};
        int argc = 3;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addInt("--count", "Count");
        parser.parse();
        CHECK(parser.getInt("count") == 42);
    }
    // Bool
    {
        const char* argv[] = {"prog", "--flag"};
        int argc = 2;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addBool("--flag", "A flag", false);
        parser.parse();
        CHECK(parser.getBool("flag") == true);
    }
    // Vector (of strings)
    {
        const char* argv[] = {"prog", "--names", "Alice", "Bob", "Charlie"};
        int argc = 5;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addStrings("--names", "List of names");
        parser.parse();
        auto names = parser.getStrings("names");
        CHECK(names.size() == 3);
        CHECK(names[0] == "Alice");
        CHECK(names[1] == "Bob");
        CHECK(names[2] == "Charlie");
    }
    // Vector (of ints)
    {
        const char* argv[] = {"prog", "--numbers", "1", "2", "3", "4"};
        int argc = 6;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addInts("--numbers", "List of numbers");
        parser.parse();
        auto numbers = parser.getInts("numbers");
        CHECK(numbers.size() == 4);
        CHECK(numbers[0] == 1);
        CHECK(numbers[1] == 2);
        CHECK(numbers[2] == 3);
        CHECK(numbers[3] == 4);
    }
    // Vector (of floats)
    {
        const char* argv[] = {"prog", "--values", "1.1", "2.2", "3.3"};
        int argc = 5;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addFloats("--values", "List of floats");
        parser.parse();
        auto values = parser.getFloats("values");
        CHECK(values.size() == 3);
        CHECK(values[0] == doctest::Approx(1.1));
        CHECK(values[1] == doctest::Approx(2.2));
        CHECK(values[2] == doctest::Approx(3.3));
    }
}

TEST_CASE("Long name only arguments with default values and override") {
    // String
    {
        const char* argv[] = {"prog", "--filename", "input.txt"};
        int argc = 3;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addString("--filename", "Input file", "default.txt");
        parser.parse();
        CHECK(parser.getString("filename") == "input.txt");
    }
    // Int
    {
        const char* argv[] = {"prog", "--count", "42"};
        int argc = 3;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addInt("--count", "Count", 99);
        parser.parse();
        CHECK(parser.getInt("count") == 42);
    }
    // Bool
    {
        const char* argv[] = {"prog", "--flag"};
        int argc = 2;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addBool("--flag", "A flag", false);
        parser.parse();
        CHECK(parser.getBool("flag") == true);
    }
    // Vector (of strings)
    {
        const char* argv[] = {"prog", "--names", "Alice", "Bob", "Charlie"};
        int argc = 5;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addStrings("--names", "List of names", std::vector<std::string>{"Default"});
        parser.parse();
        auto names = parser.getStrings("names");
        CHECK(names.size() == 3);
        CHECK(names[0] == "Alice");
        CHECK(names[1] == "Bob");
        CHECK(names[2] == "Charlie");
    }
    // Vector (of ints)
    {
        const char* argv[] = {"prog", "--numbers", "1", "2", "3", "4"};
        int argc = 6;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addInts("--numbers", "List of numbers", std::vector<int>{99, 100});
        parser.parse();
        auto numbers = parser.getInts("numbers");
        CHECK(numbers.size() == 4);
        CHECK(numbers[0] == 1);
        CHECK(numbers[1] == 2);
        CHECK(numbers[2] == 3);
        CHECK(numbers[3] == 4);
    }
    // Vector (of floats)
    {
        const char* argv[] = {"prog", "--values", "1.1", "2.2", "3.3"};
        int argc = 5;
        Parser parser(argc, const_cast<char**>(argv));
        parser.addFloats("--values", "List of floats", std::vector<float>{9.9f, 8.8f});
        parser.parse();
        auto values = parser.getFloats("values");
        CHECK(values.size() == 3);
        CHECK(values[0] == doctest::Approx(1.1));
        CHECK(values[1] == doctest::Approx(2.2));
        CHECK(values[2] == doctest::Approx(3.3));
    }
}
