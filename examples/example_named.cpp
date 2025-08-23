// Example: Comprehensive showcase of Argy library capabilities
#include "argy.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace Argy;

int main(int argc, char* argv[]) {
    CliParser cli(argc, argv);
    try {
        // === POSITIONAL ARGUMENTS ===
        cli.addString("input_file", "Input file path");
        cli.addString("output_file", "Output file path (optional)", "result.txt");

        // === BASIC TYPES WITH VALIDATION ===
        // Integer with range validation
        cli.addInt({"-c", "--count"}, "Number of items (1-100)", 10)
           .validate(IsValueInRange(1, 100));

        // Float with custom validation
        cli.addFloat({"-r", "--ratio"}, "Ratio value (0.0-1.0)", 0.5f)
           .validate([](const string& name, float value) {
               if (value < 0.0f || value > 1.0f) 
                   throw InvalidValueException("Ratio must be between 0.0 and 1.0");
           });

        // Boolean flags
        cli.addBool({"-v", "--verbose"}, "Enable verbose output", false);
        cli.addBool({"-q", "--quiet"}, "Quiet mode", false);

        // === STRING VALIDATION ===
        // Email validation
        cli.addString({"-e", "--email"}, "Contact email", "user@example.com")
           .validate(IsEmail());

        // URL validation
        cli.addString({"-u", "--url"}, "API endpoint URL", "https://api.example.com")
           .validate(IsUrl());

        // Path validation (using setValidator)
        cli.addString({"-d", "--directory"}, "Working directory", ".");
        cli.setValidator("directory", IsDirectory());

        // Enum-like validation
        cli.addString({"-m", "--mode"}, "Processing mode", "normal")
           .validate(IsOneOf({"normal", "fast", "safe", "debug"}));

        // Alpha-numeric validation
        cli.addString({"-t", "--token"}, "Access token", "ABC123")
           .validate(IsAlphaNumeric());

        // === VECTOR TYPES ===
        // Integer list with range validation
        cli.addInts({"-i", "--ids"}, "List of IDs (1-999)", Ints{1, 2, 3})
           .validate(IsVectorInRange(1, 999));

        // Float list
        cli.addFloats({"-s", "--scores"}, "Performance scores", Floats{0.8f, 0.9f, 0.75f});

        // String list
        cli.addStrings({"-p", "--plugins"}, "Plugin names", Strings{"auth", "logging"});

        // Boolean list
        cli.addBools({"-f", "--features"}, "Feature flags", Bools{true, false, true});

        // === NETWORK VALIDATION ===
        cli.addString({"--ip"}, "Server IP address", "127.0.0.1")
           .validate(IsIPAddress());

        cli.addString({"--mac"}, "MAC address (optional)", "")
           .validate([](const string& name, const string& value) {
               if (!value.empty()) IsMACAddress()(name, value);
           });

        // === PARSING AND OUTPUT ===
      auto args = cli.parse();

      // Display parsed values from parsed args
      cout << "=== PARSED ARGUMENTS ===\n";
      cout << "Input File: " << args.get<string>("input_file") << "\n";
      cout << "Output File: " << args.get<string>("output_file") << "\n";
      cout << "Count: " << args.get<int>("count") << "\n";
      cout << "Ratio: " << args.get<float>("ratio") << "\n";
      cout << "Verbose: " << (args.get<bool>("verbose") ? "ON" : "OFF") << "\n";
      cout << "Quiet: " << (args.get<bool>("quiet") ? "ON" : "OFF") << "\n";
      cout << "Email: " << args.get<string>("email") << "\n";
      cout << "URL: " << args.get<string>("url") << "\n";
      cout << "Directory: " << args.get<string>("directory") << "\n";
      cout << "Mode: " << args.get<string>("mode") << "\n";
      cout << "Token: " << args.get<string>("token") << "\n";
      cout << "Server IP: " << args.get<string>("ip") << "\n";

      if (args.has("mac") && !args.get<string>("mac").empty()) {
         cout << "MAC Address: " << args.get<string>("mac") << "\n";
      }

      // Vector outputs
      cout << "\nIDs: ";
      for (auto id : args.get<Ints>("ids")) cout << id << " ";
      cout << "\nScores: ";
      for (auto score : args.get<Floats>("scores")) cout << score << " ";
      cout << "\nPlugins: ";
      for (const auto& plugin : args.get<Strings>("plugins")) cout << plugin << " ";
      cout << "\nFeatures: ";
      for (auto feature : args.get<Bools>("features")) cout << (feature ? "ON" : "OFF") << " ";
      cout << "\n";

    } catch (const Argy::Exception& ex) {
        cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
