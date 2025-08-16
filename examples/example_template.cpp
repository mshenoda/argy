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
        cli.parse();

        // Display parsed values using template-based get<T>()
        cout << "=== PARSED ARGUMENTS (Template API) ===\n";
        cout << "Input File: " << cli.get<string>("input_file") << "\n";
        cout << "Output File: " << cli.get<string>("output_file") << "\n";
        cout << "Count: " << cli.get<int>("count") << "\n";
        cout << "Ratio: " << cli.get<float>("ratio") << "\n";
        cout << "Verbose: " << (cli.get<bool>("verbose") ? "ON" : "OFF") << "\n";
        cout << "Quiet: " << (cli.get<bool>("quiet") ? "ON" : "OFF") << "\n";
        cout << "Email: " << cli.get<string>("email") << "\n";
        cout << "URL: " << cli.get<string>("url") << "\n";
        cout << "Directory: " << cli.get<string>("directory") << "\n";
        cout << "Mode: " << cli.get<string>("mode") << "\n";
        cout << "Token: " << cli.get<string>("token") << "\n";
        cout << "Alpha String: " << cli.get<string>("alpha") << "\n";
        cout << "Numeric String: " << cli.get<string>("numeric") << "\n";
        cout << "UUID: " << cli.get<string>("uuid") << "\n";
        cout << "Server IP: " << cli.get<string>("ip") << "\n";
        cout << "IPv4: " << cli.get<string>("ipv4") << "\n";
        cout << "IPv6: " << cli.get<string>("ipv6") << "\n";
        
        if (cli.has("mac") && !cli.get<string>("mac").empty()) {
            cout << "MAC Address: " << cli.get<string>("mac") << "\n";
        }

        // Vector outputs using template-based API
        cout << "\nIDs: ";
        for (auto id : cli.get<Ints>("ids")) cout << id << " ";
        cout << "\nScores: ";
        for (auto score : cli.get<Floats>("scores")) cout << score << " ";
        cout << "\nPlugins: ";
        for (const auto& plugin : cli.get<Strings>("plugins")) cout << plugin << " ";
        cout << "\nFeatures: ";
        for (auto feature : cli.get<Bools>("features")) cout << (feature ? "ON" : "OFF") << " ";
        cout << "\n";

    } catch (const Argy::Exception& ex) {
        cerr << "Error: " << ex.what() << endl;
        return 1;
    }

    return 0;
}
