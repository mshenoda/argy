// Example: Using template methods with Argy::Parser
#include "argy.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace Argy;

int main(int argc, char* argv[]) {
    Argy::Parser args(argc, argv);

    // Template style argument definitions
    args.add<string>("image", "Path to input image");
    args.add<string>({"-m","--model"}, "Path to model file");
    args.add<float>({"-t","--threshold"}, "Confidence threshold");
    args.add<int>({"-b","--batch"}, "Batch size");
    args.add<string>({"-d","--device"}, "Device to use (cpu/gpu)");
    args.add<bool>("--visualize", "Show visualization window");
    args.add<bool>("--save", "Save output results");
    args.add<vector<float>>("--mean", "Mean values for normalization");
    args.add<vector<float>>("--std", "Std values for normalization");

    try {
        args.parse();
        cout << "image: " << args.get<string>("image") << "\n";
        cout << "model: " << args.get<string>("model") << "\n";
        cout << "threshold: " << args.get<float>("threshold") << "\n";
        cout << "batch: " << args.get<int>("batch") << "\n";
        cout << "device: " << args.get<string>("device") << "\n";
        cout << "visualize: " << args.get<bool>("visualize") << "\n";
        cout << "save: " << args.get<bool>("save") << "\n";
        auto mean = args.get<vector<float>>("mean");
        cout << "mean:";
        for (float v : mean) cout << ' ' << v;
        cout << '\n';
        auto std = args.get<vector<float>>("std");
        cout << "std:";
        for (float v : std) cout << ' ' << v;
        cout << '\n';
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << '\n';
        args.printHelp(argv[0]);
        return 1;
    }
    return 0;
}
