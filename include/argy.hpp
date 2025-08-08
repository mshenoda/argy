/*
    MIT License

    Copyright (c) 2025 mshenoda - Michael Shenoda

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.
*/
#pragma once

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <optional>
#include <functional>

namespace Argy {
    // Type aliases for supported vector types
    using Bools = std::vector<bool>;
    using Ints = std::vector<int>;
    using Floats = std::vector<float>;
    using Strings = std::vector<std::string>;
    

    /**
     * @brief Represents a command-line argument name, supporting both short and long forms.
     *
     * This struct is used to specify argument names for the parser, allowing for both a short (single character)
     * and a long (multi-character) name. Either or both can be provided.
     */
    struct ArgName {
        std::string shortName; ///< Short name (e.g., "c" for -c)
        std::string longName;  ///< Long name (e.g., "count" for --count)
    };

    /**
     * @brief Checks if a string starts with a given prefix.
     * @param str The string to check.
     * @param prefix The prefix to look for.
     * @return True if str starts with prefix, false otherwise.
     */
    inline bool startsWith(const std::string& str, const std::string& prefix)
    {
        return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
    }

    /**
     * @class Parser
     * @brief Command-line argument parser inspired by Python's argparse.
     *
     * This class provides a flexible and type-safe way to define, parse, and validate command-line arguments.
     * It supports positional and optional arguments, type validation, default values, required arguments,
     * list arguments, shorthand options, and automatic help message generation.
     *
     * Example usage:
     * @code
     *   int main(int argc, char* argv[]) {
     *     Argy::Parser parser(argc, argv);
     *     parser.addString("filename", "Input file");
     *     parser.addInt("--count", "Number of items", 10);
     *     parser.parse();
     *     std::string file = parser.getString("filename");
     *     int count = parser.getInt("count");
     *   }
     * @endcode
     */
    class Parser {
    public:
        /**
         * @brief Constructs a Parser and sets the default help handler.
         * @param argc Argument count from main().
         * @param argv Argument vector from main().
         *
         * The default help handler prints help and exits. You can override it with setHelpHandler().
         */
        Parser(int argc, char* argv[]) : m_argc(argc), m_argv(argv) {
            m_helpHandler = [this](std::string name) {
                printHelp(name);
                std::exit(0);
            };
        }

        /**
         * @brief Set a custom help handler invoked on --help or -h.
         * @param handler Function to call when help is requested. Receives the program name.
         *
         * The default handler prints help and exits. Override this if you want to return or throw instead.
         */
        void setHelpHandler(std::function<void(std::string)> handler) {
            m_helpHandler = std::move(handler);
        }

        /**
         * @brief Add an argument to the parser (positional or optional).
         * @tparam T Argument type (int, float, bool, string, or vector thereof).
         * @param name Argument name (e.g. "--count" or "filename").
         * @param help Help text for usage.
         * @param defaultValue Optional default value; if omitted, argument is required.
         * @throws std::runtime_error if attempting to override reserved names --help or -h.
         *
         * If the name starts with dashes, it is treated as an optional argument; otherwise, it is positional.
         */
        template<typename T>
        Parser& add(const std::string& name,
            const std::string& help = "",
            std::optional<T> defaultValue = std::nullopt
        ) {
            std::string cleanName = name;
            bool isPositional = true;
            std::string shortName, longName;

            // Enforce naming conventions BEFORE stripping dashes
            if (startsWith(cleanName, "--")) {
                if (cleanName.size() <= 2)
                    throw std::invalid_argument("longName must not be empty after --");
            }
            else if (startsWith(cleanName, "-")) {
                if (cleanName.size() <= 1)
                    throw std::invalid_argument("shortName must not be empty after -");
            }

            // Always strip leading dashes for single-name methods
            if (startsWith(cleanName, "--")) {
                longName = cleanName.substr(2);
                isPositional = false;
            }
            else if (startsWith(cleanName, "-")) {
                shortName = cleanName.substr(1);
                isPositional = false;
            } else {
                longName = cleanName;
            }

            // Prevent overriding help flags
            if (longName == "help" || shortName == "h") {
                throw std::runtime_error("Cannot redefine built-in --help/-h argument");
            }

            // Check for duplicates
            for (const auto& [k, v] : m_arguments) {
                if (!longName.empty() && v.longName == longName)
                    throw std::runtime_error("Duplicate longName: " + longName);
                if (!shortName.empty() && v.shortName == shortName)
                    throw std::runtime_error("Duplicate shortName: " + shortName);
            }

            ArgType type = deduceArgType<T>();
            Value val = defaultValue ? Value(*defaultValue) : Value{};
            bool isRequired = !defaultValue.has_value();

            Argument arg{shortName, longName, help, isRequired, type, val, Value{}, isPositional};
            std::string key = isPositional ? longName : (longName.empty() ? shortName : longName);
            m_arguments[key] = arg;
            if (isPositional)
                m_positionalOrder.push_back(key);
            return *this;
        }

