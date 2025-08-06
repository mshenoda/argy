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

## Usage

### 1. Template Methods
Define arguments using the template `add<T>()` method:
```cpp
#include <argy.hpp>
using namespace std;
Argy::Parser args(argc, argv);
// Positional argument (no dashes)
args.add<string>("image", "Path to input image");
// Both short and long name (optional argument)
args.add<string>({"-m", "--model"}, "Path to model weights");
args.add<float>({"-t", "--threshold"}, "Detection threshold");
// Long name only (optional argument)
args.add<bool>("--visualize", "Show detection results");
args.add<vector<int>>("--input-size", "Input size as width height", Argy::Ints{640, 480});
```

### 2. Named Convenience Methods
Define arguments using named convenience methods:
```cpp
// Positional argument (no dashes)
args.addString("image", "Path to input image");
// Long name only
args.addString("--output", "Output directory", "results/");
// Both short and long name
args.addInt({"-n", "--num-classes"}, "Number of classes", 80);
args.addBool({"-s", "--save-vis"}, "Save visualization images", false);
args.addFloats("--mean", "Mean normalization values", {0.485, 0.456, 0.406});
```

### 3. Chaining Methods
You can chain either template or convenient methods for a fluent API:
```cpp
args.add<string>("image", "Path to input image")
    .add<string>({"-m", "--model"}, "Path to model weights")
    .add<float>({"-t", "--threshold"}, "Detection threshold")
    .add<bool>({"-v", "--visualize"}, "Show detection results");

// Or chain convenient methods as above
```

### 4. Parsing and Accessing Arguments

Example command line:
```sh
./my_program image.jpg --model model.onnx --threshold 0.5 --visualize --input-size 640 480
```

```cpp
try {
    args.parse();
    auto image = args.get<string>("image");
    auto model = args.get<string>("model");
    auto threshold = args.get<float>("threshold");
    auto visualize = args.get<bool>("visualize");
    auto inputSize = args.get<vector<int>>("input-size");
    auto output = args.getString("output");
    auto numClasses = args.getInt("num-classes");
    auto saveVis = args.getBool("save-vis");
    auto meanValues = args.getFloats("mean");
} catch (const exception& ex) {
    cerr << "Error: " << ex.what() << '\n';
    args.printHelp(argv[0]);
    return 1;
}
```


### Full main example
```cpp
#include <iostream>
#include <string>
#include <vector>
#include <exception>
#include "argy.hpp"

int main(int argc, char* argv[]) {
    Argy::Parser args(argc, argv);
    try {
        // add arguments
        args.addString("image", "Path to input image");
        args.addString({"-m","--model"}, "Path to model");
        args.addFloat({"-t","--threshold"}, "Detection threshold", 0.5f);
        args.addBool({"-v", "--visualize"}, "Visualize results", false);
        args.addInts({"-i","--input-size"}, "Input size", Argy::Ints{640, 480});
        args.addString({"-o","--output"}, "Output directory", "results/");
        args.addInt({"-n", "--num-classes"}, "Number of classes", 80);
        args.addBool({"-s", "--save-vis"}, "Save visualization images", false);
        args.addFloats({"-m","--mean"}, "Mean normalization values", Argy::Floats{0.48f, 0.45f, 0.40f});

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
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << '\n';
        args.printHelp(argv[0]);
        return 1;
    }
    return 0;
}
```

## Argument Requirements

- Positional arguments (those without dashes) are always **required** and cannot have default values.
- Named arguments are **required** by default, but become **optional** if you specify a default value.
- Flag arguments (e.g., `--save-vis`) do not require a value; their presence sets them to `true`.

## Contributing
Contributions are welcome! Please open issues or pull requests for bug fixes, features, or improvements.
