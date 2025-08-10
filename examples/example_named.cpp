// Example: Using named (convenience) methods with Argy::Parser
#include "argy.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace Argy;

int main(int argc, char* argv[]) {
    ArgParser args(argc, argv);
    try {
        // Positional argument
        args.addString("filename", "Input file");

        // Optional argument with short and long names, help text, and default value
        args.addInt("-c", "--count", "Number of items", 10);

        // Optional boolean flag
        args.addBool("-v", "--verbose", "Enable verbose output", false);

        // Float argument
        args.addFloat("-r", "--ratio", "Ratio value", 0.5f);

        // Vector<int> argument
        args.addInts("-i", "--ids", "List of IDs", Ints{1, 2, 3});

        // Vector<float> argument
        args.addFloats("-s", "--scores", "List of scores", Floats{0.1f, 0.2f, 0.3f});

        // Vector<bool> argument
        args.addBools("-f", "--flags", "List of flags", Bools{true, false, true});

        // Vector<string> argument
        args.addStrings("-t", "--tags", "List of tags", Strings{"alpha", "beta"});

        args.parse();

        auto count = args.getInt("count");
        auto filename = args.getString("filename");
        auto verbose = args.getBool("verbose");
        auto ratio = args.getFloat("ratio");
        auto ids = args.getInts("ids");
        auto scores = args.getFloats("scores");
        auto flags = args.getBools("flags");
        auto tags = args.getStrings("tags");

        cout << "Filename: " << filename << "\n";
        cout << "Count: " << count << "\n";
        cout << "Verbose: " << (verbose ? "true" : "false") << "\n";
        cout << "Ratio: " << ratio << "\n";
        cout << "IDs: ";
        for (auto v : ids) cout << v << " ";
        cout << "\nScores: ";
        for (auto v : scores) cout << v << " ";
        cout << "\nFlags: ";
        for (auto v : flags) cout << (v ? "true" : "false") << " ";
        cout << "\nTags: ";
        for (const auto& v : tags) cout << v << " ";
        cout << "\n";
    } catch (const std::exception& ex) {
        cerr << "Error: " << ex.what() << std::endl;
        return 1;
    }
}
