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
#include <algorithm>

namespace Argy {
    
    /*
    * @class Exception
    * @brief Base class for all exceptions in the Argy library.
    */
    class Exception : public std::exception {
    public:
        explicit Exception(const std::string& message) : m_message(message) {}
        const char* what() const noexcept override { return m_message.c_str(); }

    private:
        std::string m_message;
    };

    /**
     * @class DefineException
     * @brief Base class for exceptions related to argument definition errors.
     */
    class DefineException : public Exception {
        using Exception::Exception;
    };

    /**
     * @class ParseException
     * @brief Base class for exceptions related to argument parsing errors.
     */
    class ReservedArgumentException : public DefineException {
        using DefineException::DefineException;
    };

    /**
     * @class DuplicateArgumentException   
     * @brief Exception thrown when an argument is defined multiple times.
     */
    class DuplicateArgumentException : public DefineException {
        using DefineException::DefineException;
    };

    /**
     * @class InvalidArgumentException
     * @brief Exception thrown when an argument definition is invalid.
     */
    class InvalidArgumentException : public DefineException {
        using DefineException::DefineException;
    };

    /**
     * @class ParseException
     * @brief Base class for exceptions related to argument parsing errors.
     */
    class ParseException : public Exception {
        using Exception::Exception;
    };

    /**
     * @class ArgumentNotFoundException
     * @brief Exception thrown when a requested argument is not found.
     */
    class UnknownArgumentException : public ParseException {
        using ParseException::ParseException;
    };

    /**
     * @class MissingArgumentException
     * @brief Exception thrown when a required argument is missing.
     */
    class MissingArgumentException : public ParseException {
        using ParseException::ParseException;
    };

    /**
     * @class TypeMismatchException
     * @brief Exception thrown when an argument's type does not match the expected type.
     */
    class TypeMismatchException : public ParseException {
        using ParseException::ParseException;
    };

    /**
     * @class UnexpectedPositionalArgumentException
     * @brief Exception thrown when an unexpected positional argument is encountered.
     */
    class UnexpectedPositionalArgumentException : public ParseException {
        using ParseException::ParseException;
    };

    /**
     * @class ValidateException
     * @brief Base class for exceptions related to argument validation errors.
     */
    class ValidateException : public Exception {
        using Exception::Exception;
    };

    /**
     * @class InvalidValueException
     * @brief Exception thrown when an argument value is invalid or cannot be converted.
     */
    class InvalidValueException : public ValidateException {
        using ValidateException::ValidateException;
    };

    // Type aliases for supported vector types
    using Bools = std::vector<bool>;
    using Ints = std::vector<int>;
    using Floats = std::vector<float>;
    using Strings = std::vector<std::string>;

