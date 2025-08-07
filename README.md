<p align="center">
  <img src="logo.svg" alt="argy logo" width="100"/>
</p>

## Argy
**Effortless Command-Line Parsing for Modern C++**  
*Because life’s too short to parse argv by hand!*

A modern, header-only C++17 argument parser that makes CLI development a breeze.

*If you love Python's argparse or click, you'll feel right at home—argy brings that developer-friendly style to C++.*


## Features
- 📦 **Header-only** & zero dependencies
- 🧩 **Intuitive API** for defining and parsing command-line arguments
- 🎯 Supports positional, optional, and flag arguments
- 🔒 **Type-safe** access to parsed values
- 📝 Customizable help output
- 🛠️ Modern, chainable API

## Installation

There are several ways to use argy in your project:

- **With CMake (FetchContent) (Recommended):**
  ```cmake
  include(FetchContent)
  FetchContent_Declare(
    argy
    GIT_REPOSITORY https://github.com/mshenoda/argy.git
    GIT_TAG        main # or a specific tag/release
  )
  FetchContent_MakeAvailable(argy)
  target_link_libraries(your_target PRIVATE argy)
  ```

- **With CMake (add_subdirectory):**
  First, clone the repository:
  ```sh
  git clone https://github.com/mshenoda/argy.git
  ```
  Then, in your CMakeLists.txt:
  ```cmake
  add_subdirectory(argy)
  target_link_libraries(your_target PRIVATE argy)
  ```

- **Header-only (any project type):** Simply copy the `include/argy.hpp` file into your project.

## API Usage

### Template Methods
Define arguments and access them with the template API:
```cpp
#include <argy.hpp>
using namespace std;
using namespace Argy;
Parser args(argc, argv);
args.add<string>("image", "Path to input image");
args.add<string>({"-m", "--model"}, "Path to model weights");
args.add<float>({"-t", "--threshold"}, "Detection threshold", 0.5f);
args.add<bool>("--visualize", "Show detection results", false);
args.add<Ints>("--input-size", "Input size", Ints{640, 480});
```

### Named Convenience Methods
Use named methods for clarity:
```cpp
using namespace std;
using namespace Argy;
Parser args(argc, argv);
args.addString("image", "Path to input image");
args.addInt({"-n", "--num-classes"}, "Number of classes", 80);
args.addBool({"-s", "--save-vis"}, "Save visualization images", false);
```

### Chaining Methods
Chain argument definitions for a fluent API:

// Template method chaining
```cpp
using namespace std;
using namespace Argy;
Parser args(argc, argv);
args.add<string>("image", "Path to input image")
    .add<string>({"-m", "--model"}, "Path to model weights")
    .add<float>({"-t", "--threshold"}, "Detection threshold");
```

// Named convenience method chaining
```cpp
using namespace std;
using namespace Argy;
Parser args(argc, argv);
args.addString("image", "Path to input image")
    .addInt({"-n", "--num-classes"}, "Number of classes", 80)
    .addBool({"-s", "--save-vis"}, "Save visualization images", false);
```

### Parsing and Accessing Arguments
Typical usage pattern:
```cpp
try {
    args.parse();
    auto image = args.get<string>("image");
    auto threshold = args.get<float>("threshold");
} catch (const exception& ex) {
    cerr << "Error: " << ex.what() << '\n';
    args.printHelp(argv[0]);
    return 1;
}
```

## Full Examples

### Full example using template methods
```cpp
#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include "argy.hpp"
using namespace std;
using namespace Argy;

int main(int argc, char* argv[]) {
    Parser args(argc, argv);
    try {
        // add arguments
        args.add<string>("image", "Path to input image");
        args.add<string>({"-m","--model"}, "Path to model");
        args.add<float>({"-t","--threshold"}, "Detection threshold", 0.5f);
        args.add<bool>({"-v", "--visualize"}, "Visualize results", false);
        args.add<Ints>({"-i","--input-size"}, "Input size", Ints{640, 480});
        args.add<string>({"-o","--output"}, "Output directory", "results/");
        args.add<int>({"-n", "--num-classes"}, "Number of classes", 80);
        args.add<bool>({"-s", "--save-vis"}, "Save visualization images", false);
        args.add<Floats>({"-m","--mean"}, "Mean normalization values", Floats{0.48f, 0.45f, 0.40f});

        // parse arguments
        args.parse();

        // get parsed arguments
        auto image = args.get<string>("image");
        auto model = args.get<string>("model");
        auto threshold = args.get<float>("threshold");
        auto visualize = args.get<bool>("visualize");
        auto inputSize = args.get<Ints>("input-size");
        auto output = args.get<string>("output");
        auto numClasses = args.get<int>("num-classes");
        auto saveVis = args.get<bool>("save-vis");
        auto meanValues = args.get<Floats>("mean");

        // use the arguments...
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << '\n';
        args.printHelp(argv[0]);
        return 1;
    }
    return 0;
}
```

