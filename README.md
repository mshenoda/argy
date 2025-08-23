<div align="center">
  <img src="logo.svg" alt="argy logo" width="100"/>
</div>

<div align="center">

  **Argy: Command-Line Argument Parsing Library for Modern C++**<br>
  *Because life's too short to parse argv by hand!*
</div>

<p align="center">
  <a href="LICENSE">
    <img src="https://img.shields.io/github/license/mshenoda/argy?color=blue" alt="License"/>
  </a>
    <a href="https://github.com/mshenoda/argy/releases">
    <img src="https://img.shields.io/github/v/tag/mshenoda/argy?label=release" alt="Release"/>
    </a>
  <a href="https://github.com/mshenoda/argy/actions/workflows/ci-linux.yml">
    <img src="https://img.shields.io/github/actions/workflow/status/mshenoda/argy/ci-linux.yml?label=Linux&logo=linux" alt="Linux Build"/>
  </a>
  <a href="https://github.com/mshenoda/argy/actions/workflows/ci-windows.yml">
    <img src="https://img.shields.io/github/actions/workflow/status/mshenoda/argy/ci-windows.yml?label=Windows&logo=windows" alt="Windows Build"/>
  </a>
  <a href="https://github.com/mshenoda/argy/actions/workflows/ci-macos.yml">
    <img src="https://img.shields.io/github/actions/workflow/status/mshenoda/argy/ci-macos.yml?label=macOS&logo=apple" alt="macOS Build"/>
  </a>
    <a href="https://github.com/mshenoda/argy/actions/workflows/ci-freebsd.yml">
    <img src="https://img.shields.io/github/actions/workflow/status/mshenoda/argy/ci-freebsd.yml?label=FreeBSD&logo=freebsd" alt="FreeBSD Build"/>
    </a>
  <p align="center">
    <a href="https://godbolt.org/z/P8xj3xfYW" target="_blank" rel="noopener noreferrer">
      <img src="https://img.shields.io/badge/Godbolt-Demo-blue?logo=compilerexplorer" alt="Try Online"/>
    </a>
  </p>
</p>

## Argy
Effortless, intuitive, and lightweight command-line argument parsing for C++17 and newer. Argy is a header-only library with zero dependencies, designed to make CLI development simple and type-safe.

*If you love Python's argparse, you'll feel right at home—Argy brings that developer-friendly experience to C++.* 

Perfect for building robust, maintainable command-line tools in modern C++.

## ✨ Features
- 📦 **Header-only** & zero dependencies, just `#include "argy.hpp"` and go
- 🧩 **Intuitive API** for defining and parsing command-line arguments. 
- 🎯 **Argument Types** Supports positional, optional, and flag arguments
- 🔄 **Flexible Naming**: Support for multiple aliases per argument (e.g., `-v`, `--verbose`, `--debug-mode`)
- 🔒 **Type-safe** access to parsed values
- 📋 **List Support**: Native support for vector arguments with validation
- 🛡️ **Built-in Validation**: Rich set of validators for common patterns (emails, URLs, file paths, ranges, etc.)
- **🎨 Beautiful Help**: Auto-generated colorized help messages that look professional out of the box
- **🎪 Python-like Experience**: Familiar API if you've used Python's argparse

