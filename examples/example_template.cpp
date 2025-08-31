// Example: Comprehensive showcase of Argy library capabilities using template-based API
#include "argy.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace Argy;

int main(int argc, char* argv[]) {
    CliParser cli(argc, argv);
    
    //--------------------------------------------------------------------------
    // HELP CUSTOMIZATION (Header, Usage, Footer)
    //--------------------------------------------------------------------------
    cli.setHelpHeader("Argy Template API Example - Comprehensive Argument Showcase");
    cli.setHelpDescription("It supports positional in the beginning followed by options \n"
                     "or POXIX style -- to treat all subsequent args as positional.");
    cli.setHelpFooter("For more information, visit: https://github.com/mshenoda/argy");
    
    try {
      //------------------------------------------------------------------------
      // POSITIONAL ARGUMENTS (Required inputs)
      //------------------------------------------------------------------------
      cli.add<string>("input_file", "Input file path").isFile(); // validate as existing file
      cli.add<string>("output_file", "Output file path (optional)", "result.txt");

      //------------------------------------------------------------------------
      // BASIC DATA TYPES (Fundamental types with validation)
      //------------------------------------------------------------------------
      // Integer with range validation
      cli.add<int>({"-c", "--count"}, "Number of items (1-100)", 10).isInRange(1, 100);

      // Float with custom validation
      cli.add<float>({"-r", "--ratio"}, "Ratio value (0.0-1.0)", 0.5f)
         .validate([](const string& name, float value) {
            if (value < 0.0f || value > 1.0f) 
                  throw InvalidValueException("Ratio must be between 0.0 and 1.0");
         });

      // Boolean flags
      cli.add<bool>({"-v", "--verbose"}, "Enable verbose output", false);
      cli.add<bool>({"-q", "--quiet"}, "Quiet mode", false);

      //------------------------------------------------------------------------
      // FILE SYSTEM & PATHS (Directory and file path validation)
      //------------------------------------------------------------------------
      // Directory validation
      cli.add<string>({"-d", "--directory"}, "Working directory", ".").isDirectory();  

      // Path validation 
      cli.add<string>({"-pth", "--path"}, "Path to file or directory", "file.txt").isPath();

      //------------------------------------------------------------------------
      // NETWORK & COMMUNICATION (Email, URLs, IP addresses)
      //------------------------------------------------------------------------
      // Email validation
      cli.add<string>({"-e", "--email"}, "Contact email", "user@example.com").isEmail();

      // URL validation
      cli.add<string>({"-u", "--url"}, "API endpoint URL", "https://api.example.com").isUrl(); 

      // IP address validations
      cli.add<string>({"--ip"}, "Server IP address", "127.0.0.1").isIPAddress();
      cli.add<string>({"--ipv4"}, "IPv4 address", "192.168.1.1").isIPv4(); 
      cli.add<string>({"--ipv6"}, "IPv6 address", "2001:0db8:85a3:0000:0000:8a2e:0370:7334").isIPv6();

      // MAC address validation
      cli.add<string>({"--mac"}, "MAC address (optional)", "").isMACAddress();

      //------------------------------------------------------------------------
      // STRING VALIDATION (Pattern and format validation)
      //------------------------------------------------------------------------
      // Enum-like validation
      cli.add<string>({"-m", "--mode"}, "Processing mode", "normal")
         .isOneOf({"normal", "fast", "safe", "debug"});

      // Character type validations
      cli.add<string>({"-t", "--token"}, "Access token", "ABC123").isAlphaNumeric();
      cli.add<string>({"--alpha"}, "Alpha-only string", "abc").isAlpha();
      cli.add<string>({"--numeric"}, "Numeric string", "123").isNumeric();

      // UUID validation
      cli.add<string>({"--uuid"}, "UUID identifier", "123e4567-e89b-12d3-a456-426614174000").isUUID();

      //------------------------------------------------------------------------
      // VECTOR TYPES (Collections with validation)
      //------------------------------------------------------------------------
      // Integer vector with range validation
      cli.add<Ints>({"-i", "--ids"}, "Vector of IDs (1-999)", Ints{1, 2, 3}).isInRange(1, 999);

      // Float vector
      cli.add<Floats>({"-s", "--scores"}, "Performance scores", Floats{0.8f, 0.9f, 0.75f}).isInRange(0.0f, 1.0f);

      // String vector
      cli.add<Strings>({"-p", "--plugins"}, "Plugin names", Strings{"auth", "logging"}).isAlpha();

      // Boolean vector
      cli.add<Bools>({"-f", "--features"}, "Feature flags", Bools{true, false, true});

      // Parse arguments and get parsed values
      auto args = cli.parse();

      // Display parsed values using template-based get<T>() from parsed args
      cout << "=== PARSED ARGUMENTS (Template API) ===\n";
      
      // Positional Arguments
      cout << "\n--- POSITIONAL ARGUMENTS ---\n";
      cout << "Input File: " << args.get<string>("input_file") << "\n";
      cout << "Output File: " << args.get<string>("output_file") << "\n";
      
      // Basic Data Types
      cout << "\n--- BASIC DATA TYPES ---\n";
      cout << "Count: " << args.get<int>("count") << "\n";
      cout << "Ratio: " << args.get<float>("ratio") << "\n";
      cout << "Verbose: " << (args.get<bool>("verbose") ? "ON" : "OFF") << "\n";
      cout << "Quiet: " << (args.get<bool>("quiet") ? "ON" : "OFF") << "\n";
      
      // File System & Paths
      cout << "\n--- FILE SYSTEM & PATHS ---\n";
      cout << "Directory: " << args.get<string>("directory") << "\n";
      cout << "Path: " << args.get<string>("path") << "\n";
      
      // Network & Communication
      cout << "\n--- NETWORK & COMMUNICATION ---\n";
      cout << "Email: " << args.get<string>("email") << "\n";
      cout << "URL: " << args.get<string>("url") << "\n";
      cout << "Server IP: " << args.get<string>("ip") << "\n";
      cout << "IPv4: " << args.get<string>("ipv4") << "\n";
      cout << "IPv6: " << args.get<string>("ipv6") << "\n";
      if (args.has("mac") && !args.get<string>("mac").empty()) {
         cout << "MAC Address: " << args.get<string>("mac") << "\n";
      }
      
      // String Validation
      cout << "\n--- STRING VALIDATION ---\n";
      cout << "Mode: " << args.get<string>("mode") << "\n";
      cout << "Token: " << args.get<string>("token") << "\n";
      cout << "Alpha String: " << args.get<string>("alpha") << "\n";
      cout << "Numeric String: " << args.get<string>("numeric") << "\n";
      cout << "UUID: " << args.get<string>("uuid") << "\n";

      // Vector Types
      cout << "\n--- VECTOR TYPES ---\n";
      cout << "IDs: ";
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