### Full example using named convenience methods
```cpp
#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include "argy.hpp"
using namespace std;
using namespace Argy;

int main(int argc, char* argv[]) {
    Parser args(argc, argv);
    try {
        // add arguments
        args.addString("image", "Path to input image");
        args.addString({"-m","--model"}, "Path to model");
        args.addFloat({"-t","--threshold"}, "Detection threshold", 0.5f);
        args.addBool({"-v", "--visualize"}, "Visualize results", false);
        args.addInts({"-i","--input-size"}, "Input size", Ints{640, 480});
        args.addString({"-o","--output"}, "Output directory", "results/");
        args.addInt({"-n", "--num-classes"}, "Number of classes", 80);
        args.addBool({"-s", "--save-vis"}, "Save visualization images", false);
        args.addFloats({"-m","--mean"}, "Mean normalization values", Floats{0.48f, 0.45f, 0.40f});

        // parse arguments
        args.parse();

        // get parsed arguments
        auto image = args.getString("image");
        auto model = args.getString("model");
        auto threshold = args.getFloat("threshold");
        auto visualize = args.getBool("visualize");
        auto inputSize = args.getInts("input-size");
        auto output = args.getString("output");
        auto numClasses = args.getInt("num-classes");
        auto saveVis = args.getBool("save-vis");
        auto meanValues = args.getFloats("mean");

        // use the arguments...
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << '\n';
        args.printHelp(argv[0]);
        return 1;
    }
    return 0;
}
```

## Example Help Output

Argy prints a help message when you run your program with `--help` or `-h`, or call `args.printHelp(argv[0]);`. The actual output will be colorized and bold in supported terminals. For documentation, here is a simulated colorized version using HTML (may not render everywhere):

<div style="font-family: monospace; background: #222; color: #eee; padding: 1em; border-radius: 6px;">
  <span style="font-weight:bold;">Usage:</span> <span style="color:#eee;">./my_program</span> <span style="color:#00bfff;">&lt;image&gt;</span> [options]<br><br>
  <span style="font-weight:bold;">Positional:</span><br>
  &nbsp;&nbsp;<span style="color:#00bfff;">image</span>                <span style="color:#eee;">Path to input image                <span style="color:#ffd700;">(required)</span></span><br><br>
  <span style="font-weight:bold;">Options:</span><br>
  &nbsp;&nbsp;<span style="color:#00e390;">-m, --model</span>          <span style="color:#eee;">Path to model                      <span style="color:#ffd700;">(required)</span></span><br>
  &nbsp;&nbsp;<span style="color:#00e390;">-t, --threshold</span>      <span style="color:#eee;">Detection threshold                <span style="color:#eee;">(default: 0.5)</span></span><br>
  &nbsp;&nbsp;<span style="color:#00e390;">-v, --visualize</span>      <span style="color:#eee;">Visualize results                  <span style="color:#eee;">(default: false)</span></span><br>
  &nbsp;&nbsp;<span style="color:#00e390;">-i, --input-size</span>     <span style="color:#eee;">Input size                         <span style="color:#eee;">(default: 640, 480)</span></span><br>
  &nbsp;&nbsp;<span style="color:#00e390;">-o, --output</span>         <span style="color:#eee;">Output directory                   <span style="color:#eee;">(default: results/)</span></span><br>
  &nbsp;&nbsp;<span style="color:#00e390;">-n, --num-classes</span>    <span style="color:#eee;">Number of classes                  <span style="color:#eee;">(default: 80)</span></span><br>
  &nbsp;&nbsp;<span style="color:#00e390;">-s, --save-vis</span>       <span style="color:#eee;">Save visualization images          <span style="color:#eee;">(default: false)</span></span><br>
  &nbsp;&nbsp;<span style="color:#00e390;">-m, --mean</span>           <span style="color:#eee;">Mean normalization values          <span style="color:#eee;">(default: 0.48, 0.45, 0.40)</span></span><br>
  &nbsp;&nbsp;<span style="color:#00e390;">-h, --help</span>           <span style="color:#eee;">Show this help message</span><br>
</div>

> **Note:** This HTML block simulates terminal colors. It may not render in all Markdown viewers (e.g., GitHub, VS Code), but works in some web-based Markdown renderers.

## Argument Requirements

- Positional arguments (those without dashes) are always **required** and cannot have default values.
- Named arguments are **required** by default, but become **optional** if you specify a default value.
- Flag arguments (e.g., `--save-vis`) do not require a value; their presence sets them to `true`.

## Contributing
Contributions are welcome! Please open issues or pull requests for bug fixes, features, or improvements.

