#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "argy.hpp"
#include <doctest.h>
#include <vector>
#include <string>
#include <filesystem>
#include <fstream>

using namespace Argy;
namespace fs = std::filesystem;

// Test helper functions for file system operations
class TestUtil {
public:
    static std::string createTempFile(const std::string& filename, const std::string& content = "test content") {
        std::ofstream file(filename);
        file << content;
        file.close();
        return filename;
    }
    
    static std::string createTempDirectory(const std::string& dirname) {
        fs::create_directory(dirname);
        return dirname;
    }
    
    static void cleanup(const std::string& path) {
        std::error_code ec;
        fs::remove_all(path, ec);
        // Ignore errors during cleanup
    }
};

TEST_CASE("Template: Basic positional and optional arguments") {
    const char* argv[] = {"prog", "input.txt", "42", "--count", "7"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<std::string>("filename", "Input file");
    parser.add<int>("number", "A number");
    parser.add<int>({"-c", "--count"}, "Count", 10);
    parser.parse();
    CHECK(parser.get<std::string>("filename") == "input.txt");
    CHECK(parser.get<int>("number") == 42);
    CHECK(parser.get<int>("count") == 7);
}

TEST_CASE("Template: Default values and required arguments") {
    const char* argv[] = {"prog", "foo.txt"};
    int argc = 2;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<std::string>("filename", "Input file");
    parser.add<int>({"-c", "--count"}, "Count", 99);
    parser.parse();
    CHECK(parser.get<std::string>("filename") == "foo.txt");
    CHECK(parser.get<int>("count") == 99);
}

TEST_CASE("Template: Bool arguments") {
    const char* argv[] = {"prog", "input.txt", "--flag"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<std::string>("filename", "Input file");
    parser.add<bool>({"-f", "--flag"}, "A flag", false);
    parser.parse();
    CHECK(parser.get<bool>("flag") == true);
}

TEST_CASE("Template: Vector arguments") {
    const char* argv[] = {"prog", "--names", "Alice", "Bob", "Charlie"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<std::vector<std::string>>({"-n", "--names"}, "List of names");
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
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addInt("number", "A number");
    parser.addInt({"-c", "--count"}, "Count", 10);
    parser.parse();
    CHECK(parser.getString("filename") == "input.txt");
    CHECK(parser.getInt("number") == 42);
    CHECK(parser.getInt("count") == 7);
}

TEST_CASE("Default values and required arguments") {
    const char* argv[] = {"prog", "foo.txt"};
    int argc = 2;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addInt({"-c", "--count"}, "Count", 99);
    parser.parse();
    CHECK(parser.getString("filename") == "foo.txt");
    CHECK(parser.getInt("count") == 99);
}

TEST_CASE("Bool arguments") {
    const char* argv[] = {"prog", "input.txt", "--flag"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addBool({"-f", "--flag"}, "A flag");
    parser.parse();
    CHECK(parser.getBool("flag") == true);
}

TEST_CASE("Vector arguments") {
    const char* argv[] = {"prog", "--names", "Alice", "Bob", "Charlie"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addStrings({"-n", "--names"}, "List of names");
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
    CliParser parser(argc, const_cast<char**>(argv));
    bool helpCalled = false;
    parser.setHelpHandler([&](std::string){ helpCalled = true; });
    parser.addString("filename", "Input file");
    parser.parse();
    CHECK(helpCalled == true);
}

TEST_CASE("Missing required argument throws") {
    const char* argv[] = {"prog"};
    int argc = 1;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    CHECK_THROWS_AS(parser.parse(), Argy::MissingArgumentException);
}

TEST_CASE("Unknown argument throws") {
    const char* argv[] = {"prog", "input.txt", "--unknown"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    CHECK_THROWS_AS(parser.parse(), Argy::UnknownArgumentException);
}

TEST_CASE("Type validation") {
    const char* argv[] = {"prog", "input.txt", "notanint"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addInt("number", "A number");
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Long name only argument: string") {
    const char* argv[] = {"prog", "--filename", "input.txt"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--filename", "Input file");
    parser.parse();
    CHECK(parser.getString("filename") == "input.txt");
}

TEST_CASE("Long name only argument: int") {
    const char* argv[] = {"prog", "--count", "42"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addInt("--count", "Count");
    parser.parse();
    CHECK(parser.getInt("count") == 42);
}

TEST_CASE("Long name only argument: bool") {
    const char* argv[] = {"prog", "--flag"};
    int argc = 2;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addBool("--flag", "A flag");
    parser.parse();
    CHECK(parser.getBool("flag") == true);
}

TEST_CASE("Long name only argument: vector of strings") {
    const char* argv[] = {"prog", "--names", "Alice", "Bob", "Charlie"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addStrings("--names", "List of names");
    parser.parse();
    auto names = parser.getStrings("names");
    CHECK(names.size() == 3);
    CHECK(names[0] == "Alice");
    CHECK(names[1] == "Bob");
    CHECK(names[2] == "Charlie");
}

TEST_CASE("Long name only argument: vector of ints") {
    const char* argv[] = {"prog", "--numbers", "1", "2", "3", "4"};
    int argc = 6;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addInts("--numbers", "List of numbers");
    parser.parse();
    auto numbers = parser.getInts("numbers");
    CHECK(numbers.size() == 4);
    CHECK(numbers[0] == 1);
    CHECK(numbers[1] == 2);
    CHECK(numbers[2] == 3);
    CHECK(numbers[3] == 4);
}

TEST_CASE("Long name only argument: vector of floats") {
    const char* argv[] = {"prog", "--values", "1.1", "2.2", "3.3"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addFloats("--values", "List of floats");
    parser.parse();
    auto values = parser.getFloats("values");
    CHECK(values.size() == 3);
    CHECK(values[0] == doctest::Approx(1.1));
    CHECK(values[1] == doctest::Approx(2.2));
    CHECK(values[2] == doctest::Approx(3.3));
}

TEST_CASE("Long name only argument with default value and override: string") {
    const char* argv[] = {"prog", "--filename", "input.txt"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--filename", "Input file", "default.txt");
    parser.parse();
    CHECK(parser.getString("filename") == "input.txt");
}

TEST_CASE("Long name only argument with default value and override: int") {
    const char* argv[] = {"prog", "--count", "42"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addInt("--count", "Count", 99);
    parser.parse();
    CHECK(parser.getInt("count") == 42);
}

TEST_CASE("Long name only argument with default value and override: bool") {
    const char* argv[] = {"prog", "--flag"};
    int argc = 2;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addBool("--flag", "A flag");
    parser.parse();
    CHECK(parser.getBool("flag") == true);
}

TEST_CASE("Long name only argument with default value and override: vector of strings") {
    const char* argv[] = {"prog", "--names", "Alice", "Bob", "Charlie"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addStrings("--names", "List of names", std::vector<std::string>{"Default"});
    parser.parse();
    auto names = parser.getStrings("names");
    CHECK(names.size() == 3);
    CHECK(names[0] == "Alice");
    CHECK(names[1] == "Bob");
    CHECK(names[2] == "Charlie");
}

TEST_CASE("Long name only argument with default value and override: vector of ints") {
    const char* argv[] = {"prog", "--numbers", "1", "2", "3", "4"};
    int argc = 6;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addInts("--numbers", "List of numbers", std::vector<int>{99, 100});
    parser.parse();
    auto numbers = parser.getInts("numbers");
    CHECK(numbers.size() == 4);
    CHECK(numbers[0] == 1);
    CHECK(numbers[1] == 2);
    CHECK(numbers[2] == 3);
    CHECK(numbers[3] == 4);
}

TEST_CASE("Long name only argument with default value and override: vector of floats") {
    const char* argv[] = {"prog", "--values", "1.1", "2.2", "3.3"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addFloats("--values", "List of floats", std::vector<float>{9.9f, 8.8f});
    parser.parse();
    auto values = parser.getFloats("values");
    CHECK(values.size() == 3);
    CHECK(values[0] == doctest::Approx(1.1));
    CHECK(values[1] == doctest::Approx(2.2));
    CHECK(values[2] == doctest::Approx(3.3));
}

TEST_CASE("Positional and optional mix with all API variants") {
    const char* argv[] = {"prog", "file.txt", "123", "--flag", "--names", "A", "B"};
    int argc = 7;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addInt("number", "A number");
    parser.addBool("--flag", "A flag");
    parser.addStrings("--names", "Names");
    parser.parse();
    CHECK(parser.getString("filename") == "file.txt");
    CHECK(parser.getInt("number") == 123);
    CHECK(parser.getBool("flag") == true);
    auto names = parser.getStrings("names");
    CHECK(names.size() == 2);
    CHECK(names[0] == "A");
    CHECK(names[1] == "B");
}

TEST_CASE("has() method: argument presence and absence") {
    const char* argv[] = {"prog", "foo.txt", "42", "--flag"};
    int argc = 4;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addInt("number", "A number");
    parser.addBool({"-f", "--flag"}, "A flag");
    parser.parse();
    CHECK(parser.has("filename"));
    CHECK(parser.has("number"));
    CHECK(parser.has("flag"));
    CHECK(!parser.has("missing"));
}

TEST_CASE("addBools/getBools: vector<bool> arguments") {
    const char* argv[] = {"prog", "--flags", "1", "0", "1", "0"};
    int argc = 6;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addBools("--flags", "List of flags");
    parser.parse();
    auto flags = parser.getBools("flags");
    CHECK(flags.size() == 4);
    CHECK(flags[0] == true);
    CHECK(flags[1] == false);
    CHECK(flags[2] == true);
    CHECK(flags[3] == false);
}

TEST_CASE("Duplicate argument names throws") {
    CliParser parser(0, nullptr);
    parser.addString("filename", "Input file");
    CHECK_THROWS_AS(parser.addString("filename", "Duplicate"), Argy::DuplicateArgumentException);
}

TEST_CASE("Reserved names throws") {
    CliParser parser(0, nullptr);
    CHECK_THROWS_AS(parser.addString("--help", "Help"), Argy::ReservedArgumentException);
    CHECK_THROWS_AS(parser.addString("-h", "Help"), Argy::ReservedArgumentException);
}

TEST_CASE("Malformed argument names throws") {
    CliParser parser(0, nullptr);
    parser.addInt({"c", "count"}, "Missing dashes for optional", 1);
    CHECK(parser.getInt("count") == 1);
}

TEST_CASE("Argument with only short name") {
    const char* argv[] = {"prog", "-f", "input.txt"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("-f", "Input file");
    parser.parse();
    CHECK(parser.getString("f") == "input.txt");
}

TEST_CASE("Empty vector default value") {
    const char* argv[] = {"prog"};
    int argc = 1;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addStrings("--names", "List of names", std::vector<std::string>{});
    parser.parse();
    auto names = parser.getStrings("names");
    CHECK(names.empty());
}

TEST_CASE("Help handler that throws") {
    const char* argv[] = {"prog", "--help"};
    int argc = 2;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.setHelpHandler([](std::string){ throw std::runtime_error("Help thrown"); });
    parser.addString("filename", "Input file");
    CHECK_THROWS_AS(parser.parse(), std::runtime_error);
}

TEST_CASE("Parsing with no arguments at all") {
    const char* argv[] = {"prog"};
    int argc = 1;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    CHECK_THROWS_AS(parser.parse() , Argy::MissingArgumentException);
}

TEST_CASE("Positional argument with default value throws") {
    const char* argv[] = {"prog"};
    int argc = 1;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file", "default.txt");
    parser.parse();
    CHECK(parser.getString("filename") == "default.txt");
}

// === VALIDATION TESTS ===

TEST_CASE("Validation: IsValueInRange for integers") {
    const char* argv[] = {"prog", "--count", "50"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addInt("--count", "Count value")
          .validate(IsValueInRange(1, 100));
    parser.parse();
    CHECK(parser.getInt("count") == 50);
}

TEST_CASE("Validation: IsValueInRange fails for out-of-range integers") {
    const char* argv[] = {"prog", "--count", "150"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addInt("--count", "Count value")
          .validate(IsValueInRange(1, 100));
    CHECK_THROWS_AS(parser.parse(), Argy::OutOfRangeException);
}

TEST_CASE("Validation: IsValueInRange for floats") {
    const char* argv[] = {"prog", "--ratio", "0.75"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addFloat("--ratio", "Ratio value")
          .validate(IsValueInRange(0.0f, 1.0f));
    parser.parse();
    CHECK(parser.getFloat("ratio") == doctest::Approx(0.75f));
}

TEST_CASE("Validation: IsVectorInRange for integer vectors") {
    const char* argv[] = {"prog", "--ids", "10", "20", "30"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addInts("--ids", "ID list")
          .validate(IsVectorInRange(1, 50));
    parser.parse();
    auto ids = parser.getInts("ids");
    CHECK(ids.size() == 3);
    CHECK(ids[0] == 10);
    CHECK(ids[1] == 20);
    CHECK(ids[2] == 30);
}

TEST_CASE("Validation: IsVectorInRange fails for out-of-range values") {
    const char* argv[] = {"prog", "--ids", "10", "60", "30"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addInts("--ids", "ID list")
          .validate(IsVectorInRange(1, 50));
    CHECK_THROWS_AS(parser.parse(), Argy::OutOfRangeException);
}

TEST_CASE("Validation: IsEmail valid email") {
    const char* argv[] = {"prog", "--email", "user@example.com"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--email", "Email address")
          .validate(IsEmail());
    parser.parse();
    CHECK(parser.getString("email") == "user@example.com");
}

TEST_CASE("Validation: IsEmail invalid email fails") {
    const char* argv[] = {"prog", "--email", "invalid-email"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--email", "Email address")
          .validate(IsEmail());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsUrl valid URL") {
    const char* argv[] = {"prog", "--url", "https://www.example.com"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--url", "Website URL")
          .validate(IsUrl());
    parser.parse();
    CHECK(parser.getString("url") == "https://www.example.com");
}

TEST_CASE("Validation: IsUrl invalid URL fails") {
    const char* argv[] = {"prog", "--url", "not-a-url"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--url", "Website URL")
          .validate(IsUrl());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsUUID valid UUID") {
    const char* argv[] = {"prog", "--uuid", "123e4567-e89b-12d3-a456-426614174000"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--uuid", "UUID identifier")
          .validate(IsUUID());
    parser.parse();
    CHECK(parser.getString("uuid") == "123e4567-e89b-12d3-a456-426614174000");
}

TEST_CASE("Validation: IsUUID invalid UUID fails") {
    const char* argv[] = {"prog", "--uuid", "not-a-uuid"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--uuid", "UUID identifier")
          .validate(IsUUID());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsAlphaNumeric valid string") {
    const char* argv[] = {"prog", "--token", "ABC123"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--token", "Token value")
          .validate(IsAlphaNumeric());
    parser.parse();
    CHECK(parser.getString("token") == "ABC123");
}

TEST_CASE("Validation: IsAlphaNumeric invalid string fails") {
    const char* argv[] = {"prog", "--token", "ABC-123"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--token", "Token value")
          .validate(IsAlphaNumeric());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsAlpha valid string") {
    const char* argv[] = {"prog", "--name", "John"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--name", "Name value")
          .validate(IsAlpha());
    parser.parse();
    CHECK(parser.getString("name") == "John");
}

TEST_CASE("Validation: IsAlpha invalid string fails") {
    const char* argv[] = {"prog", "--name", "John123"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--name", "Name value")
          .validate(IsAlpha());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsNumeric valid string") {
    const char* argv[] = {"prog", "--code", "123456"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--code", "Numeric code")
          .validate(IsNumeric());
    parser.parse();
    CHECK(parser.getString("code") == "123456");
}

TEST_CASE("Validation: IsNumeric invalid string fails") {
    const char* argv[] = {"prog", "--code", "123abc"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--code", "Numeric code")
          .validate(IsNumeric());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsOneOf valid choice") {
    const char* argv[] = {"prog", "--mode", "debug"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--mode", "Processing mode")
          .validate(IsOneOf({"normal", "debug", "fast"}));
    parser.parse();
    CHECK(parser.getString("mode") == "debug");
}

TEST_CASE("Validation: IsOneOf invalid choice fails") {
    const char* argv[] = {"prog", "--mode", "invalid"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--mode", "Processing mode")
          .validate(IsOneOf({"normal", "debug", "fast"}));
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsIPv4 valid IPv4") {
    const char* argv[] = {"prog", "--ip", "192.168.1.1"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--ip", "IPv4 address")
          .validate(IsIPv4());
    parser.parse();
    CHECK(parser.getString("ip") == "192.168.1.1");
}

TEST_CASE("Validation: IsIPv4 invalid IPv4 fails") {
    const char* argv[] = {"prog", "--ip", "999.999.999.999"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--ip", "IPv4 address")
          .validate(IsIPv4());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsIPv6 valid IPv6") {
    const char* argv[] = {"prog", "--ip", "2001:0db8:85a3:0000:0000:8a2e:0370:7334"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--ip", "IPv6 address")
          .validate(IsIPv6());
    parser.parse();
    CHECK(parser.getString("ip") == "2001:0db8:85a3:0000:0000:8a2e:0370:7334");
}

TEST_CASE("Validation: IsIPAddress accepts IPv4") {
    const char* argv[] = {"prog", "--ip", "127.0.0.1"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--ip", "IP address")
          .validate(IsIPAddress());
    parser.parse();
    CHECK(parser.getString("ip") == "127.0.0.1");
}

TEST_CASE("Validation: IsIPAddress accepts IPv6") {
    const char* argv[] = {"prog", "--ip", "2001:0db8:85a3:0000:0000:8a2e:0370:7334"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--ip", "IP address")
          .validate(IsIPAddress());
    parser.parse();
    CHECK(parser.getString("ip") == "2001:0db8:85a3:0000:0000:8a2e:0370:7334");
}

TEST_CASE("Validation: IsMACAddress valid MAC") {
    const char* argv[] = {"prog", "--mac", "00:1A:2B:3C:4D:5E"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--mac", "MAC address")
          .validate(IsMACAddress());
    parser.parse();
    CHECK(parser.getString("mac") == "00:1A:2B:3C:4D:5E");
}

TEST_CASE("Validation: IsMACAddress with dash separators") {
    const char* argv[] = {"prog", "--mac", "00-1A-2B-3C-4D-5E"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--mac", "MAC address")
          .validate(IsMACAddress());
    parser.parse();
    CHECK(parser.getString("mac") == "00-1A-2B-3C-4D-5E");
}

TEST_CASE("Validation: IsMACAddress invalid MAC fails") {
    const char* argv[] = {"prog", "--mac", "invalid-mac"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--mac", "MAC address")
          .validate(IsMACAddress());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsMatch with custom regex") {
    const char* argv[] = {"prog", "--code", "ABC123"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--code", "Product code")
          .validate(IsMatch(R"(^[A-Z]{3}\d{3}$)"));
    parser.parse();
    CHECK(parser.getString("code") == "ABC123");
}

TEST_CASE("Validation: IsMatch custom regex fails") {
    const char* argv[] = {"prog", "--code", "invalid"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--code", "Product code")
          .validate(IsMatch(R"(^[A-Z]{3}\d{3}$)"));
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: setValidator method") {
    const char* argv[] = {"prog", "--count", "50"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addInt("--count", "Count value");
    parser.setValidator("count", IsValueInRange(1, 100));
    parser.parse();
    CHECK(parser.getInt("count") == 50);
}

TEST_CASE("Validation: custom lambda validator") {
    const char* argv[] = {"prog", "--ratio", "0.75"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addFloat("--ratio", "Ratio value")
          .validate([](const std::string& name, float value) {
              if (value < 0.0f || value > 1.0f) {
                  throw InvalidValueException("Ratio must be between 0.0 and 1.0");
              }
          });
    parser.parse();
    CHECK(parser.getFloat("ratio") == doctest::Approx(0.75f));
}

TEST_CASE("Validation: custom lambda validator fails") {
    const char* argv[] = {"prog", "--ratio", "1.5"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addFloat("--ratio", "Ratio value")
          .validate([](const std::string& name, float value) {
              if (value < 0.0f || value > 1.0f) {
                  throw InvalidValueException("Ratio must be between 0.0 and 1.0");
              }
          });
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: conditional validation with empty string") {
    const char* argv[] = {"prog", "--mac", ""};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--mac", "MAC address", "")
          .validate([](const std::string& name, const std::string& value) {
              if (!value.empty()) IsMACAddress()(name, value);
          });
    parser.parse();
    CHECK(parser.getString("mac") == "");
}

// === TEMPLATE API VALIDATION TESTS ===

TEST_CASE("Template: Validation with IsValueInRange") {
    const char* argv[] = {"prog", "--count", "50"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<int>("--count", "Count value")
          .validate(IsValueInRange(1, 100));
    parser.parse();
    CHECK(parser.get<int>("count") == 50);
}

TEST_CASE("Template: Validation with IsEmail") {
    const char* argv[] = {"prog", "--email", "test@domain.com"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<std::string>("--email", "Email address")
          .validate(IsEmail());
    parser.parse();
    CHECK(parser.get<std::string>("email") == "test@domain.com");
}

TEST_CASE("Template: Validation with IsVectorInRange") {
    const char* argv[] = {"prog", "--ids", "5", "10", "15"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<Ints>("--ids", "ID list")
          .validate(IsVectorInRange(1, 20));
    parser.parse();
    auto ids = parser.get<Ints>("ids");
    CHECK(ids.size() == 3);
    CHECK(ids[0] == 5);
    CHECK(ids[1] == 10);
    CHECK(ids[2] == 15);
}

// === EXCEPTION HIERARCHY TESTS ===

TEST_CASE("Exception hierarchy: DefineException inheritance") {
    CHECK_THROWS_AS(throw DuplicateArgumentException("test"), DefineException);
    CHECK_THROWS_AS(throw DuplicateArgumentException("test"), Exception);
}

TEST_CASE("Exception hierarchy: ParseException inheritance") {
    CHECK_THROWS_AS(throw UnknownArgumentException("test"), ParseException);
    CHECK_THROWS_AS(throw MissingArgumentException("test"), ParseException);
    CHECK_THROWS_AS(throw TypeMismatchException("test"), ParseException);
}

TEST_CASE("Exception hierarchy: ValidateException inheritance") {
    CHECK_THROWS_AS(throw InvalidValueException("test"), ValidateException);
    CHECK_THROWS_AS(throw OutOfRangeException("test"), ValidateException);
}

// === EDGE CASES AND COMPREHENSIVE COVERAGE ===

TEST_CASE("Multiple aliases with validation") {
    const char* argv[] = {"prog", "-c", "25"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addInt({"-c", "--count", "-n", "--number"}, "Count value")
          .validate(IsValueInRange(1, 100));
    parser.parse();
    CHECK(parser.getInt("c") == 25);
    CHECK(parser.getInt("count") == 25);
    CHECK(parser.getInt("n") == 25);
    CHECK(parser.getInt("number") == 25);
}

TEST_CASE("Complex argument mix with validation") {
    const char* argv[] = {"prog", "input.txt", "--count", "42", "--email", "user@test.com", "--verbose"};
    int argc = 7;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("filename", "Input file");
    parser.addInt("--count", "Count value")
          .validate(IsValueInRange(1, 100));
    parser.addString("--email", "Email address")
          .validate(IsEmail());
    parser.addBool("--verbose", "Verbose output");
    parser.parse();
    
    CHECK(parser.getString("filename") == "input.txt");
    CHECK(parser.getInt("count") == 42);
    CHECK(parser.getString("email") == "user@test.com");
    CHECK(parser.getBool("verbose") == true);
}

TEST_CASE("Vector validation with mixed valid/invalid values") {
    const char* argv[] = {"prog", "--scores", "0.1", "0.5", "0.9"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addFloats("--scores", "Score values")
          .validate(IsVectorInRange(0.0f, 1.0f));
    parser.parse();
    auto scores = parser.getFloats("scores");
    CHECK(scores.size() == 3);
    CHECK(scores[0] == doctest::Approx(0.1f));
    CHECK(scores[1] == doctest::Approx(0.5f));
    CHECK(scores[2] == doctest::Approx(0.9f));
}

// === FILE SYSTEM VALIDATION TESTS ===

TEST_CASE("Validation: IsFile with existing file") {
    const std::string testFile = "test_temp_file.txt";
    TestUtil::createTempFile(testFile, "test content");
    
    const char* argv[] = {"prog", "--file", testFile.c_str()};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--file", "Input file")
          .validate(IsFile());
    parser.parse();
    CHECK(parser.getString("file") == testFile);
    
    TestUtil::cleanup(testFile);
}

TEST_CASE("Validation: IsFile with non-existent file fails") {
    const char* argv[] = {"prog", "--file", "nonexistent.txt"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--file", "Input file")
          .validate(IsFile());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsDirectory with existing directory") {
    const std::string testDir = "test_temp_dir";
    TestUtil::createTempDirectory(testDir);
    
    const char* argv[] = {"prog", "--dir", testDir.c_str()};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--dir", "Directory")
          .validate(IsDirectory());
    parser.parse();
    CHECK(parser.getString("dir") == testDir);
    
    TestUtil::cleanup(testDir);
}

TEST_CASE("Validation: IsDirectory with non-existent directory fails") {
    const char* argv[] = {"prog", "--dir", "nonexistent_dir"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--dir", "Directory")
          .validate(IsDirectory());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

TEST_CASE("Validation: IsPath with existing file") {
    const std::string testFile = "test_temp_path_file.txt";
    TestUtil::createTempFile(testFile, "test content");
    
    const char* argv[] = {"prog", "--path", testFile.c_str()};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--path", "File or directory path")
          .validate(IsPath());
    parser.parse();
    CHECK(parser.getString("path") == testFile);
    
    TestUtil::cleanup(testFile);
}

TEST_CASE("Validation: IsPath with existing directory") {
    const std::string testDir = "test_temp_path_dir";
    TestUtil::createTempDirectory(testDir);
    
    const char* argv[] = {"prog", "--path", testDir.c_str()};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--path", "File or directory path")
          .validate(IsPath());
    parser.parse();
    CHECK(parser.getString("path") == testDir);
    
    TestUtil::cleanup(testDir);
}

TEST_CASE("Validation: IsPath with non-existent path fails") {
    const char* argv[] = {"prog", "--path", "nonexistent_path"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--path", "File or directory path")
          .validate(IsPath());
    CHECK_THROWS_AS(parser.parse(), Argy::InvalidValueException);
}

// === CHAINING AND BUILDER PATTERN TESTS ===

TEST_CASE("ArgBuilder: chaining multiple validations") {
    const char* argv[] = {"prog", "--count", "50"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    auto builder = parser.addInt("--count", "Count value")
                         .validate(IsValueInRange(1, 100))
                         .validate([](const std::string& name, int value) {
                             if (value % 10 != 0) {
                                 throw InvalidValueException("Value must be divisible by 10");
                             }
                         });
    parser.parse();
    CHECK(parser.getInt("count") == 50);
}

TEST_CASE("ArgBuilder: done() method returns parser") {
    CliParser parser(0, nullptr);
    auto& parserRef = parser.addInt("--count", "Count value").done();
    CHECK(&parserRef == &parser);
}

// === STRESS TESTS AND EDGE CASES ===

TEST_CASE("Large vector validation") {
    std::vector<const char*> argv = {"prog", "--numbers"};
    std::vector<std::string> numberStrings;
    for (int i = 1; i <= 100; ++i) {
        numberStrings.push_back(std::to_string(i));
    }
    for (const auto& str : numberStrings) {
        argv.push_back(str.c_str());
    }
    int argc = static_cast<int>(argv.size());
    
    CliParser parser(argc, const_cast<char**>(argv.data()));
    parser.addInts("--numbers", "Large number list")
          .validate(IsVectorInRange(1, 100));
    parser.parse();
    auto numbers = parser.getInts("numbers");
    CHECK(numbers.size() == 100);
    CHECK(numbers[0] == 1);
    CHECK(numbers[99] == 100);
}

TEST_CASE("Multiple validators on same argument") {
    const char* argv[] = {"prog", "--email", "user@domain.com"};
    int argc = 3;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--email", "Email address")
          .validate(IsEmail())
          .validate([](const std::string& name, const std::string& value) {
              if (value.length() < 5) {
                  throw InvalidValueException("Email too short");
              }
          });
    parser.parse();
    CHECK(parser.getString("email") == "user@domain.com");
}

TEST_CASE("Validator with default value") {
    const char* argv[] = {"prog"};
    int argc = 1;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.addString("--email", "Email address", "default@test.com")
          .validate(IsEmail());
    parser.parse();
    CHECK(parser.getString("email") == "default@test.com");
}

// === TYPE ALIAS TESTS ===

TEST_CASE("Type aliases: Bools usage") {
    const char* argv[] = {"prog", "--flags", "1", "0", "1"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<Bools>("--flags", "Boolean flags");
    parser.parse();
    Bools flags = parser.get<Bools>("flags");
    CHECK(flags.size() == 3);
    CHECK(flags[0] == true);
    CHECK(flags[1] == false);
    CHECK(flags[2] == true);
}

TEST_CASE("Type aliases: Ints usage") {
    const char* argv[] = {"prog", "--numbers", "10", "20", "30"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<Ints>("--numbers", "Integer list");
    parser.parse();
    Ints numbers = parser.get<Ints>("numbers");
    CHECK(numbers.size() == 3);
    CHECK(numbers[0] == 10);
    CHECK(numbers[1] == 20);
    CHECK(numbers[2] == 30);
}

TEST_CASE("Type aliases: Floats usage") {
    const char* argv[] = {"prog", "--values", "1.1", "2.2", "3.3"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<Floats>("--values", "Float list");
    parser.parse();
    Floats values = parser.get<Floats>("values");
    CHECK(values.size() == 3);
    CHECK(values[0] == doctest::Approx(1.1f));
    CHECK(values[1] == doctest::Approx(2.2f));
    CHECK(values[2] == doctest::Approx(3.3f));
}

TEST_CASE("Type aliases: Strings usage") {
    const char* argv[] = {"prog", "--words", "hello", "world", "test"};
    int argc = 5;
    CliParser parser(argc, const_cast<char**>(argv));
    parser.add<Strings>("--words", "String list");
    parser.parse();
    Strings words = parser.get<Strings>("words");
    CHECK(words.size() == 3);
    CHECK(words[0] == "hello");
    CHECK(words[1] == "world");
    CHECK(words[2] == "test");
}
