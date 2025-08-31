// Example: Comprehensive showcase of Argy library capabilities using named API
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
    cli.setHelpHeader("Argy Named API Example - Comprehensive Argument Showcase");
    cli.setHelpDescription("It supports positional in the beginning followed by options \n"
                     "or POXIX style -- to treat all subsequent args as positional.");
    cli.setHelpFooter("For more information, visit: https://github.com/mshenoda/argy");
    
    try {
      //------------------------------------------------------------------------
      // POSITIONAL ARGUMENTS (Required inputs)
      //------------------------------------------------------------------------
      cli.addString("input_file", "Input file path").isFile(); // validate as existing file
      cli.addString("output_file", "Output file path (optional)", "result.txt");

      //------------------------------------------------------------------------
      // BASIC DATA TYPES (Fundamental types with validation)
      //------------------------------------------------------------------------
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

      //------------------------------------------------------------------------
      // FILE SYSTEM & PATHS (Directory and file path validation)
      //------------------------------------------------------------------------
      // Directory validation
      cli.addString({"-d", "--directory"}, "Working directory", ".").isDirectory();  

      // Path validation 
      cli.addString({"-pth", "--path"}, "Path to file or directory", "file.txt").isPath();

      //------------------------------------------------------------------------
      // NETWORK & COMMUNICATION (Email, URLs, IP addresses)
      //------------------------------------------------------------------------
      // Email validation
      cli.addString({"-e", "--email"}, "Contact email", "user@example.com").isEmail();

      // URL validation
      cli.addString({"-u", "--url"}, "API endpoint URL", "https://api.example.com").isUrl(); 

      // IP address validations
      cli.addString({"--ip"}, "Server IP address", "127.0.0.1").isIPAddress();
      cli.addString({"--ipv4"}, "IPv4 address", "192.168.1.1").isIPv4(); 
      cli.addString({"--ipv6"}, "IPv6 address", "2001:0db8:85a3:0000:0000:8a2e:0370:7334").isIPv6();

      // MAC address validation
      cli.addString({"--mac"}, "MAC address (optional)", "").isMACAddress();

      //------------------------------------------------------------------------
      // STRING VALIDATION (Pattern and format validation)
      //------------------------------------------------------------------------
      // Enum-like validation
      cli.addString({"-m", "--mode"}, "Processing mode", "normal")
         .isOneOf({"normal", "fast", "safe", "debug"});

      // Character type validations
      cli.addString({"-t", "--token"}, "Access token", "ABC123").isAlphaNumeric();
      cli.addString({"--alpha"}, "Alpha-only string", "abc").isAlpha();
      cli.addString({"--numeric"}, "Numeric string", "123").isNumeric();

      // UUID validation
      cli.addString({"--uuid"}, "UUID identifier", "123e4567-e89b-12d3-a456-426614174000").isUUID();

      //------------------------------------------------------------------------
      // VECTOR TYPES (Collections with validation)
      //------------------------------------------------------------------------
      // Integer vector with range validation
      cli.addInts({"-i", "--ids"}, "Vector of IDs (1-999)", Ints{1, 2, 3}).isInRange(1, 999);

      // Float vector
      cli.addFloats({"-s", "--scores"}, "Performance scores", Floats{0.8f, 0.9f, 0.75f}).isInRange(0.0f, 1.0f);

      // String vector
      cli.addStrings({"-p", "--plugins"}, "Plugin names", Strings{"auth", "logging"}).isAlpha();

      // Boolean vector
      cli.addBools({"-f", "--features"}, "Feature flags", Bools{true, false, true});

      // Parse arguments and get parsed values
      auto args = cli.parse();

      // Display parsed values using named API from parsed args
      cout << "=== PARSED ARGUMENTS (Named API) ===\n";
      
      // Positional Arguments
      cout << "\n--- POSITIONAL ARGUMENTS ---\n";
      cout << "Input File: " << args.getString("input_file") << "\n";
      cout << "Output File: " << args.getString("output_file") << "\n";
      
      // Basic Data Types
      cout << "\n--- BASIC DATA TYPES ---\n";
      cout << "Count: " << args.getInt("count") << "\n";
      cout << "Ratio: " << args.getFloat("ratio") << "\n";
      cout << "Verbose: " << (args.getBool("verbose") ? "ON" : "OFF") << "\n";
      cout << "Quiet: " << (args.getBool("quiet") ? "ON" : "OFF") << "\n";
      
      // File System & Paths
      cout << "\n--- FILE SYSTEM & PATHS ---\n";
      cout << "Directory: " << args.getString("directory") << "\n";
      cout << "Path: " << args.getString("path") << "\n";
      
      // Network & Communication
      cout << "\n--- NETWORK & COMMUNICATION ---\n";
      cout << "Email: " << args.getString("email") << "\n";
      cout << "URL: " << args.getString("url") << "\n";
      cout << "Server IP: " << args.getString("ip") << "\n";
      cout << "IPv4: " << args.getString("ipv4") << "\n";
      cout << "IPv6: " << args.getString("ipv6") << "\n";
      if (args.has("mac") && !args.getString("mac").empty()) {
         cout << "MAC Address: " << args.getString("mac") << "\n";
      }
      
      // String Validation
      cout << "\n--- STRING VALIDATION ---\n";
      cout << "Mode: " << args.getString("mode") << "\n";
      cout << "Token: " << args.getString("token") << "\n";
      cout << "Alpha String: " << args.getString("alpha") << "\n";
      cout << "Numeric String: " << args.getString("numeric") << "\n";
      cout << "UUID: " << args.getString("uuid") << "\n";

      // Vector Types
      cout << "\n--- VECTOR TYPES ---\n";
      cout << "IDs: ";
      for (auto id : args.getInts("ids")) cout << id << " ";
      cout << "\nScores: ";
      for (auto score : args.getFloats("scores")) cout << score << " ";
      cout << "\nPlugins: ";
      for (const auto& plugin : args.getStrings("plugins")) cout << plugin << " ";
      cout << "\nFeatures: ";
      for (auto feature : args.getBools("features")) cout << (feature ? "ON" : "OFF") << " ";
      cout << "\n";

    } catch (const Argy::Exception& ex) {
        cerr << "Error: " << ex.what() << endl;
        return 1;
    }

    return 0;
}