## Demo: 
🌐 **[Try Argy in your browser on Compiler Explorer!](https://godbolt.org/z/P8xj3xfYW)**


## 📦 Installation

### CMake (Recommended)

**With FetchContent:**
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

### Header-Only (Any C++ Project Type)
Simply copy `include/argy.hpp` into your project and `#include "argy.hpp"`.


## 🚀 Quick Start

```cpp
#include <iostream>
#include "argy.hpp"

int main(int argc, char* argv[]) {
  Argy::CliParser cli(argc, argv);
    
  // Define arguments
  cli.addString("input", "Input file path");
  cli.addInt({"-n", "--count"}, "Number of items", 10);
  cli.addBool({"-v", "--verbose"}, "Enable verbose output");
    
  // Parse and use
  auto args = cli.parse();
    
  std::cout << "Input: " << args.get<std::string>("input") << "\n";
  std::cout << "Count: " << args.get<int>("count") << "\n";
  std::cout << "Verbose: " << (args.get<bool>("verbose") ? "true" : "false") << "\n";
    
  return 0;
}
```

**That's it!** Compile and run:
```bash
./my_app input.txt --count 42 --verbose
```
## 📖 API Guide

Argy offers two equivalent APIs - choose the one you prefer:

### Template API (Type-Safe)
```cpp
cli.add<std::string>("file", "Input file");                    // Positional
cli.add<int>({"-n", "--count"}, "Number of items", 10);        // Optional with default
cli.add<bool>({"-v", "--verbose"}, "Enable verbose output");   // Flag

// Access values (using ParsedArgs)
auto args = cli.parse();
auto file = args.get<std::string>("file");
auto count = args.get<int>("count");
auto verbose = args.get<bool>("verbose");
```

### Named Methods API (Explicit)
```cpp
cli.addString("file", "Input file");                           // Positional
cli.addInt({"-n", "--count"}, "Number of items", 10);          // Optional with default
cli.addBool({"-v", "--verbose"}, "Enable verbose output");     // Flag

// Access values (using ParsedArgs)
auto args = cli.parse();
auto file = args.get<std::string>("file");
auto count = args.get<int>("count");
auto verbose = args.get<bool>("verbose");
```

### Supported Types
| Type | Template API | Named API | Example |
|------|-------------|-----------|---------|
| String | `add<std::string>()` | `addString()` | `"hello"` |
| Integer | `add<int>()` | `addInt()` | `42` |
| Float | `add<float>()` | `addFloat()` | `3.14f` |
| Boolean | `add<bool>()` | `addBool()` | `true` |
| String List | `add<Argy::Strings>()` | `addStrings()` | `{"a", "b", "c"}` |
| Integer List | `add<Argy::Ints>()` | `addInts()` | `{1, 2, 3}` |
| Float List | `add<Argy::Floats>()` | `addFloats()` | `{1.0f, 2.0f}` |
| Boolean List | `add<Argy::Bools>()` | `addBools()` | `{true, false}` |

## 🛡️ Built-in Validation

Argy provides rich validation capabilities for robust argument parsing:

### Basic Validators
```cpp
// Range validation
cli.add<int>({"-p", "--port"}, "Port number", 8080)
   .validate(Argy::IsValueInRange(1024, 65535));

// Choice validation  
cli.add<std::string>({"-m", "--mode"}, "Processing mode", "normal")
   .validate(Argy::IsOneOf({"normal", "fast", "debug"}));

// Regex validation
cli.add<std::string>({"-t", "--token"}, "API token")
   .validate(Argy::IsMatch(R"([A-Za-z0-9]{32})"));
```

### String Pattern Validators
```cpp
// Common patterns
cli.addString({"-e", "--email"}, "Email address").validate(Argy::IsEmail());
cli.addString({"-u", "--url"}, "API endpoint").validate(Argy::IsUrl());
cli.addString({"--ip"}, "Server IP").validate(Argy::IsIPAddress());
cli.addString({"--mac"}, "MAC address").validate(Argy::IsMACAddress());
cli.addString({"--uuid"}, "UUID").validate(Argy::IsUUID());

// Character type validation
cli.addString({"--alpha"}, "Letters only").validate(Argy::IsAlpha());
cli.addString({"--numeric"}, "Numbers only").validate(Argy::IsNumeric());
cli.addString({"--alphanum"}, "Alphanumeric").validate(Argy::IsAlphaNumeric());
```

### File System Validators
```cpp
cli.addString({"-f", "--file"}, "Input file").validate(Argy::IsFile());
cli.addString({"-d", "--dir"}, "Output directory").validate(Argy::IsDirectory());
cli.addString({"-p", "--path"}, "File or directory").validate(Argy::IsPath());
```

### Vector Validation
```cpp
// Validate all elements in a vector
cli.add<Argy::Ints>({"-i", "--ids"}, "User IDs")
   .validate(Argy::IsVectorInRange(1, 1000000));
```

### Custom Validators
```cpp
// Custom validation lambda function
cli.add<float>({"-r", "--ratio"}, "Ratio value")
   .validate([](const std::string& name, float value) {
       if (value < 0.0f || value > 1.0f) {
           throw Argy::InvalidValueException("Ratio must be between 0.0 and 1.0");
       }
   });
```

## 📋 Complete Examples

### Real-World Example: Image Processing Tool

```cpp
#include "argy.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    Argy::CliParser cli(argc, argv);
    
    try {
        // Positional arguments (required)
        cli.addString("input", "Input image path")
           .validate(Argy::IsFile());
        
        // Optional arguments with defaults
        cli.addString({"-o", "--output"}, "Output directory", "./results/")
           .validate(Argy::IsDirectory());
        cli.addInt({"-q", "--quality"}, "JPEG quality (1-100)", 85)
           .validate(Argy::IsValueInRange(1, 100));
        cli.addString({"-f", "--format"}, "Output format", "jpg")
           .validate(Argy::IsOneOf({"jpg", "png", "webp", "tiff"}));
        
        // Flags (boolean)
        cli.addBool({"-v", "--verbose"}, "Enable verbose output");
        cli.addBool({"-p", "--preview"}, "Generate preview images");
        
        // Vector arguments
        cli.addInts({"-s", "--sizes"}, "Output sizes", Argy::Ints{800, 1200})
           .validate(Argy::IsVectorInRange(100, 4000));
        cli.addStrings({"-t", "--tags"}, "Image tags", Argy::Strings{"processed"});
        
        // Parse command line
        cli.parse();
        
        // Use parsed values
        std::cout << "Processing: " << cli.getString("input") << "\n";
        std::cout << "Output: " << cli.getString("output") << "\n";
        std::cout << "Quality: " << cli.getInt("quality") << "%\n";
        std::cout << "Format: " << cli.getString("format") << "\n";
        
        if (cli.getBool("verbose")) {
            std::cout << "Verbose mode enabled\n";
        }
        
        auto sizes = cli.getInts("sizes");
        std::cout << "Output sizes: ";
        for (auto size : sizes) std::cout << size << "px ";
        std::cout << "\n";
        
    } catch (const Argy::Exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        cli.printHelp(argv[0]);
        return 1;
    }
    
    return 0;
}
```

**Usage examples:**
```bash
# Basic usage
./image_tool input.jpg

# With options
./image_tool input.jpg --output ./processed --quality 95 --format png

# With verbose and multiple sizes
./image_tool input.jpg -v --sizes 400 800 1600 --tags "landscape" "hdr"

# Help
./image_tool --help
```

## 📺 Beautiful Help Output

Argy automatically generates beautiful, colorized help messages. When you run your program with `--help` or `-h`:

```
Usage: ./image_tool <input> [options]

Positional:
  input     Input image path  (required)

Options:
  -o, --output   <string>   Output directory          (default: ./results)
  -q, --quality  <int>      JPEG quality (1-100)      (default: 85)
  -f, --format   <string>   Output format             (default: jpg)
  -v, --verbose             Enable verbose output     (default: false)
  -p, --preview             Generate preview images   (default: false)
  -s, --sizes    <int[]>    Output sizes              (default: [800, 1200])
  -t, --tags     <string[]> Image tags                (default: ["processed"])
  -h, --help                Show this help message
```

> **Note:** The actual output in your terminal will be beautifully colorized with proper highlighting!

## 🔧 Advanced Features

### Multiple Aliases
Arguments can have multiple names for better UX:
```cpp
cli.add<bool>({"-v", "--verbose", "--debug-mode"}, "Enable detailed output");
// Works with: -v, --verbose, or --debug-mode
```

### Argument Presence Checking
```cpp
auto args = cli.parse();
if (args.has("verbose")) {
  std::cout << "Verbose mode was explicitly enabled\n";
}
```

### Custom Help Handler
```cpp
cli.setHelpHandler([](const std::string& programName) {
    std::cout << "Custom help for " << programName << "\n";
    // custom handler logic
});
```

### Error Handling
```cpp
try {
  auto args = cli.parse();
} catch (const Argy::MissingArgumentException& ex) {
  std::cerr << "Missing required argument: " << ex.what() << "\n";
} catch (const Argy::InvalidValueException& ex) {
  std::cerr << "Invalid value: " << ex.what() << "\n";
} catch (const Argy::Exception& ex) {
  std::cerr << "Argy error: " << ex.what() << "\n";
}
```

### Type Aliases
For convenience, Argy provides type aliases:
```cpp
using Argy::Strings;  // std::vector<std::string>
using Argy::Ints;     // std::vector<int>
using Argy::Floats;   // std::vector<float>
using Argy::Bools;    // std::vector<bool>
```

## 📚 Key Concepts

### Argument Types
- **Positional**: Arguments without dashes (e.g., `filename`) - can be required or optional
- **Optional**: Named arguments with dashes (e.g., `--count`, `-v`)
- **Flags**: Boolean arguments that don't need values (e.g., `--verbose`)

### Argument Requirements
- **Arguments without default values** are required
- **Arguments with default values** are optional
- **Boolean flags** default to `false` and become `true` when specified

### Exception Types
Argy provides specific exception types for better error handling:
- `MissingArgumentException` - Required argument not provided
- `InvalidValueException` - Value cannot be converted or fails validation
- `UnknownArgumentException` - Unrecognized argument
- `TypeMismatchException` - Type conversion error
- `OutOfRangeException` - Value outside expected range

## 🎯 Best Practices

1. **Use validation** for robust input handling
2. **Provide sensible defaults** for optional arguments
3. **Use descriptive help text** for better UX
4. **Handle exceptions gracefully** with proper error messages
5. **Test with `--help`** to ensure good documentation

## 🤝 Contributing

We welcome contributions! Whether it's:
- 🐛 **Bug reports** - Found an issue? Let us know!
- 💡 **Feature requests** - Have an idea? We'd love to hear it!
- 🔧 **Pull requests** - Want to contribute code? Awesome!
- 📚 **Documentation** - Help make Argy even easier to use!

Please check our [issues page](https://github.com/mshenoda/argy/issues) or open a new one.

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