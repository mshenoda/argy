// Example: Using template methods with Argy::Parser
#include "argy.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace Argy;

int main(int argc, char* argv[]) {
    CliParser args(argc, argv);
    try {
        // Positional argument
        args.add<string>("filename", "Input file");

        // Optional argument with short and long names, help text, and default value
        args.add<int>("-c", "--count", "Number of items", 10);

        // Optional boolean flag
        args.add<bool>("-v", "--verbose", "Enable verbose output", false);

        // Float argument
        args.add<float>("-r", "--ratio", "Ratio value", 0.5f);

        // Vector<int> argument
        args.add<Ints>("-i", "--ids", "List of IDs", Ints{1, 2, 3});

        // Vector<float> argument
        args.add<Floats>("-s", "--scores", "List of scores", Floats{0.1f, 0.2f, 0.3f});

        // Vector<bool> argument
        args.add<Bools>("-f", "--flags", "List of flags", Bools{true, false, true});

        // Vector<string> argument
        args.add<Strings>("-t", "--tags", "List of tags", Strings{"alpha", "beta"});

        args.parse();

        auto count = args.get<int>("count");
        auto filename = args.get<string>("filename");
        auto verbose = args.get<bool>("verbose");
        auto ratio = args.get<float>("ratio");
        auto ids = args.get<Ints>("ids");
        auto scores = args.get<Floats>("scores");
        auto flags = args.get<Bools>("flags");
        auto tags = args.get<Strings>("tags");

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
        cerr << "Error: " << ex.what() << endl;
        return 1;
    }
}