    /**
     * @class CliParser
     * @brief Command-line argument parser inspired by Python's argparse.
     *
     * This class provides a flexible and type-safe way to define, parse, and validate command-line arguments.
     * It supports positional and optional arguments, type validation, default values, required arguments,
     * list arguments, shorthand options, and automatic help message generation.
     */
    class CliParser {
    public:
        /**
         * @brief Constructs a CliParser and sets the default help handler.
         * @param argc Argument count from main().
         * @param argv Argument vector from main().
         * @param useColors Whether to use ANSI color codes in help output (default: true).
         *
         * The default help handler prints help and exits. You can override it with setHelpHandler().
         */
        CliParser(int argc, char* argv[], bool useColors = true)
            : m_argc(argc), m_argv(argv), m_useColors(useColors) {
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
        CliParser& add(const std::string& name, const std::string& help = "", std::optional<T> defaultValue = std::nullopt) {
            std::string cleanName = name;
            bool isPositional = true;
            std::string shortName, longName;

            // Enforce naming conventions BEFORE stripping dashes
            if (startsWith(cleanName, "--")) {
                if (cleanName.size() <= 2)
                    throw InvalidArgumentException("longName must not be empty after --");
            }
            else if (startsWith(cleanName, "-")) {
                if (cleanName.size() <= 1)
                    throw InvalidArgumentException("shortName must not be empty after -");
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
                if(defaultValue.has_value()) {
                    throw InvalidArgumentException("Positional arguments cannot have default values");
                }
            }

            // Prevent overriding help flags
            if (longName == "help" || shortName == "h") {
                throw ReservedArgumentException("Cannot redefine built-in --help/-h argument");
            }

            // Check for duplicates
            for (const auto& [k, v] : m_arguments) {
                if (!longName.empty() && v.longName == longName)
                    throw DuplicateArgumentException("Duplicate longName: " + longName);
                if (!shortName.empty() && v.shortName == shortName)
                    throw DuplicateArgumentException("Duplicate shortName: " + shortName);
            }

            ArgType type = deduceArgType<T>();
            Value val = defaultValue ? Value(*defaultValue) : Value{};
            bool isRequired = !defaultValue.has_value();
            // Boolean flags should never be required
            if constexpr (std::is_same_v<T, bool>) {
                isRequired = false;
            }

            Arg arg{shortName, longName, help, isRequired, type, val, Value{}, isPositional};
            std::string key = isPositional ? longName : (longName.empty() ? shortName : longName);
            m_arguments[key] = arg;
            // Register all forms in lookup map
            if (!longName.empty()) {
                m_nameLookup[normalizeName(longName)] = key;
                m_nameLookup[longName] = key;
                if (!longName.empty()) {
                    m_nameLookup["--" + longName] = key;
                }
            }
            if (!shortName.empty()) {
                m_nameLookup[normalizeName(shortName)] = key;
                m_nameLookup[shortName] = key;
                if (!shortName.empty()) {
                    m_nameLookup["-" + shortName] = key;
                }
            }
            if (isPositional) {
                m_nameLookup[normalizeName(longName)] = key;
                m_nameLookup[longName] = key;
                m_positionalOrder.push_back(key);
            }
            return *this;
        }

        CliParser& add(const std::string& name, const std::string& help, const std::string& defaultValue) {
            return add<std::string>(std::string(name), std::string(help), std::optional<std::string>(std::string(defaultValue)));
        }

        CliParser& add(const char* name, const char* help, const char* defaultValue) {  
            add<std::string>(std::string(name), std::string(help), std::optional<std::string>(std::string(defaultValue)));
        }

        /**
         * @brief Add an argument with both short and long names.
         * @tparam T Argument type.
         * @param shortName Short name (e.g., "-c").
         * @param longName Long name (e.g., "--count").
         * @param help Help text.
         * @param defaultValue Optional default value.
         * @return Reference to this CliParser for chaining.
         */
        template<typename T>
        std::enable_if_t<!std::is_same_v<T, std::string>, CliParser&>
        add(const char* shortName,const char* longName, const std::string& help, std::optional<T> defaultValue = std::nullopt) {
            return add<T>(ArgName{std::string(shortName), std::string(longName)}, help, defaultValue);
        }

        template<typename T>
        std::enable_if_t<std::is_same_v<T, std::string>, CliParser&>
        add(const char* shortName, const char* longName, const std::string& help, const char* defaultValue) {
            return add<std::string>(ArgName{shortName, longName}, help, std::string(defaultValue));
        }

        // Shorter method names for adding arguments
        /**
         * @name Convenience methods for adding arguments of specific types
         * @{
         */
        CliParser& addString(const std::string& name, const std::string& help, std::optional<std::string> defaultValue = std::nullopt) { 
            return add<std::string>(name, help, defaultValue); 
        }
        CliParser& addString(const char* name,  const char* help, std::optional<std::string> defaultValue = std::nullopt) {
            return addString(std::string(name), help, defaultValue);
        }
        CliParser& addString(const char* name, const char* help, const char* defaultValue) {
            return addString(std::string(name), std::string(help), std::string(defaultValue));
        }
        CliParser& addInt(const char* name, const char* help, std::optional<int> defaultValue = std::nullopt) { 
            return add<int>(name, help, defaultValue);
        }
        CliParser& addFloat(const char* name, const char* help, std::optional<float> defaultValue = std::nullopt) { 
            return add<float>(name, help, defaultValue);
        }
        CliParser& addBool(const std::string& name, const std::string& help, std::optional<bool> defaultValue = false) { 
            return add<bool>(name, help, defaultValue); 
        }
        CliParser& addStrings(const char* name, const char* help, std::optional<std::vector<std::string>> defaultValue = std::nullopt) {
            return add<std::vector<std::string>>(name, help, defaultValue);
        }
        CliParser& addInts(const char* name, const char* help, std::optional<std::vector<int>> defaultValue = std::nullopt) { 
            return add<std::vector<int>>(name, help, defaultValue); 
        }
        CliParser& addFloats(const char* name, const char* help, std::optional<std::vector<float>> defaultValue = std::nullopt) { 
            return add<std::vector<float>>(name, help, defaultValue); 
        }
        CliParser& addBools(const char* name, const char* help, std::optional<std::vector<bool>> defaultValue = std::nullopt) { 
            return add<std::vector<bool>>(name, help, defaultValue); 
        }
 
        // --- Explicit overloads for named arguments, help, and default value ---
        CliParser& addInt(const char* shortName, const char* longName, const std::string& help, std::optional<int> defaultValue = std::nullopt) {
            return add<int>(ArgName{shortName, longName}, help, defaultValue);
        }
        CliParser& addBool(const char* shortName, const char* longName,  const std::string& help, std::optional<bool> defaultValue = false) {
            return add<bool>(ArgName{shortName, longName}, help, defaultValue);
        }
        CliParser& addString(const char* shortName, const char* longName, const std::string& help, std::optional<std::string> defaultValue = std::nullopt) {
            return add<std::string>(ArgName{shortName, longName}, help, defaultValue);
        }
        CliParser& addFloat(const char* shortName, const char* longName, const std::string& help, std::optional<float> defaultValue = std::nullopt) {
            return add<float>(ArgName{shortName, longName}, help, defaultValue);
        }
        CliParser& addInts(const char* shortName, const char* longName, const std::string& help, std::optional<std::vector<int>> defaultValue = std::nullopt) {
            return add<std::vector<int>>(ArgName{shortName, longName}, help, defaultValue);
        }
        CliParser& addBools(const char* shortName, const char* longName, const std::string& help, std::optional<std::vector<bool>> defaultValue = std::nullopt) {
            return add<std::vector<bool>>(ArgName{shortName, longName}, help, defaultValue);
        }
        CliParser& addStrings(const char* shortName, const char* longName, const std::string& help, std::optional<std::vector<std::string>> defaultValue = std::nullopt) {
            return add<std::vector<std::string>>(ArgName{shortName, longName}, help, defaultValue);
        }
        CliParser& addFloats(const char* shortName, const char* longName, const std::string& help, std::optional<std::vector<float>> defaultValue = std::nullopt) {
            return add<std::vector<float>>(ArgName{shortName, longName}, help, defaultValue);
        }
        /** @} */

        /**
         * @brief Parse command-line arguments using stored argc/argv.
         *
         * This method processes the command-line arguments, validates types, checks for required arguments,
         * and sets default values where appropriate. Throws on unknown or missing required arguments.
         *
         * @throws UnknownArgumentException if an unknown argument is encountered.
         * @throws MissingArgumentException if a required argument is missing.
         * @throws TypeMismatchException if an argument's type does not match the expected type.
         * @throws UnexpectedPositionalArgumentException if a positional argument is encountered out of order.
         * @throws InvalidValueException if a value cannot be converted to the expected type.
         * @note This method automatically handles the --help and -h flags by invoking the help handler.
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
                    if (it == m_arguments.end()) throw UnknownArgumentException("Unknown argument: --" + normKey);
                    currentKey = it->first;
                    Arg& arg = it->second;
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
                    if (it == m_arguments.end()) throw UnknownArgumentException("Unknown short argument: -" + normKey);
                    currentKey = it->first;
                    Arg& arg = it->second;
                    if (isListType(arg.type)) {
                        arg.parsedValue = std::vector<std::string>{};
                        continue;
                    }
                    if (arg.type == ArgType::Bool) {
                        arg.parsedValue = true;
                        currentKey.clear();
                    }
                } else if (!currentKey.empty()) {
                    Arg& arg = m_arguments.at(currentKey);
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
                        throw UnexpectedPositionalArgumentException("Unexpected positional argument: " + token);
                    std::string name = m_positionalOrder[positionalIndex++];
                    Arg& arg = m_arguments.at(name);
                    arg.parsedValue = token;
                }
            }

            // Validate required, set defaults, and convert types
            for (auto& [key, argument] : m_arguments) {
                if (!isListType(argument.type) && std::holds_alternative<std::monostate>(argument.parsedValue)) {
                    if (argument.required)
                        throw MissingArgumentException("Missing required argument: " + argument.longName);
                    argument.parsedValue = argument.defaultValue;
                } else if (isListType(argument.type) && std::holds_alternative<std::monostate>(argument.parsedValue)) {
                    if (argument.required)
                        throw MissingArgumentException("Missing required list argument: " + argument.longName);
                    argument.parsedValue = argument.defaultValue;
                } else {
                    // Convert single value types
                    if (!isListType(argument.type) && std::holds_alternative<std::string>(argument.parsedValue)) {
                        const std::string& val = std::get<std::string>(argument.parsedValue);
                        try {
                            switch (argument.type) {
                            case ArgType::Int:
                                argument.parsedValue = std::stoi(val);
                                break;
                            case ArgType::Float:
                                argument.parsedValue = std::stof(val);
                                break;
                            case ArgType::Bool:
                                argument.parsedValue = (val == "true" || val == "1");
                                break;
                            case ArgType::String:
                                // already string
                                break;
                            default:
                                break;
                            }
                        } catch (const std::invalid_argument& e) {
                            throw InvalidValueException(std::string("Invalid value for argument '") + argument.longName + "': " + val + " (" + e.what() + ")");
                        } catch (const std::out_of_range& e) {
                            throw InvalidValueException(std::string("Value out of range for argument '") + argument.longName + "': " + val + " (" + e.what() + ")");
                        }
                    }
                    // Convert list types
                    else if (isListType(argument.type) && std::holds_alternative<std::vector<std::string>>(argument.parsedValue)) {
                        const auto& vec = std::get<std::vector<std::string>>(argument.parsedValue);
                        try {
                            switch (argument.type) {
                            case ArgType::IntList: {
                                std::vector<int> out;
                                for (const auto& v : vec) out.push_back(std::stoi(v));
                                argument.parsedValue = out;
                                break;
                            }
                            case ArgType::FloatList: {
                                std::vector<float> out;
                                for (const auto& v : vec) out.push_back(std::stof(v));
                                argument.parsedValue = out;
                                break;
                            }
                            case ArgType::BoolList: {
                                std::vector<bool> out;
                                for (const auto& v : vec) out.push_back(v == "true" || v == "1");
                                argument.parsedValue = out;
                                break;
                            }
                            case ArgType::StringList:
                                // already string vector
                                break;
                            default:
                                break;
                            }
                        } catch (const std::invalid_argument& e) {
                            throw InvalidValueException("Invalid value in list for argument '" + argument.longName + "'" + std::string(" (") + e.what() + ")");
                        } catch (const std::out_of_range& e) {
                            throw InvalidValueException("Value out of range in list for argument '" + argument.longName + "'" + std::string(" (") + e.what() + ")");
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
         * @throws UnknownArgumentException if the argument is not found.
         * @throws TypeMismatchException if the argument type does not match T.
         */
        template<typename T>
        T get(const std::string& name) const {
            std::string normName = normalizeName(name);
            auto lookupIt = m_nameLookup.find(normName);
            if (lookupIt == m_nameLookup.end()) throw UnknownArgumentException("Argument not found: " + name);
            const Arg& arg = m_arguments.at(lookupIt->second);
            // Handle vector types
            if constexpr (is_vector<T>::value) {
                if (std::holds_alternative<T>(arg.parsedValue)) {
                    return std::get<T>(arg.parsedValue);
                } else if (!std::holds_alternative<std::monostate>(arg.defaultValue) && std::holds_alternative<T>(arg.defaultValue)) {
                    return std::get<T>(arg.defaultValue);
                }
                throw TypeMismatchException("Type mismatch: argument '" + name + "' is not of type " + typeid(T).name() + ".");
            } else {
                if (std::holds_alternative<std::monostate>(arg.parsedValue)) {
                    if (!std::holds_alternative<std::monostate>(arg.defaultValue) && std::holds_alternative<T>(arg.defaultValue)) {
                        return std::get<T>(arg.defaultValue);
                    }
                    throw UnknownArgumentException("Argument not found: " + name);
                }
                if (std::holds_alternative<T>(arg.parsedValue)) {
                    return std::get<T>(arg.parsedValue);
                }
                throw TypeMismatchException("Type mismatch: argument '" + name + "' is not of type " + typeid(T).name() + ".");
            }
        }

        /**
         * @brief Check if an argument was provided on the command line.
         * @param name Argument name.
         * @return True if the argument is present, false otherwise.
         */
        bool has(const std::string& name) const {
            std::string normName = normalizeName(name);
            auto lookupIt = m_nameLookup.find(normName);
            if (lookupIt == m_nameLookup.end()) return false;
            const Arg& arg = m_arguments.at(lookupIt->second);
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
            const char* bold = m_useColors ? "\033[1m" : "";
            const char* cyan = m_useColors ? "\033[36m" : "";
            const char* yellow = m_useColors ? "\033[33m" : "";
            const char* reset = m_useColors ? "\033[0m" : "";
            const char* gray = m_useColors ? "\033[90m" : "";
            const char* green = m_useColors ? "\033[32m" : "";

            // Usage line
            std::cout << bold << "Usage: " << reset << programName;
            for (const auto& positional : m_positionalOrder)
                std::cout << " " << cyan << "<" << positional << ">" << reset;
            std::cout << " " << green << "[options]" << reset << "\n\n";

            // Section: Positional arguments
            if (!m_positionalOrder.empty()) {
                std::cout << bold << "Positional:" << reset << "\n";
                // Find max width for alignment (name only, no type)
                size_t maxPosLen = 0;
                std::vector<std::string> posNames;
                for (const auto& key : m_positionalOrder) {
                    const auto& argument = m_arguments.at(key);
                    std::string pos = argument.longName;
                    if (pos.size() > maxPosLen) maxPosLen = pos.size();
                    posNames.push_back(pos);
                }
                size_t posIdx = 0;
                for (const auto& key : m_positionalOrder) {
                    const auto& argument = m_arguments.at(key);
                    std::string pos = posNames[posIdx++];
                    std::cout << "  " << cyan << pos << reset;
                    size_t pad = maxPosLen > pos.size() ? maxPosLen - pos.size() : 0;
                    std::cout << std::string(pad, ' ');
                    // Help message starts here
                    if (!argument.help.empty())
                        std::cout << "  " << argument.help;
                    if (!std::holds_alternative<std::monostate>(argument.defaultValue))
                        std::cout << gray << " (default: " << toString(argument.defaultValue) << ")" << reset;
                    std::cout << "\n";
                }
                std::cout << "\n";
            }

            // Section: Options
            std::cout << bold << "Options:" << reset << "\n";
            // Find max width for alignment (name only, no type)
            size_t maxOptNameLen = 0;
            std::vector<std::string> optNames;
            std::vector<std::string> valueTypes;
            std::vector<bool> isBoolFlag;
            for (const auto& [key, argument] : m_arguments) {
                if (!argument.positional) {
                    std::string opt;
                    if (!argument.shortName.empty() && !argument.longName.empty()) {
                        opt = "-" + argument.shortName + ", --" + argument.longName;
                    } else if (!argument.shortName.empty()) {
                        opt = "-" + argument.shortName;
                    } else if (!argument.longName.empty()) {
                        opt = "    --" + argument.longName; // 4 spaces for alignment
                    } else {
                        opt = "";
                    }
                    // Determine value type
                    std::string valueType;
                    bool isBool = false;
                    switch (argument.type) {
                        case ArgType::Int: valueType = "<int>"; break;
                        case ArgType::Float: valueType = "<float>"; break;
                        case ArgType::Bool: valueType = ""; isBool = true; break;
                        case ArgType::String: valueType = "<string>"; break;
                        case ArgType::IntList: valueType = "<int[]>"; break;
                        case ArgType::FloatList: valueType = "<float[]>"; break;
                        case ArgType::BoolList: valueType = "<bool[]>"; break;
                        case ArgType::StringList: valueType = "<string[]>"; break;
                        default: valueType = "<value>"; break;
                    }
                    if (opt.size() > maxOptNameLen) maxOptNameLen = opt.size();
                    optNames.push_back(opt);
                    valueTypes.push_back(valueType);
                    isBoolFlag.push_back(isBool);
                }
            }
            // Also consider help flag in maxOptNameLen
            std::string helpFlag = "-h, --help";
            if (helpFlag.size() > maxOptNameLen) maxOptNameLen = helpFlag.size();

            // Find max width for value type
            size_t maxTypeLen = 0;
            for (const auto& vt : valueTypes) {
                if (vt.size() > maxTypeLen) maxTypeLen = vt.size();
            }

            // Print options with aligned <value> and help text
            size_t optIdx = 0;
            for (const auto& [key, argument] : m_arguments) {
                if (!argument.positional) {
                    std::string opt = optNames[optIdx];
                    std::string valueType = valueTypes[optIdx];
                    bool isBool = isBoolFlag[optIdx++];
                    std::cout << "  " << green << opt << reset;
                    size_t padName = maxOptNameLen > opt.size() ? maxOptNameLen - opt.size() : 0;
                    std::cout << std::string(padName, ' ');
                    if (!isBool && !valueType.empty()) {
                        std::cout << " " << gray << valueType << reset;
                        size_t padType = maxTypeLen > valueType.size() ? maxTypeLen - valueType.size() : 0;
                        std::cout << std::string(padType, ' ');
                    } else {
                        std::cout << std::string(maxTypeLen + 1, ' '); // +1 for space before type
                    }
                    // Help message starts here
                    if (!argument.help.empty())
                        std::cout << "  " << argument.help;
                    if (!std::holds_alternative<std::monostate>(argument.defaultValue))
                        std::cout << gray << " (default: " << toString(argument.defaultValue) << ")" << reset;
                    if (argument.required) {
                        std::cout << " " << yellow << "(required)" << reset;
                    }
                    std::cout << "\n";
                }
            }
            // Help flag, aligned
            size_t helpPadName = maxOptNameLen > helpFlag.size() ? maxOptNameLen - helpFlag.size() : 0;
            std::cout << "  " << green << helpFlag << reset << std::string(helpPadName, ' ') << std::string(maxTypeLen + 1, ' ') << "  Show this help message\n";
        }

    private:
        bool m_useColors = true;
        
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
        struct Arg {
            std::string shortName;   
            std::string longName;   
            std::string help;       ///< Help/description string.
            bool required{ true };  ///< True if argument must be provided by the user.
            ArgType type{ ArgType::String }; ///< Argument type.
            Value defaultValue;     ///< Default value if any.
            Value parsedValue;     ///< Parsed value if any.
            bool positional{ false }; ///< True if this is a positional argument.
        };

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

        // Lookup map: maps all forms to canonical key
        std::unordered_map<std::string, std::string> m_nameLookup; ///< Maps argument names to canonical keys.
        std::unordered_map<std::string, Arg> m_arguments; ///< Map of all arguments.
        std::vector<std::string> m_positionalOrder; ///< Order of positional arguments.
        std::function<void(std::string)> m_helpHandler; ///< Function to handle help requests.
        int m_argc; ///< Argument count from main().
        char** m_argv; ///< Argument vector from main().

        /**
         * @brief Add an argument to the parser (positional or optional) using ArgName.
         * @tparam T Argument type (int, float, bool, string, or vector thereof).
         * @param argName ArgName object with short and/or long name.
         * @param help Help text for usage.
         * @param defaultValue Optional default value; if omitted, argument is required.
         * @throws std::runtime_error if attempting to override reserved names --help or -h.
         */
        template<typename T>
        CliParser& add(const ArgName& argName,
            const std::string& help = "",
            std::optional<T> defaultValue = std::nullopt) {
            std::string shortKey = argName.shortName;
            std::string longKey = argName.longName;

            // Enforce naming conventions BEFORE stripping dashes
            if (!shortKey.empty() && !startsWith(shortKey, "-"))
                throw InvalidArgumentException("shortName must start with -");
            if (!longKey.empty() && !startsWith(longKey, "--"))
                throw InvalidArgumentException("longName must start with --");

            // Strip dashes for storage
            if (startsWith(shortKey, "-")) shortKey = shortKey.substr(1);
            if (startsWith(longKey, "--")) longKey = longKey.substr(2);
            else if (startsWith(longKey, "-")) longKey = longKey.substr(1);

            ArgType type = deduceArgType<T>();
            Value val = defaultValue ? Value(*defaultValue) : Value{};
            bool isRequired = !defaultValue.has_value();

            if constexpr (std::is_same_v<T, bool>) {
                isRequired = false;
            }

            bool isPositional = longKey.empty();
            // Check for duplicates
            for (const auto& [k, v] : m_arguments) {
                if (!longKey.empty() && v.longName == longKey)
                    throw DuplicateArgumentException("Duplicate longName: " + longKey);
                if (!shortKey.empty() && v.shortName == shortKey)
                    throw DuplicateArgumentException("Duplicate shortName: " + shortKey);
            }

            Arg arg{shortKey, longKey, help, isRequired, type, val, Value{}, isPositional};
            std::string key = isPositional ? shortKey : longKey;
            m_arguments[key] = arg;
            // Register all forms in lookup map
            if (!longKey.empty()) {
                m_nameLookup[normalizeName(longKey)] = key;
                m_nameLookup[longKey] = key;
                m_nameLookup["--" + longKey] = key;
            }
            if (!shortKey.empty()) {
                m_nameLookup[normalizeName(shortKey)] = key;
                m_nameLookup[shortKey] = key;
                m_nameLookup["-" + shortKey] = key;
            }
            if (isPositional) {
                m_nameLookup[normalizeName(shortKey)] = key;
                m_nameLookup[shortKey] = key;
                m_positionalOrder.push_back(key);
            }
            return *this;
        }

        // checks if a string starts with a given prefix
        static bool startsWith(const std::string& str, const std::string& prefix) {
            return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
        }
        // normalize argument name (strip leading dashes)
        static std::string normalizeName(const std::string& name) {
            if (startsWith(name, "--")) return name.substr(2);
            if (startsWith(name, "-")) return name.substr(1);
            return name;
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
}