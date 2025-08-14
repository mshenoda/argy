// Example: Using named (convenience) methods with Argy::Parser
#include "argy.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace Argy;

int main(int argc, char* argv[]) {
    CliParser cli(argc, argv);
    try {
        // Positional argument
        cli.addString("filename", "Input file");
  
        // Optional argument with multiple aliases
        cli.addInt({"-c", "--count", "-n", "--num"}, "Number of items", 10);

        // Optional boolean flag
        cli.addBool({"-v", "--verbose"}, "Enable verbose output", false);

        // Float argument
        cli.addFloat({"-r", "--ratio"}, "Ratio value", 0.5f);

        // Vector<int> argument
        cli.addInts({"-i", "--ids"}, "List of IDs", Ints{1, 2, 3});

        // Vector<float> argument
        cli.addFloats({"-s", "--scores"}, "List of scores", Floats{0.1f, 0.2f, 0.3f});

        // Vector<bool> argument
        cli.addBools({"-f", "--flags"}, "List of flags", Bools{true, false, true});

        // Vector<string> argument
        cli.addStrings({"-t", "--tags"}, "List of tags", Strings{"alpha", "beta"});

        cli.parse();

        auto count = cli.getInt("n");
        auto filename = cli.getString("filename");
        auto verbose = cli.getBool("verbose");
        auto ratio = cli.getFloat("ratio");
        auto ids = cli.getInts("ids");
        auto scores = cli.getFloats("scores");
        auto flags = cli.getBools("flags");
        auto tags = cli.getStrings("tags");

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
