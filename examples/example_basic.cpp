// Example: Basic usage of Argy library - A gentle introduction
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
        // Required positional argument
        cli.addString("filename", "Input file to process");
        
        // Optional positional argument with default value
        cli.addString("output", "Output file", "result.txt");

        // === BASIC OPTIONAL ARGUMENTS ===
        // Integer argument with default value
        cli.addInt({"-c", "--count"}, "Number of items to process", 5);
        
        // Float argument
        cli.addFloat({"-r", "--rate"}, "Processing rate", 1.0f);
        
        // Boolean flags (default to false)
        cli.addBool({"-v", "--verbose"}, "Enable verbose output");
        cli.addBool({"-d", "--debug"}, "Enable debug mode");
        
        // String argument with multiple aliases
        cli.addString({"-n", "--name", "--username"}, "User name", "anonymous");

        // === VECTOR ARGUMENTS (LISTS) ===
        // List of integers
        cli.addInts({"-i", "--ids"}, "List of item IDs", Ints{1, 2, 3});
        
        // List of strings
        cli.addStrings({"-t", "--tags"}, "List of tags", Strings{"default"});
        
        // List of floats
        cli.addFloats({"-s", "--scores"}, "Score values", Floats{0.5f, 0.7f});

        // === PARSE ARGUMENTS ===
        cli.parse();

        // === ACCESS PARSED VALUES ===
        cout << "=== Basic Argy Example ===\n";
        cout << "Input file: " << cli.getString("filename") << "\n";
        cout << "Output file: " << cli.getString("output") << "\n";
        cout << "Count: " << cli.getInt("count") << "\n";
        cout << "Rate: " << cli.getFloat("rate") << "\n";
        cout << "Verbose: " << (cli.getBool("verbose") ? "ON" : "OFF") << "\n";
        cout << "Debug: " << (cli.getBool("debug") ? "ON" : "OFF") << "\n";
        cout << "Username: " << cli.getString("name") << "\n";

        // Display vector values
        cout << "\nList values:\n";
        cout << "IDs: ";
        for (int id : cli.getInts("ids")) {
            cout << id << " ";
        }
        cout << "\nTags: ";
        for (const string& tag : cli.getStrings("tags")) {
            cout << tag << " ";
        }
        cout << "\nScores: ";
        for (float score : cli.getFloats("scores")) {
            cout << score << " ";
        }
        cout << "\n";

        // === DEMONSTRATE has() METHOD ===
        cout << "\nArgument presence check:\n";
        cout << "Verbose flag provided: " << (cli.has("verbose") ? "YES" : "NO") << "\n";
        cout << "Debug flag provided: " << (cli.has("debug") ? "YES" : "NO") << "\n";

    } catch (const std::exception& ex) {
        cerr << "Error: " << ex.what() << endl;
        cerr << "Try running with --help for usage information." << endl;
        return 1;
    }

    return 0;
}