        /**
         * @brief Add an argument to the parser (positional or optional) using ArgName.
         * @tparam T Argument type (int, float, bool, string, or vector thereof).
         * @param argName ArgName object with short and/or long name.
         * @param help Help text for usage.
         * @param defaultValue Optional default value; if omitted, argument is required.
         * @throws std::runtime_error if attempting to override reserved names --help or -h.
         */
        template<typename T>
        Parser& add(const ArgName& argName,
            const std::string& help = "",
            std::optional<T> defaultValue = std::nullopt) {
            std::string shortKey = argName.shortName;
            std::string longKey = argName.longName;
            // Enforce naming conventions BEFORE stripping dashes
            if (!shortKey.empty() && !startsWith(shortKey, "-"))
                throw std::invalid_argument("shortName must start with -");
            if (!longKey.empty() && !startsWith(longKey, "--"))
                throw std::invalid_argument("longName must start with --");
            // Strip dashes for storage
            if (startsWith(shortKey, "-")) shortKey = shortKey.substr(1);
            if (startsWith(longKey, "--")) longKey = longKey.substr(2);
            else if (startsWith(longKey, "-")) longKey = longKey.substr(1);
            ArgType type = deduceArgType<T>();
            Value val = defaultValue ? Value(*defaultValue) : Value{};
            bool isRequired = !defaultValue.has_value();
            bool isPositional = longKey.empty();
            // Check for duplicates
            for (const auto& [k, v] : m_arguments) {
                if (!longKey.empty() && v.longName == longKey)
                    throw std::runtime_error("Duplicate longName: " + longKey);
                if (!shortKey.empty() && v.shortName == shortKey)
                    throw std::runtime_error("Duplicate shortName: " + shortKey);
            }
            Argument arg{shortKey, longKey, help, isRequired, type, val, Value{}, isPositional};
            std::string key = isPositional ? shortKey : longKey;
            m_arguments[key] = arg;
            if (isPositional)
                m_positionalOrder.push_back(key);
            return *this;
        }

        /**
         * @brief Add an argument using an initializer list of names.
         * @tparam T Argument type.
         * @param names List of argument names (one or two: short and/or long).
         * @param help Help text.
         * @param defaultValue Optional default value.
         * @throws std::invalid_argument if the number of names is not one or two.
         */
        template<typename T>
        Parser& add(std::initializer_list<const char*> names,
                 const std::string& help = "",
                 std::optional<T> defaultValue = std::nullopt) {
            auto it = names.begin();
            std::string shortName, longName;
            if (names.size() == 2) {
                shortName = *it;
                ++it;
                longName = *it;
            } else if (names.size() == 1) {
                longName = *it;
            } else {
                throw std::invalid_argument("Must provide one or two argument names");
            }
            add<T>(ArgName{shortName, longName}, help, defaultValue);
            return *this;
        }

