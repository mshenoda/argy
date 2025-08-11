<p align="center">
  <img src="logo.svg" alt="argy logo" width="100"/>
</p>

<p align="center">
  <a href="LICENSE">
    <img src="https://img.shields.io/github/license/mshenoda/argy?color=blue" alt="License"/>
  </a>
    <a href="https://github.com/mshenoda/argy/releases">
    <img src="https://img.shields.io/github/v/tag/mshenoda/argy?label=release" alt="Release"/>
    </a>
  <a href="https://github.com/mshenoda/argy/actions/workflows/ci-linux.yml">
    <img src="https://github.com/mshenoda/argy/actions/workflows/ci-linux.yml/badge.svg" alt="Linux Build"/>
  </a>
  <a href="https://github.com/mshenoda/argy/actions/workflows/ci-windows.yml">
    <img src="https://github.com/mshenoda/argy/actions/workflows/ci-windows.yml/badge.svg" alt="Windows Build"/>
  </a>
  <a href="https://github.com/mshenoda/argy/actions/workflows/ci-macos.yml">
    <img src="https://github.com/mshenoda/argy/actions/workflows/ci-macos.yml/badge.svg" alt="macOS Build"/>
  </a>
  <p align="center">
    <a href="https://godbolt.org/z/1WrjPjq8r" target="_blank" rel="noopener noreferrer">
      <img src="https://img.shields.io/badge/Godbolt-Demo-blue?logo=compilerexplorer" alt="Try Online"/>
    </a>
  </p>
</p>

## Argy

**Argy: Command-Line Parsing Library for Modern C++**<br>
*Because life’s too short to parse argv by hand!*

Effortless, intuitive, and lightweight command-line argument parsing for C++17 and newer. Argy is a header-only library with zero dependencies, designed to make CLI development simple and type-safe.

*If you love Python's argparse or click, you'll feel right at home—Argy brings that developer-friendly experience to C++.* 

Perfect for building robust, maintainable command-line tools in modern C++.

## Online Interactive Demo

Try argy in your browser using Compiler Explorer:
<a href="https://godbolt.org/z/1WrjPjq8r" target="_blank" rel="noopener noreferrer">Open Interavtive Demo on godbolt.org</a>


