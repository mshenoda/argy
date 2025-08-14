// Example: Using template methods with Argy::Parser
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
        cli.add<string>("filename", "Input file");

        // Optional argument with multiple aliases
        cli.add<int>({"-c", "--count", "-n", "--num"}, "Number of items");

        cli.add<bool>({"-v", "--visualize", "-s", "--show"}, "Visualize results"); // adding argument with multiple aliases 

        // Float argument with aliases
        cli.add<float>({"-r", "--ratio"}, "Ratio value", 0.5f);

        // Vector<int> argument
        cli.add<Ints>({"-i", "--ids"}, "List of IDs", Ints{1, 2, 3});

        // Vector<float> argument
        cli.add<Floats>({"-ss", "--scores"}, "List of scores", Floats{0.1f, 0.2f, 0.3f});

        // Vector<bool> argument
        cli.add<Bools>({"-f", "--flags"}, "List of flags", Bools{true, false, true});

        // Vector<string> argument
        cli.add<Strings>({"-t", "--tags"}, "List of tags", Strings{"alpha", "beta"});

        cli.parse();

        auto count = cli.get<int>("count");
        auto filename = cli.get<string>("filename");
        auto visualize = cli.get<bool>("show");
        auto ratio = cli.get<float>("ratio");
        auto ids = cli.get<Ints>("ids");
        auto scores = cli.get<Floats>("scores");
        auto flags = cli.get<Bools>("flags");
        auto tags = cli.get<Strings>("tags");

        cout << "Filename: " << filename << "\n";
        cout << "Count: " << count << "\n";
        cout << "Visualize: " << (visualize ? "true" : "false") << "\n";
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