        /**
         * @brief Add an argument with both short and long names.
         * @tparam T Argument type.
         * @param shortName Short name (e.g., "-c").
         * @param longName Long name (e.g., "--count").
         * @param help Help text.
         * @param defaultValue Optional default value.
         * @return Reference to this Parser for chaining.
         */
        template<typename T>
        Parser& add(const char* shortName, const char* longName,
                 const std::string& help,
                 std::optional<T> defaultValue = std::nullopt) {
    
            if (Argy::startsWith(shortName, "--") && !Argy::startsWith(longName, "--")) {
                const std::string h = longName;
                const std::string d = help;
                return add<std::string>(shortName, h, d);
            }
            
            return add<T>({shortName, longName}, help, defaultValue);
        }

        // Shorter method names for adding arguments
        /**
         * @name Convenience methods for adding arguments of specific types
         * @{
         */
        Parser& addString(const std::string& name, const std::string& help="", std::optional<std::string> defaultValue = std::nullopt) { 
            return add<std::string>(name, help, defaultValue); 
        }
        Parser& addInt(const char* name, const char* help, std::optional<int> defaultValue = std::nullopt) { 
            return add<int>(name, help, defaultValue);
        }
        Parser& addFloat(const char* name, const char* help, std::optional<float> defaultValue = std::nullopt) { 
            add<float>(name, help, defaultValue); return *this; 
        }
        Parser& addBool(const char* name, const char* help, bool defaultValue = false) { 
            add<bool>(name, help, defaultValue); return *this; 
        }
        Parser& addStrings(const char* name, const char* help, std::optional<std::vector<std::string>> defaultValue = std::nullopt) {
             add<std::vector<std::string>>(name, help, defaultValue); return *this; 
        }
        Parser& addInts(const char* name, const char* help, std::optional<std::vector<int>> defaultValue = std::nullopt) { 
            add<std::vector<int>>(name, help, defaultValue); return *this; 
        }
        Parser& addFloats(const char* name, const char* help, std::optional<std::vector<float>> defaultValue = std::nullopt) { 
            add<std::vector<float>>(name, help, defaultValue); return *this; 
        }
        Parser& addBools(const char* name, const char* help, std::optional<std::vector<bool>> defaultValue = std::nullopt) { 
            add<std::vector<bool>>(name, help, defaultValue); return *this; 
        }
 
        // --- Explicit overloads for named arguments, help, and default value ---
        Parser& addInt(const char* shortName, const char* longName, const std::string& help, std::optional<int> defaultValue = std::nullopt) {
            return add<int>({shortName, longName}, help, defaultValue);
        }
        Parser& addBool(const char* shortName, const char* longName, const std::string& help, bool defaultValue = false) {
            return add<bool>({shortName, longName}, help, defaultValue);
        }
        Parser& addString(const char* shortName, const char* longName, const std::string& help, std::optional<std::string> defaultValue = std::nullopt) {
            std::string s(shortName), l(longName);
            if (Argy::startsWith(s, "--") && !Argy::startsWith(l, "--")) {
                std::string h = l;
                std::string d = help;
                return add<std::string>(s, l, d);
            }
            else if (!Argy::startsWith(s, "-") && !Argy::startsWith(l, "--") && !help.empty()) { // positional with default not allowed
                throw std::invalid_argument("Positional arguments cannot have default values");
            }
            return add<std::string>({shortName, longName}, help, defaultValue);
        }
        Parser& addFloat(const char* shortName, const char* longName, const std::string& help, std::optional<float> defaultValue = std::nullopt) {
            return add<float>({shortName, longName}, help, defaultValue);
        }
        Parser& addInts(const char* shortName, const char* longName, const std::string& help, std::optional<std::vector<int>> defaultValue = std::nullopt) {
            return add<std::vector<int>>({shortName, longName}, help, defaultValue);
        }
        Parser& addBools(const char* shortName, const char* longName, const std::string& help, std::optional<std::vector<bool>> defaultValue = std::nullopt) {
            return add<std::vector<bool>>({shortName, longName}, help, defaultValue);
        }
        Parser& addStrings(const char* shortName, const char* longName, const std::string& help, std::optional<std::vector<std::string>> defaultValue = std::nullopt) {
            return add<std::vector<std::string>>({shortName, longName}, help, defaultValue);
        }
        Parser& addFloats(const char* shortName, const char* longName, const std::string& help, std::optional<std::vector<float>> defaultValue = std::nullopt) {
            return add<std::vector<float>>({shortName, longName}, help, defaultValue);
        }
        /** @} */