## Features
- 📦 **Header-only** & zero dependencies
- 🧩 **Intuitive API** for defining and parsing command-line arguments
- 🎯 Supports positional, optional, and flag arguments
- 🔒 **Type-safe** access to parsed values
- 📝 Customizable help output

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
#include "argy.hpp"
Argy::CliParser cli(argc, argv);
cli.add<std::string>("image", "Path to input image");            // Positional argument
cli.add<int>("-n", "--num-classes", "Number of classes", 80);  // Named int argument, with default
cli.add<bool>("-s", "--save-vis", "Save visualization images"); // Named bool argument
```

### Convenience Methods
Use named methods for clarity:
```cpp
Argy::CliParser cli(argc, argv);
cli.addString("image", "Path to input image");   // Positional argument
cli.addInt("-n", "--num-classes", "Number of classes", 80);  // Named int argument, with default
cli.addBool("-s", "--save-vis", "Save visualization images"); // Named bool argument
```

### Parsing and Accessing Arguments
Typical usage pattern:
```cpp
try {
  cli.parse();
  auto image = cli.getString("image");
  auto threshold = cli.getFloat("threshold");
} catch (const Argy::Exception& ex) {
  std::cerr << "Error: " << ex.what() << '\n';
  cli.printHelp(argv[0]);
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

int main(int argc, char* argv[]) {
  Argy::CliParser cli(argc, argv);
  try {
    // add arguments using template methods
    cli.add<std::string>("image", "Path to input image");
    cli.add<std::string>("-m", "--model", "Path to model");
    cli.add<float>("-t", "--threshold", "Detection threshold", 0.5f);
    cli.add<bool>("-v", "--visualize", "Visualize results");
    cli.add<Argy::Ints>("-i", "--input-size", "Input size", Argy::Ints{640, 480});
    cli.add<std::string>("-o", "--output", "Output directory", "results/");
    cli.add<int>("-n", "--num-classes", "Number of classes", 80);

    // parse arguments
    cli.parse();

    // get parsed arguments
    auto image = cli.get<std::string>("image");
    auto model = cli.get<std::string>("model");
    auto threshold = cli.get<float>("threshold");
    auto visualize = cli.get<bool>("visualize");
    auto inputSize = cli.get<Argy::Ints>("input-size");
    auto output = cli.get<std::string>("output");
    auto numClasses = cli.get<int>("num-classes");

    // use the arguments...
  } catch (const Argy::Exception& ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    cli.printHelp(argv[0]);
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

int main(int argc, char* argv[]) {
  Argy::CliParser cli(argc, argv);
  try {
    // add arguments
    cli.addString("image", "Path to input image");
    cli.addString("-m","--model", "Path to model");
    cli.addFloat("-t","--threshold", "Detection threshold", 0.5f);
    cli.addBool("-v", "--visualize", "Visualize results");
    cli.addInts("-i","--input-size", "Input size", Argy::Ints{640, 480});
    cli.addString("-o","--output", "Output directory", "results/");
    cli.addInt("-n", "--num-classes", "Number of classes", 80);

    // parse arguments
    cli.parse();

    // get parsed arguments
    auto image = cli.getString("image");
    auto model = cli.getString("model");
    auto threshold = cli.getFloat("threshold");
    auto visualize = cli.getBool("visualize");
    auto inputSize = cli.getInts("input-size");
    auto output = cli.getString("output");
    auto numClasses = cli.getInt("num-classes");

    // use the arguments...
  } catch (const Argy::Exception& ex) {
    std::cerr << "Error: " << ex.what() << '\n';
    cli.printHelp(argv[0]);
    return 1;
  }
  return 0;
}
```

## Example Help Output

Argy prints a help message when you run your program with `--help` or `-h`, or call `args.printHelp(argv[0]);`. The actual output will be colorized and bold in supported terminals. Here is the plain text of help message:

```
Usage: ./my_program <image> [options]

Positional:
  image     Path to input image  (required)

Options:
  -m, --model       <string> Path to model             (required)
  -t, --threshold   <float>  Detection threshold       (default: 0.5)
  -v, --visualize            Visualize results         (default: false)
  -i, --input-size  <int[]>  Input size                (default: [640, 480])
  -o, --output      <string> Output directory          (default: results/)
  -n, --num-classes <int>    Number of classes         (default: 80)
  -s, --save-vis             Save visualization images 
  -h, --help                 Show this help message
```

> **Note:** The actual output in your terminal will be colorized and bold if ANSI colors are supported.

## Argument Requirements & API Notes

- The main parser class is now `CliParser`.
- Exception types are more granular: `ReservedArgumentException`, `DuplicateArgumentException`, `InvalidArgumentException`, `UnknownArgumentException`, `MissingArgumentException`, `TypeMismatchException`, `UnexpectedPositionalArgumentException`, `InvalidValueException`.
- Type aliases for vector arguments: `Bools`, `Ints`, `Floats`, `Strings`.
- Positional arguments (those without dashes) are always **required** and cannot have default values.
- Named arguments are **required** by default, but become **optional** if you specify a default value.
- Flag arguments (e.g., `--save-vis`) do not require a value; their presence sets them to `true`.
- The parser auto-handles `--help` and `-h` flags and prevents overriding them.
- Convenience getters: `getInt`, `getFloat`, `getBool`, `getString`, `getInts`, etc.
- Argument naming conventions are strictly enforced (e.g., `--long` for long names, `-s` for short names).

## Contributing
Contributions are welcome! Please open issues or pull requests for bug fixes, features, or improvements.

## Citation

If you use this project in your research, please cite it as follows:

```bibtex
@misc{mshenoda_argy,
  author = {Michael Shenoda},
  title = {Argy: Effortless Command-Line Parsing for Modern C++},
  year = {2025},
  url = {https://github.com/mshenoda/argy},
  note = {Author website: https://mshenoda.com}
}
```
