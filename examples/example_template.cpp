// Example: Comprehensive showcase of Argy library capabilities using template-based API
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
        cli.add<string>("input_file", "Input file path");
        cli.add<string>("output_file", "Output file path (optional)", "result.txt");

        // === BASIC TYPES WITH VALIDATION ===
        // Integer with range validation
        cli.add<int>({"-c", "--count"}, "Number of items (1-100)", 10)
           .validate(IsValueInRange(1, 100));

        // Float with custom validation
        cli.add<float>({"-r", "--ratio"}, "Ratio value (0.0-1.0)", 0.5f)
           .validate([](const string& name, float value) {
               if (value < 0.0f || value > 1.0f) 
                   throw InvalidValueException("Ratio must be between 0.0 and 1.0");
           });

        // Boolean flags
        cli.add<bool>({"-v", "--verbose"}, "Enable verbose output", false);
        cli.add<bool>({"-q", "--quiet"}, "Quiet mode", false);

        // === STRING VALIDATION ===
        // Email validation
        cli.add<string>({"-e", "--email"}, "Contact email", "user@example.com")
           .validate(IsEmail());

        // URL validation
        cli.add<string>({"-u", "--url"}, "API endpoint URL", "https://api.example.com")
           .validate(IsUrl());

        // Path validation (using setValidator)
        cli.add<string>({"-d", "--directory"}, "Working directory", ".");
        cli.setValidator("directory", IsDirectory());

        // Enum-like validation
        cli.add<string>({"-m", "--mode"}, "Processing mode", "normal")
           .validate(IsOneOf({"normal", "fast", "safe", "debug"}));

        // Alpha-numeric validation
        cli.add<string>({"-t", "--token"}, "Access token", "ABC123")
           .validate(IsAlphaNumeric());

        // === ADDITIONAL STRING VALIDATORS ===
        // Alpha-only validation
        cli.add<string>({"--alpha"}, "Alpha-only string", "abc")
           .validate(IsAlpha());

        // Numeric-only validation
        cli.add<string>({"--numeric"}, "Numeric string", "123")
           .validate(IsNumeric());

        // UUID validation
        cli.add<string>({"--uuid"}, "UUID identifier", "123e4567-e89b-12d3-a456-426614174000")
           .validate(IsUUID());

        // === VECTOR TYPES ===
        // Integer list with range validation
        cli.add<Ints>({"-i", "--ids"}, "List of IDs (1-999)", Ints{1, 2, 3})
           .validate(IsVectorInRange(1, 999));

        // Float list
        cli.add<Floats>({"-s", "--scores"}, "Performance scores", Floats{0.8f, 0.9f, 0.75f});

        // String list
        cli.add<Strings>({"-p", "--plugins"}, "Plugin names", Strings{"auth", "logging"});

        // Boolean list
        cli.add<Bools>({"-f", "--features"}, "Feature flags", Bools{true, false, true});

        // === NETWORK VALIDATION ===
        cli.add<string>({"--ip"}, "Server IP address", "127.0.0.1")
           .validate(IsIPAddress());

        cli.add<string>({"--ipv4"}, "IPv4 address", "192.168.1.1")
           .validate(IsIPv4());

        cli.add<string>({"--ipv6"}, "IPv6 address", "2001:0db8:85a3:0000:0000:8a2e:0370:7334")
           .validate(IsIPv6());

        cli.add<string>({"--mac"}, "MAC address (optional)", "")
           .validate([](const string& name, const string& value) {
               if (!value.empty()) IsMACAddress()(name, value);
           });

      // === PARSING AND OUTPUT ===
      auto args = cli.parse();

      // Display parsed values using template-based get<T>() from parsed args
      cout << "=== PARSED ARGUMENTS (Template API) ===\n";
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
      cout << "Alpha String: " << args.get<string>("alpha") << "\n";
      cout << "Numeric String: " << args.get<string>("numeric") << "\n";
      cout << "UUID: " << args.get<string>("uuid") << "\n";
      cout << "Server IP: " << args.get<string>("ip") << "\n";
      cout << "IPv4: " << args.get<string>("ipv4") << "\n";
      cout << "IPv6: " << args.get<string>("ipv6") << "\n";

      if (args.has("mac") && !args.get<string>("mac").empty()) {
         cout << "MAC Address: " << args.get<string>("mac") << "\n";
      }

      // Vector outputs using template-based API
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
        cerr << "Error: " << ex.what() << endl;
        return 1;
    }

    return 0;
}