        // Overloads for ArgName
        /**
         * @name Overloads for ArgName
         * @{
         */
        Parser& addString(const ArgName& argName, const std::string& help = "", std::optional<std::string> defaultValue = std::nullopt) { add<std::string>(argName, help, defaultValue); return *this; }
        Parser& addInt(const ArgName& argName, const std::string& help = "", std::optional<int> defaultValue = std::nullopt) { add<int>(argName, help, defaultValue); return *this; }
        Parser& addFloat(const ArgName& argName, const std::string& help = "", std::optional<float> defaultValue = std::nullopt) { add<float>(argName, help, defaultValue); return *this; }
        Parser& addBool(const ArgName& argName, const std::string& help = "", std::optional<bool> defaultValue = std::nullopt) { add<bool>(argName, help, defaultValue); return *this; }
        Parser& addStrings(const ArgName& argName, const std::string& help = "", std::optional<std::vector<std::string>> defaultValue = std::nullopt) { add<std::vector<std::string>>(argName, help, defaultValue); return *this; }
        Parser& addInts(const ArgName& argName, const std::string& help = "", std::optional<std::vector<int>> defaultValue = std::nullopt) { add<std::vector<int>>(argName, help, defaultValue); return *this; }
        Parser& addFloats(const ArgName& argName, const std::string& help = "", std::optional<std::vector<float>> defaultValue = std::nullopt) { add<std::vector<float>>(argName, help, defaultValue); return *this; }
        Parser& addBools(const ArgName& argName, const std::string& help = "", std::optional<std::vector<bool>> defaultValue = std::nullopt) { add<std::vector<bool>>(argName, help, defaultValue); return *this; }
        /** @} */

        /**
         * @brief Parse command-line arguments using stored argc/argv.
         *
         * This method processes the command-line arguments, validates types, checks for required arguments,
         * and sets default values where appropriate. Throws on unknown or missing required arguments.
         *
         * @throws std::runtime_error on unknown or missing required arguments.
         */
        void parse() {
            int argc = m_argc;
            char** argv = m_argv;
            // Auto-handle help flags
            for (int i = 1; i < argc; ++i) {
                std::string arg = argv[i];
                if (arg == "--help" || arg == "-h") {
                    m_helpHandler(argv[0]);
                    return;
                }
            }

            std::string currentKey;
            size_t positionalIndex = 0;

            // Parse loop
            for (int i = 1; i < argc; ++i) {
                std::string token = argv[i];
                std::string key;
                if (startsWith(token, "--")) {
                    std::string normKey = token.substr(2);
                    // Find by longName (with or without dashes)
                    auto it = std::find_if(m_arguments.begin(), m_arguments.end(), [&](const auto& pair) {
                        return pair.second.longName == "--" + normKey || pair.second.longName == normKey;
                    });
                    if (it == m_arguments.end()) throw std::runtime_error("Unknown argument: --" + normKey);
                    currentKey = it->first;
                    Argument& arg = it->second;
                    if (isListType(arg.type)) {
                        arg.parsedValue = std::vector<std::string>{};
                        continue;
                    }
                    if (arg.type == ArgType::Bool) {
                        arg.parsedValue = true;
                        currentKey.clear();
                    }
                } else if (startsWith(token, "-") && token.size() > 1) {
                    std::string normKey = token.substr(1);
                    // Find by shortName (with or without dash)
                    auto it = std::find_if(m_arguments.begin(), m_arguments.end(), [&](const auto& pair) {
                        return pair.second.shortName == normKey;
                    });
                    if (it == m_arguments.end()) throw std::runtime_error("Unknown short argument: -" + normKey);
                    currentKey = it->first;
                    Argument& arg = it->second;
                    if (isListType(arg.type)) {
                        arg.parsedValue = std::vector<std::string>{};
                        continue;
                    }
                    if (arg.type == ArgType::Bool) {
                        arg.parsedValue = true;
                        currentKey.clear();
                    }
                } else if (!currentKey.empty()) {
                    Argument& arg = m_arguments.at(currentKey);
                    if (isListType(arg.type)) {
                        if (std::holds_alternative<std::vector<std::string>>(arg.parsedValue)) {
                            std::get<std::vector<std::string>>(arg.parsedValue).push_back(token);
                        }
                    } else {
                        arg.parsedValue = token;
                        currentKey.clear();
                    }
                } else {
                    // Positional argument
                    if (positionalIndex >= m_positionalOrder.size())
                        throw std::runtime_error("Unexpected positional argument: " + token);
                    std::string name = m_positionalOrder[positionalIndex++];
                    Argument& arg = m_arguments.at(name);
                    arg.parsedValue = token;
                }
            }

            // Validate required and set defaults
            for (auto& [key, argument] : m_arguments) {
                if (!isListType(argument.type) && std::holds_alternative<std::monostate>(argument.parsedValue)) {
                    if (argument.required)
                        throw std::runtime_error("Missing required argument: " + argument.longName);
                    argument.parsedValue = argument.defaultValue;
                } else if (isListType(argument.type) && std::holds_alternative<std::monostate>(argument.parsedValue)) {
                    if (argument.required)
                        throw std::runtime_error("Missing required list argument: " + argument.longName);
                    argument.parsedValue = argument.defaultValue;
                } else {
                    if (!isListType(argument.type) && !std::holds_alternative<std::monostate>(argument.parsedValue)) {
                        validateType(toString(argument.parsedValue), argument.type);
                    } else if (isListType(argument.type) && std::holds_alternative<std::vector<std::string>>(argument.parsedValue)) {
                        const auto& vec = std::get<std::vector<std::string>>(argument.parsedValue);
                        for (const auto& v : vec) {
                            validateType(v, argument.type);
                        }
                    }
                }
            }
        }

