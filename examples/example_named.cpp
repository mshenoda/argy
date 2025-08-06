// Example: Using named (convenience) methods with Argy::Parser
#include "argy.hpp"
#include <iostream>
#include <vector>
#include <string>

using namespace std;
using namespace Argy;

int main(int argc, char* argv[]) {
    Argy::Parser args(argc, argv);

    // Named convenience methods for argument definitions
    args.addString("file", "Input File");
    args.addString({"-o", "--output"}, "Output directory");
    args.addFloat({"-nms", "--nms-thresh"}, "NMS threshold", 0.45f);
    args.addBool({"-v", "--verbose"}, "Enable verbose output", false);
    args.addInts({"-s", "--input-shape"}, "Input shape (HxW)", vector<int>{640, 480});

    try {
        args.parse();
        cout << "output: " << args.getString("output") << "\n";
        cout << "nms-thresh: " << args.getFloat("nms-thresh") << "\n";
        cout << "verbose: " << args.getBool("verbose") << "\n";
        auto input_shape = args.getInts("input-shape");
        cout << "input-shape:";
        for (int s : input_shape) cout << ' ' << s;
        cout << '\n';
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << '\n';
        args.printHelp(argv[0]);
        return 1;
    }
    return 0;
}