        /**
         * @brief Get the parsed argument value by name.
         * @tparam T Expected argument type.
         * @param name Argument name.
         * @return Parsed argument value of type T.
         * @throws std::runtime_error if the argument is not found or type conversion fails.
         */
        template<typename T>
        T get(const std::string& name) const {
            // Normalize input name: strip leading dashes
            std::string normName = name;
            if (startsWith(normName, "--")) normName = normName.substr(2);
            else if (startsWith(normName, "-")) normName = normName.substr(1);
            // Try direct key
            auto it = m_arguments.find(name);
            if (it == m_arguments.end()) {
                // Try normalized longName or shortName
                it = std::find_if(m_arguments.begin(), m_arguments.end(), [&](const auto& pair) {
                    return pair.second.longName == "--" + normName || pair.second.longName == normName ||
                           pair.second.shortName == "-" + normName || pair.second.shortName == normName;
                });
                if (it == m_arguments.end()) throw std::runtime_error("Argument not found: " + name);
            }
            const Argument& arg = it->second;
            if constexpr (is_vector<T>::value) {
                if (std::holds_alternative<std::vector<std::string>>(arg.parsedValue)) {
                    return fromStringVector<typename T::value_type>(std::get<std::vector<std::string>>(arg.parsedValue));
                } else if (!std::holds_alternative<std::monostate>(arg.defaultValue)) {
                    // Use default value if available
                    if constexpr (std::is_same_v<T, std::vector<int>>) {
                        return std::get<std::vector<int>>(arg.defaultValue);
                    } else if constexpr (std::is_same_v<T, std::vector<float>>) {
                        return std::get<std::vector<float>>(arg.defaultValue);
                    } else if constexpr (std::is_same_v<T, std::vector<bool>>) {
                        return std::get<std::vector<bool>>(arg.defaultValue);
                    } else if constexpr (std::is_same_v<T, std::vector<std::string>>) {
                        return std::get<std::vector<std::string>>(arg.defaultValue);
                    }
                }
                throw std::runtime_error("Argument list not found: " + name);
            } else {
                if (std::holds_alternative<std::monostate>(arg.parsedValue)) {
                    if (!std::holds_alternative<std::monostate>(arg.defaultValue)) {
                        return fromString<T>(toString(arg.defaultValue));
                    }
                    throw std::runtime_error("Argument not found: " + name);
                }
                return fromString<T>(toString(arg.parsedValue));
            }
        }

        /**
         * @brief Check if an argument was provided on the command line.
         * @param name Argument name.
         * @return True if the argument is present, false otherwise.
         */
        bool has(const std::string& name) const {
            auto it = m_arguments.find(name);
            if (it == m_arguments.end()) {
                it = std::find_if(m_arguments.begin(), m_arguments.end(), [&](const auto& pair) {
                    return pair.second.longName == name || pair.second.shortName == name;
                });
                if (it == m_arguments.end()) return false;
            }
            const Argument& arg = it->second;
            return !std::holds_alternative<std::monostate>(arg.parsedValue);
        }

        /**
         * @name Convenience getters for specific types
         * @{
         */
        int getInt(const std::string& name) const { return get<int>(name); }
        float getFloat(const std::string& name) const { return get<float>(name); }
        bool getBool(const std::string& name) const { return get<bool>(name); }
        std::string getString(const std::string& name) const { return get<std::string>(name); }

        std::vector<int> getInts(const std::string& name) const { return get<std::vector<int>>(name); }
        std::vector<float> getFloats(const std::string& name) const { return get<std::vector<float>>(name); }
        std::vector<bool> getBools(const std::string& name) const { return get<std::vector<bool>>(name); }
        std::vector<std::string> getStrings(const std::string& name) const { return get<std::vector<std::string>>(name); }
        /** @} */

        /**
         * @brief Print help message to stdout.
         * @param programName The program's executable name (usually argv[0]).
         *
         * This prints a usage summary and all registered arguments, including their help text and default values.
         */
        void printHelp(const std::string& programName) const {
            // ANSI color codes
            const char* bold = "\033[1m";
            const char* cyan = "\033[36m";
            const char* yellow = "\033[33m";
            const char* reset = "\033[0m";
            const char* gray = "\033[90m";
            const char* green = "\033[32m";

            // Usage line
            std::cout << bold << "Usage: " << reset << programName;
            for (const auto& positional : m_positionalOrder)
                std::cout << " " << cyan << "<" << positional << ">" << reset;
            std::cout << " [options]\n\n";

            // Section: Positional arguments
            if (!m_positionalOrder.empty()) {
                std::cout << bold << "Positional:" << reset << "\n";
                for (const auto& key : m_positionalOrder) {
                    const auto& argument = m_arguments.at(key);
                    std::cout << "  " << cyan << argument.longName << reset;
                    if (!argument.help.empty())
                        std::cout << "\t" << argument.help;
                    if (!std::holds_alternative<std::monostate>(argument.defaultValue))
                        std::cout << gray << " (default: " << toString(argument.defaultValue) << ")" << reset;
                    std::cout << "\n";
                }
                std::cout << "\n";
            }

            // Section: Options
            std::cout << bold << "Options:" << reset << "\n";
            // Find max width for alignment
            size_t maxOptLen = 0;
            std::vector<std::string> optStrings;
            for (const auto& [key, argument] : m_arguments) {
                if (!argument.positional) {
                    std::string opt;
                    if (!argument.shortName.empty())
                        opt = "-" + argument.shortName + ", --" + argument.longName;
                    else
                        opt = "    --" + argument.longName; // 4 spaces for alignment
                    if (argument.type != ArgType::Bool)
                        opt += " <value>";
                    if (opt.size() > maxOptLen) maxOptLen = opt.size();
                    optStrings.push_back(opt);
                }
            }
            // Also consider help flag in maxOptLen
            std::string helpFlag = "-h, --help";
            if (helpFlag.size() > maxOptLen) maxOptLen = helpFlag.size();

            size_t optIdx = 0;
            for (const auto& [key, argument] : m_arguments) {
                if (!argument.positional) {
                    const std::string& opt = optStrings[optIdx++];
                    std::cout << "  " << green << opt << reset;
                    size_t pad = maxOptLen > opt.size() ? maxOptLen - opt.size() : 0;
                    std::cout << std::string(pad + 2, ' ');
                    if (!argument.help.empty())
                        std::cout << argument.help;
                    if (!std::holds_alternative<std::monostate>(argument.defaultValue))
                        std::cout << gray << " (default: " << toString(argument.defaultValue) << ")" << reset;
                    if (argument.required)
                        std::cout << " " << yellow << "(required)" << reset;
                    std::cout << "\n";
                }
            }
            // Help flag, aligned
            size_t helpPad = maxOptLen > helpFlag.size() ? maxOptLen - helpFlag.size() : 0;
            std::cout << "  " << green << helpFlag << reset << std::string(helpPad + 2, ' ') << "Show this help message\n";
        }

    private:  
        /**
         * @brief Variant to hold any supported argument value type.
         *
         * This variant is used for storing argument values of different types, including lists.
         */
        using Value = std::variant<
            std::monostate,      ///< No value
            std::string,         ///< String value
            int,                 ///< Integer value
            float,               ///< Floating-point value
            bool,                ///< Boolean value
            std::vector<std::string>, ///< List of strings
            std::vector<int>,         ///< List of integers
            std::vector<float>,       ///< List of floats
            std::vector<bool>>;       ///< List of booleans

        /**
         * @enum ArgType
         * @brief Supported argument types for validation and parsing.
         */
        enum class ArgType {
            String,     ///< Single string value
            Int,        ///< Single integer value
            Float,      ///< Single float value
            Bool,       ///< Single boolean value
            StringList, ///< List of strings
            IntList,    ///< List of integers
            FloatList,  ///< List of floats
            BoolList    ///< List of booleans
        };

        /**
         * @struct Argument
         * @brief Represents one command-line argument and its metadata.
         */
        struct Argument {
            std::string shortName;   
            std::string longName;   
            std::string help;       ///< Help/description string.
            bool required{ true };  ///< True if argument must be provided by the user.
            ArgType type{ ArgType::String }; ///< Argument type.
            Value defaultValue;     ///< Default value if any.
            Value parsedValue;     ///< Parsed value if any.
            bool positional{ false }; ///< True if this is a positional argument.
        };

        std::unordered_map<std::string, Argument> m_arguments; ///< Map of all arguments.
        std::vector<std::string> m_positionalOrder; ///< Order of positional arguments.
        std::function<void(std::string)> m_helpHandler; ///< Function to handle help requests.
        int m_argc; ///< Argument count from main().
        char** m_argv; ///< Argument vector from main().

        /**
         * @brief Throws if unknown or positional argument used as option.
         * @param key Argument key to check.
         * @throws std::runtime_error if the argument is unknown or positional.
         */
        void ensureKnown(const std::string& key) const {
            if (!m_arguments.count(key) || m_arguments.at(key).positional)
                throw std::runtime_error("Unknown argument: --" + key);
        }

        /**
         * @brief Converts Value variant to string for defaults and printing.
         * @param value The Value to convert.
         * @return String representation of the value.
         */
        static std::string toString(const Value& value) {
            if (std::holds_alternative<std::string>(value)) return std::get<std::string>(value);
            if (std::holds_alternative<int>(value)) return std::to_string(std::get<int>(value));
            if (std::holds_alternative<float>(value)) return std::to_string(std::get<float>(value));
            if (std::holds_alternative<bool>(value)) return std::get<bool>(value) ? "true" : "false";
            if (std::holds_alternative<std::vector<std::string>>(value)) {
                const auto& vec = std::get<std::vector<std::string>>(value);
                std::string out = "[";
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (i > 0) out += ", ";
                    out += '"' + vec[i] + '"';
                }
                out += "]";
                return out;
            }
            if (std::holds_alternative<std::vector<int>>(value)) {
                const auto& vec = std::get<std::vector<int>>(value);
                std::string out = "[";
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (i > 0) out += ", ";
                    out += std::to_string(vec[i]);
                }
                out += "]";
                return out;
            }
            if (std::holds_alternative<std::vector<float>>(value)) {
                const auto& vec = std::get<std::vector<float>>(value);
                std::string out = "[";
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (i > 0) out += ", ";
                    out += std::to_string(vec[i]);
                }
                out += "]";
                return out;
            }
            if (std::holds_alternative<std::vector<bool>>(value)) {
                const auto& vec = std::get<std::vector<bool>>(value);
                std::string out = "[";
                for (size_t i = 0; i < vec.size(); ++i) {
                    if (i > 0) out += ", ";
                    out += vec[i] ? "true" : "false";
                }
                out += "]";
                return out;
            }
            return "";
        }

        /**
         * @brief Validate a string against the expected argument type.
         * @param val The string value to check.
         * @param type The expected argument type.
         * @throws std::invalid_argument if validation fails.
         */
        static void validateType(const std::string& val, ArgType type) {
            switch (type) {
            case ArgType::Int: (void)std::stoi(val); break;
            case ArgType::Float: (void)std::stof(val); break;
            case ArgType::Bool:
                if (val != "true" && val != "false" && val != "1" && val != "0")
                    throw std::invalid_argument("Expected boolean value");
                break;
            default:
                break;
            }
        }

        /**
         * @brief Convert string to value of type T.
         * @tparam T Target type.
         * @param value String value to convert.
         * @return Value of type T.
         */
        template<typename T>
        static T fromString(const std::string& value);

        /**
         * @brief Convert vector of strings to vector of type T.
         * @tparam T Target type.
         * @param vec Vector of strings to convert.
         * @return Vector of type T.
         */
        template<typename T>
        static std::vector<T> fromStringVector(const std::vector<std::string>& vec) {
            std::vector<T> result;
            for (const auto& s : vec)
                result.push_back(fromString<T>(s));
            return result;
        }

        /**
         * @brief Helper type trait to detect std::vector types.
         * @tparam T Type to check.
         */
        template<typename T>
        struct is_vector : std::false_type {};

        template<typename T, typename A>
        struct is_vector<std::vector<T, A>> : std::true_type {};

        /**
         * @brief Check if an ArgType represents a list type.
         * @param type The ArgType to check.
         * @return True if the type is a list type, false otherwise.
         */
        static bool isListType(ArgType type) {
            return type == ArgType::StringList || type == ArgType::IntList ||
                type == ArgType::FloatList || type == ArgType::BoolList;
        }

        /**
         * @brief Deduce ArgType enum from C++ type.
         * @tparam T C++ type to deduce from.
         * @return Corresponding ArgType value.
         */
        template<typename T>
        static constexpr ArgType deduceArgType() {
            if constexpr (std::is_same_v<T, int>) return ArgType::Int;
            else if constexpr (std::is_same_v<T, float>) return ArgType::Float;
            else if constexpr (std::is_same_v<T, bool>) return ArgType::Bool;
            else if constexpr (std::is_same_v<T, std::string>) return ArgType::String;
            else if constexpr (std::is_same_v<T, std::vector<int>>) return ArgType::IntList;
            else if constexpr (std::is_same_v<T, std::vector<float>>) return ArgType::FloatList;
            else if constexpr (std::is_same_v<T, std::vector<bool>>) return ArgType::BoolList;
            else if constexpr (std::is_same_v<T, std::vector<std::string>>) return ArgType::StringList;
            else static_assert(sizeof(T) == 0, "Unsupported argument type");
        }       
    };

    template<>
    inline int Parser::fromString<int>(const std::string& value) { return std::stoi(value); }
    template<>
    inline float Parser::fromString<float>(const std::string& value) { return std::stof(value); }
    template<>
    inline bool Parser::fromString<bool>(const std::string& value) { return value == "true" || value == "1"; }
    template<>
    inline std::string Parser::fromString<std::string>(const std::string& value) { return value; }
}
