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
#include <filesystem>
#include <regex>

/// @brief Namespace for the Argy command-line argument parser library.
namespace Argy {
    /// @brief Base class for all exceptions in the Argy library.
    class Exception : public std::exception {
    public:
        explicit Exception(const std::string& message) : m_message(message) {}
        const char* what() const noexcept override { return m_message.c_str(); }
    private:
        std::string m_message;
    };

    /// @brief Base class for exceptions related to argument definition errors.
    class DefineException : public Exception {
        using Exception::Exception;
    };

    /// @brief Base class for exceptions related to argument parsing errors.
    class ReservedArgumentException : public DefineException {
        using DefineException::DefineException;
    };

    /// @brief Exception thrown when an argument is defined multiple times.
    class DuplicateArgumentException : public DefineException {
        using DefineException::DefineException;
    };

    /// @brief Exception thrown when an argument definition is invalid.
    class InvalidArgumentException : public DefineException {
        using DefineException::DefineException;
    };

    /// @brief Base class for exceptions related to argument parsing errors.
    class ParseException : public Exception {
        using Exception::Exception;
    };

    /// @brief Exception thrown when a requested argument is not found.
    class UnknownArgumentException : public ParseException {
        using ParseException::ParseException;
    };

    /// @brief Exception thrown when a required argument is missing.
    class MissingArgumentException : public ParseException {
        using ParseException::ParseException;
    };

    /// @brief Exception thrown when a type mismatch occurs during parsing.
    class TypeMismatchException : public ParseException {
        using ParseException::ParseException;
    };

    /// @brief Exception thrown when an unexpected positional argument is encountered.
    class UnexpectedPositionalArgumentException : public ParseException {
        using ParseException::ParseException;
    };

    /// @brief Base class for exceptions related to argument validation errors.
    class ValidateException : public Exception {
        using Exception::Exception;
    };

    /// @brief Exception thrown when an argument value is invalid or cannot be converted.
    class InvalidValueException : public ValidateException {
        using ValidateException::ValidateException;
    };

    /// @brief Exception thrown when a value is out of the expected range.
    class OutOfRangeException : public ValidateException {
        using ValidateException::ValidateException;
    };

    // Type aliases for supported vector types
    using Bools = std::vector<bool>;
    using Ints = std::vector<int>;
    using Floats = std::vector<float>;
    using Strings = std::vector<std::string>;

    /// @brief Returns a validator lambda that checks if a value is within a specified range.
    /// @param min Minimum allowed value (inclusive).
    /// @param max Maximum allowed value (inclusive).
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be within a specific range.
    template<typename T>
    auto IsValueInRange(T min, T max) {
        return [min, max](const std::string& name, const T& value) {
            if (value < min || value > max)
                throw Argy::OutOfRangeException("Argument '" + name + "' value " + std::to_string(value) +
                                                " is out of range [" + std::to_string(min) + ", " + std::to_string(max) + "]");
        };
    }

    /// @brief Returns a validator lambda that checks if all values in a vector are within a specified range.
    /// @param min Minimum allowed value (inclusive).
    /// @param max Maximum allowed value (inclusive).
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that all values in a vector argument must be within a specific range.
    template<typename T>
    auto IsVectorInRange(T min, T max) {
        return [min, max](const std::string& name, const std::vector<T>& values) {
            for (const auto& v : values) {
                IsValueInRange(min, max)(name, v);
            }
        };
    }

    /// @brief Returns a validator lambda that checks if a string value is a valid RGB color code.
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be a valid RGB color code in the format "rgb(r, g, b)".
    inline auto IsAlphaNumeric() {
        return [](const std::string& name, const std::string& value) {
            if (!std::all_of(value.begin(), value.end(), ::isalnum)) {
                throw InvalidValueException("Value '" + value + "' for argument '" + name +
                    "' must contain only alphanumeric characters");
            }
        };
    }

    /// @brief Returns a validator lambda that checks if a string value contains only letters.
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must contain only alphabetic characters.
    inline auto IsAlpha() {
        return [](const std::string& name, const std::string& value) {
            if (!std::all_of(value.begin(), value.end(), ::isalpha)) {
                throw InvalidValueException("Value '" + value + "' for argument '" + name +
                    "' must contain only alphabetic characters");
            }
        };
    }

    /// @brief Returns a validator lambda that checks if a string value contains only digits.
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must contain only numeric characters.
    inline auto IsNumeric() {
        return [](const std::string& name, const std::string& value) {
            if (!std::all_of(value.begin(), value.end(), ::isdigit)) {
                throw InvalidValueException("Value '" + value + "' for argument '" + name +
                    "' must contain only digits");
            }
        };
    }

    /// @brief Returns a validator lambda that checks if a string value is a valid path (file or directory).
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be a valid file or directory path.
    inline auto IsPath() {
        return [](const std::string& name, const std::string& value) {
            std::filesystem::path p(value);
            // If it's a symlink, resolve to target
            if (std::filesystem::is_symlink(p)) {
                p = std::filesystem::read_symlink(p);
                // If relative, resolve against parent
                if (p.is_relative()) {
                    p = std::filesystem::absolute(std::filesystem::path(value).parent_path() / p);
                }
            }
            if (!std::filesystem::exists(p)) {
                throw InvalidValueException("Value '" + value + "' for argument '" + name + "' does not exist");
            }
        };
    }

    /// @brief Returns a validator lambda that checks if a string value is a file and if it exists.
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    inline auto IsFile() {
        return [](const std::string& name, const std::string& value) {
            std::filesystem::path p(value);
            // If it's a symlink, resolve to target
            if (std::filesystem::is_symlink(p)) {
                p = std::filesystem::read_symlink(p);
                // If relative, resolve against parent
                if (p.is_relative()) {
                    p = std::filesystem::absolute(std::filesystem::path(value).parent_path() / p);
                }
            }
            if (!std::filesystem::exists(p) || !std::filesystem::is_regular_file(p)) {
                throw InvalidValueException("Value '" + value + "' for argument '" + name + "' is not a valid file path");
            }
        };
    }

    /// @brief Returns a validator lambda that checks if a string value is a directory and if it exists.
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    inline auto IsDirectory() {
        return [](const std::string& name, const std::string& value) {
            std::filesystem::path p(value);
            // If it's a symlink, resolve to target
            if (std::filesystem::is_symlink(p)) {
                p = std::filesystem::read_symlink(p);
                // If relative, resolve against parent
                if (p.is_relative()) {
                    p = std::filesystem::absolute(std::filesystem::path(value).parent_path() / p);
                }
            }
            if (!std::filesystem::exists(p) || !std::filesystem::is_directory(p)) {
                throw InvalidValueException("Value '" + value + "' for argument '" + name + "' is not a valid directory path");
            }
        };
    }

    /// @brief Returns a validator lambda that checks if a string value is one of the specified valid values.
    /// @param validValues Vector of valid string values
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be one of a predefined set of strings.
    inline auto IsOneOf(const std::vector<std::string>& validValues) {
        // Helper function to join vector<string> with a separator
        auto join = [](const std::vector<std::string>& vec, const std::string& sep) {
            std::string result;
            for (size_t i = 0; i < vec.size(); ++i) {
                if (i > 0) result += sep;
                result += vec[i];
            }
            return result;
        };
        return [validValues, join](const std::string& name, const std::string& value) {
            if (std::find(validValues.begin(), validValues.end(), value) == validValues.end()) {
                throw InvalidValueException("Value '" + value + "' for argument '" + name +
                    "' must be one of: " + join(validValues, ", "));
            }
        };
    }

    /// @brief Returns a validator lambda that checks if a string value matches a regex pattern.
    /// @param pattern Regex pattern to match against
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must match a specific regex pattern.
    inline auto IsMatch(const std::string& regexPattern) {
        return [regexPattern](const std::string& name, const std::string& value) {
            std::regex re(regexPattern);
            if (!std::regex_match(value, re)) {
                throw InvalidValueException("Value '" + value + "' for argument '" + name +
                    "' does not match pattern: " + regexPattern);
            }
        };
    }

    /// @brief Returns a validator lambda that checks if a string value is a valid IP address (IPv4 or IPv6).
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be a valid IP address format.
    inline auto IsIPv4() {
        return IsMatch(R"(\b((25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\.){3}(25[0-5]|2[0-4][0-9]|1[0-9]{2}|[1-9]?[0-9])\b)");
    }

    /// @brief Returns a validator lambda that checks if a string value is a valid IPv6 address.
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be a valid IPv6 address format.
    inline auto IsIPv6() {
        return IsMatch(R"(\b([0-9a-fA-F]{1,4}:){7}[0-9a-fA-F]{1,4}\b)");
    }

    /// @brief Returns a validator lambda that checks if a string value is a valid MAC address.
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be a valid MAC address format.
    inline auto IsMACAddress() {
        return IsMatch(R"(\b([0-9A-Fa-f]{2}[:-]){5}([0-9A-Fa-f]{2})\b)");
    }

    /// @brief Returns a validator lambda that checks if a string value is a valid IP address (IPv4 or IPv6).
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be a valid IP address format.
    inline auto IsIPAddress() {
        return [](const std::string& name, const std::string& value) {
            try {
                IsIPv4()(name, value);
            } catch (const InvalidValueException&) {
                try {
                    IsIPv6()(name, value);
                } catch (const InvalidValueException&) {
                    throw InvalidValueException("Value '" + value + "' for argument '" + name +
                        "' is not a valid IP address (IPv4 or IPv6)");
                }
            }
        };
    }

    /// @brief Returns a validator lambda that checks if a string value is a valid email address.
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be a valid email address format.
    inline auto IsEmail() {
        return IsMatch(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    }
     
    /// @brief Returns a validator lambda that checks if a string value is a valid URL.
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be a valid URL format.
    inline auto IsUrl() {
        return IsMatch(R"(^https?://[a-zA-Z0-9.-]+(?:\.[a-zA-Z]{2,})+.*$)");
    }

    /// @brief Returns a validator lambda that checks if a string value is a valid UUID (version 4).
    /// @return Lambda suitable for CliParser::setValidator() and CliParser::ArgBuilder::validate()
    /// This allows you to enforce that an argument's value must be a valid UUID format.
    inline auto IsUUID() {
        return IsMatch(R"(^[0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}$)");
    }

    /// @class CliParser
    /// @brief Command-line argument parser inspired by Python's argparse.
    /// This class provides a flexible and type-safe way to define, parse, and validate command-line arguments.
    /// It supports positional and optional arguments, type validation, default values, required arguments,
    /// list arguments, shorthand options, and automatic help message generation.
    class CliParser {
    public:
        /// Helper to deduce lambda argument types
        template<typename T>
        struct lambda_arg_type;

        /// Single-argument lambda
        template<typename R, typename C, typename arg>
        struct lambda_arg_type<R(C::*)(arg) const> { using type = arg; };

        /// Two-argument lambda (by value)
        template<typename R, typename C, typename arg1, typename arg2>
        struct lambda_arg_type<R(C::*)(arg1, arg2) const> { using type = arg2; };

        /// Two-argument lambda (by const reference)
        template<typename R, typename C, typename arg1, typename arg2>
        struct lambda_arg_type<R(C::*)(arg1, const arg2&) const> { using type = arg2; };

        /// Alias for deducing lambda argument types
        template<typename F>
        using lambda_arg_t = typename lambda_arg_type<decltype(&F::operator())>::type;

        /// @brief ArgBuilder class for adding additional functionality to argument definitions.
        /// This class allows you to chain validation functions to an argument after it has been defined.
        class ArgBuilder {
        public:
            /// @brief Constructs an ArgBuilder for a specific argument key.
            /// @param parser Reference to the CliParser instance.
            /// @param key The argument key (name) to build upon.
            ArgBuilder(CliParser& parser, const std::string& key)
                : m_parser(parser), m_key(key) {}

            /// @brief Adds a validation function to the argument.
            template<typename F>
            ArgBuilder& validate(F&& fn) {
                using T = lambda_arg_t<F>;
                m_parser.setValidator(m_key, std::forward<F>(fn));
                return *this;
            }

            /// @brief Sets a default value for the argument.
            /// @returns a reference to the CliParser for further chaining.
            CliParser& done() { return m_parser; }

        private:
            CliParser& m_parser;
            std::string m_key;
        };

    public:
        /// @brief Constructs a CliParser and sets the default help handler.
        /// @param argc Argument count from main().
        /// @param argv Argument vector from main().
        /// @param useColors Whether to use ANSI color codes in help output (default: true).
        /// The default help handler prints help and exits. You can override it with setHelpHandler().
        CliParser(int argc, char* argv[], bool useColors = true)
            : m_argc(argc), m_argv(argv), m_useColors(useColors) {
            m_helpHandler = [this](std::string name) {
                printHelp(name);
                std::exit(0);
            };
        }

        /// @brief Set a custom help handler invoked on --help or -h.
        /// @param handler Function to call when help is requested. Receives the program name.
        /// The default handler prints help and exits. Override this if you want to return or throw instead.
        void setHelpHandler(std::function<void(std::string)> handler) {
            m_helpHandler = std::move(handler);
        }

        /// @brief set validator for an argument
        /// @param name Argument name to set the validator for.
        /// @param fn Validation function that takes the argument value and throws TypeMismatchException on failure.
        /// This allows you to enforce custom validation rules for argument values.
        template<typename F>
        void setValidator(const std::string& name, F&& fn) {
            auto lookupIt = m_nameLookup.find(normalizeName(name));
            if (lookupIt == m_nameLookup.end())
                throw UnknownArgumentException("Argument not found for validator: " + name);
            auto& arg = m_arguments.at(lookupIt->second);
            using T = lambda_arg_t<F>;
            arg.validator = [fn = std::forward<F>(fn), name](const Value& v) {
                if constexpr (std::is_invocable_v<F, T>) {
                    if (!std::holds_alternative<T>(v))
                        throw TypeMismatchException("Validator type mismatch for argument '" + name + "'");
                    fn(std::get<T>(v));
                } else if constexpr (std::is_invocable_v<F, std::string, T>) {
                    if (!std::holds_alternative<T>(v))
                        throw TypeMismatchException("Validator type mismatch for argument '" + name + "'");
                    fn(name, std::get<T>(v));
                } else {
                    static_assert(std::is_invocable_v<F, T> || std::is_invocable_v<F, std::string, T>,
                        "Validator must be invocable with (value) or (name, value)");
                }
            };
        }

        /// @brief Add an argument to the parser with a single name.
        /// @tparam T Argument type (int, float, bool, string, or vector thereof).
        /// @param name Argument name (e.g. "filename", "-c", "--count").
        /// @param help Help text for usage.
        /// @param defaultValue Optional default value; if omitted, argument is required.
        template<typename T>
        ArgBuilder add(const char* name, const char* help, std::optional<T> defaultValue = std::nullopt) {
            return add<T>(std::vector<std::string>{std::string(name)}, help, defaultValue);
        }

        /// @brief Add an argument to the parser with multiple names (aliases).
        /// @tparam T Argument type (int, float, bool, string, or vector thereof).
        /// @param names Vector of argument names (e.g. {"-c", "--count", "--cnt"}).
        /// @param help Help text for usage.
        /// @param defaultValue Optional default value; if omitted, argument is required.
        template<typename T>
        ArgBuilder add(const std::vector<std::string>& names, const std::string& help, std::optional<T> defaultValue = std::nullopt) {
            std::vector<std::string> cleanNames;
            bool isPositional = true;
            std::vector<std::string> shortNames, longNames;
            for (const auto& n : names) {
                if (startsWith(n, "--")) {
                    if (n.size() <= 2) throw InvalidArgumentException("longName must not be empty after --");
                    longNames.push_back(n.substr(2));
                    isPositional = false;
                }
                else if (startsWith(n, "-")) {
                    if (n.size() <= 1) throw InvalidArgumentException("shortName must not be empty after -");
                    shortNames.push_back(n.substr(1));
                    isPositional = false;
                }
                else {
                    longNames.push_back(n);
                }
                cleanNames.push_back(normalizeName(n));
            }

            // Prevent overriding help flags
            for (const auto& ln : longNames) {
                if (ln == "help") throw ReservedArgumentException("Cannot redefine built-in --help argument");
            }
            for (const auto& sn : shortNames) {
                if (sn == "h") throw ReservedArgumentException("Cannot redefine built-in -h argument");
            }

            // Check for duplicates across all existing aliases
            for (const auto& [k, v] : m_arguments) {
                for (const auto& existing : v.names) {
                    for (const auto& cand : cleanNames) {
                        if (existing == cand) throw DuplicateArgumentException("Duplicate argument name: " + cand);
                    }
                }
            }
            ArgType type = deduceArgType<T>();
            Value val = defaultValue ? Value(*defaultValue) : Value{};
            bool isRequired = !defaultValue.has_value();
            if constexpr (std::is_same_v<T, bool>) { isRequired = false; }
            // Use first provided normalized name as canonical key
            std::string key = cleanNames.empty() ? std::string() : cleanNames[0];
            // Store aliases (normalized names) and original classification
            Arg arg{ cleanNames, shortNames, longNames, help, isRequired, type, val, Value{}, isPositional };
            m_arguments[key] = arg;
            // Register all forms in lookup map
            for (const auto& cn : cleanNames) {
                m_nameLookup[cn] = key;
            }
            // Also register dashed forms for display/lookup convenience
            for (const auto& ln : longNames) {
                m_nameLookup[ln] = key;
                m_nameLookup["--" + ln] = key;
            }
            for (const auto& sn : shortNames) {
                m_nameLookup[sn] = key;
                m_nameLookup["-" + sn] = key;
            }
            if (isPositional) {
                m_positionalOrder.push_back(key);
            }
            return ArgBuilder(*this, key);
        }

        // Convenience overloads for single name
        ArgBuilder addString(const char* name, const std::string& help, std::optional<std::string> defaultValue = std::nullopt) {
            return add<std::string>(std::vector<std::string>{std::string(name)}, help, defaultValue);
        }
        ArgBuilder addInt(const char* name, const std::string& help, std::optional<int> defaultValue = std::nullopt) {
            return add<int>(std::vector<std::string>{std::string(name)}, help, defaultValue);
        }
        ArgBuilder addFloat(const char* name, const std::string& help, std::optional<float> defaultValue = std::nullopt) {
            return add<float>(std::vector<std::string>{std::string(name)}, help, defaultValue);
        }
        ArgBuilder addBool(const char* name, const std::string& help, std::optional<bool> defaultValue = false) {
            return add<bool>(std::vector<std::string>{std::string(name)}, help, defaultValue);
        }
        ArgBuilder addStrings(const char* name, const std::string& help, std::optional<std::vector<std::string>> defaultValue = std::nullopt) {
            return add<std::vector<std::string>>(std::vector<std::string>{std::string(name)}, help, defaultValue);
        }
        ArgBuilder addInts(const char* name, const std::string& help, std::optional<std::vector<int>> defaultValue = std::nullopt) {
            return add<std::vector<int>>(std::vector<std::string>{std::string(name)}, help, defaultValue);
        }
        ArgBuilder addFloats(const char* name, const std::string& help, std::optional<std::vector<float>> defaultValue = std::nullopt) {
            return add<std::vector<float>>(std::vector<std::string>{std::string(name)}, help, defaultValue);
        }
        ArgBuilder addBools(const char* name, const std::string& help, std::optional<std::vector<bool>> defaultValue = std::nullopt) {
            return add<std::vector<bool>>(std::vector<std::string>{std::string(name)}, help, defaultValue);
        }
        // Convenience methods for adding arguments of specific types using vector<string> API
        ArgBuilder addString(const std::vector<std::string>& names, const std::string& help, std::optional<std::string> defaultValue = std::nullopt) {
            return add<std::string>(names, help, defaultValue);
        }
        ArgBuilder addInt(const std::vector<std::string>& names, const std::string& help, std::optional<int> defaultValue = std::nullopt) {
            return add<int>(names, help, defaultValue);
        }
        ArgBuilder addFloat(const std::vector<std::string>& names, const std::string& help, std::optional<float> defaultValue = std::nullopt) {
            return add<float>(names, help, defaultValue);
        }
        ArgBuilder addBool(const std::vector<std::string>& names, const std::string& help, std::optional<bool> defaultValue = false) {
            return add<bool>(names, help, defaultValue);
        }
        ArgBuilder addStrings(const std::vector<std::string>& names, const std::string& help, std::optional<std::vector<std::string>> defaultValue = std::nullopt) {
            return add<std::vector<std::string>>(names, help, defaultValue);
        }
        ArgBuilder addInts(const std::vector<std::string>& names, const std::string& help, std::optional<std::vector<int>> defaultValue = std::nullopt) {
            return add<std::vector<int>>(names, help, defaultValue);
        }
        ArgBuilder addFloats(const std::vector<std::string>& names, const std::string& help, std::optional<std::vector<float>> defaultValue = std::nullopt) {
            return add<std::vector<float>>(names, help, defaultValue);
        }
        ArgBuilder addBools(const std::vector<std::string>& names, const std::string& help, std::optional<std::vector<bool>> defaultValue = std::nullopt) {
            return add<std::vector<bool>>(names, help, defaultValue);
        }

        /// @brief Parse command-line arguments using stored argc/argv.
        /// This method processes the command-line arguments, validates types, checks for required arguments,
        /// and sets default values where appropriate. Throws on unknown or missing required arguments.
        /// @throws UnknownArgumentException if an unknown argument is encountered.
        /// @throws MissingArgumentException if a required argument is missing.
        /// @throws TypeMismatchException if an argument's type does not match the expected type.
        /// @throws UnexpectedPositionalArgumentException if a positional argument is encountered out of order.
        /// @throws InvalidValueException if a value cannot be converted to the expected type.
        /// @throws OutOfRangeException if a value is outside the expected range.
        /// @note This method automatically handles the --help and -h flags by invoking the help handler.
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
                    // Find by any registered name (long forms are registered with and without dashes)
                    auto it = std::find_if(m_arguments.begin(), m_arguments.end(), [&](const auto& pair) {
                        const Arg& a = pair.second;
                        return std::find(a.names.begin(), a.names.end(), normKey) != a.names.end() ||
                            std::find(a.names.begin(), a.names.end(), "--" + normKey) != a.names.end();
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
                }
                else if (startsWith(token, "-") && token.size() > 1 && !isNegativeNumber(token)) {
                    std::string normKey = token.substr(1);
                    // Find by shortName (with or without dash)
                    auto it = std::find_if(m_arguments.begin(), m_arguments.end(), [&](const auto& pair) {
                        const Arg& a = pair.second;
                        return std::find(a.names.begin(), a.names.end(), normKey) != a.names.end() ||
                            std::find(a.names.begin(), a.names.end(), "-" + normKey) != a.names.end();
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
                }
                else if (!currentKey.empty()) {
                    Arg& arg = m_arguments.at(currentKey);
                    if (isListType(arg.type)) {
                        if (std::holds_alternative<std::vector<std::string>>(arg.parsedValue)) {
                            std::get<std::vector<std::string>>(arg.parsedValue).push_back(token);
                        }
                    }
                    else {
                        arg.parsedValue = token;
                        currentKey.clear();
                    }
                }
                else {
                    // Positional argument
                    if (positionalIndex >= m_positionalOrder.size())
                        throw UnexpectedPositionalArgumentException("Unexpected positional argument: " + token);
                    std::string name = m_positionalOrder[positionalIndex++];
                    Arg& arg = m_arguments.at(name);
                    arg.parsedValue = token;
                }
            }

            // Validate required, set defaults, convert types, and run validator
            for (auto& [key, argument] : m_arguments) {
                if (!isListType(argument.type) && std::holds_alternative<std::monostate>(argument.parsedValue)) {
                    if (argument.required)
                        throw MissingArgumentException("Missing required argument: " + (argument.names.empty() ? key : argument.names[0]));
                    argument.parsedValue = argument.defaultValue;
                }
                else if (isListType(argument.type) && std::holds_alternative<std::monostate>(argument.parsedValue)) {
                    if (argument.required)
                        throw MissingArgumentException("Missing required list argument: " + (argument.names.empty() ? key : argument.names[0]));
                    argument.parsedValue = argument.defaultValue;
                }
                else {
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
                        }
                        catch (const std::invalid_argument& e) {
                            throw InvalidValueException(std::string("Invalid value for argument '") + (argument.names.empty() ? key : argument.names[0]) + "': " + val + " (" + e.what() + ")");
                        }
                        catch (const std::out_of_range& e) {
                            throw OutOfRangeException(std::string("Value out of range for argument '") + (argument.names.empty() ? key : argument.names[0]) + "': " + val + " (" + e.what() + ")");
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
                        }
                        catch (const std::invalid_argument& e) {
                            throw InvalidValueException("Invalid value in list for argument '" + (argument.names.empty() ? key : argument.names[0]) + "'" + std::string(" (") + e.what() + ")");
                        }
                        catch (const std::out_of_range& e) {
                            throw InvalidValueException("Value out of range in list for argument '" + (argument.names.empty() ? key : argument.names[0]) + "'" + std::string(" (") + e.what() + ")");
                        }
                    }
                }
                // Run validator if present
                if (argument.validator) {
                    argument.validator(argument.parsedValue);
                }
            }
        }

        /// @brief Get the parsed argument value by name.
        /// @tparam T Expected argument type.
        /// @param name Argument name.
        /// @return Parsed argument value of type T.
        /// @throws UnknownArgumentException if the argument is not found.
        /// @throws TypeMismatchException if the argument type does not match T.
        template<typename T>
        T get(const std::string& name) const {
            std::string normName = normalizeName(name);
            auto lookupIt = m_nameLookup.find(normName);
            if (lookupIt == m_nameLookup.end()) throw UnknownArgumentException("Argument not found: " + name);
            const Arg& arg = m_arguments.at(lookupIt->second);
            // Special handling for bool: always optional, default is false if not present
            if constexpr (std::is_same_v<T, bool>) {
                if (std::holds_alternative<bool>(arg.parsedValue)) {
                    return std::get<bool>(arg.parsedValue);
                }
                return false;
            }
            // Handle vector types
            if constexpr (is_vector<T>::value) {
                if (std::holds_alternative<T>(arg.parsedValue)) {
                    return std::get<T>(arg.parsedValue);
                }
                else if (!std::holds_alternative<std::monostate>(arg.defaultValue) && std::holds_alternative<T>(arg.defaultValue)) {
                    return std::get<T>(arg.defaultValue);
                }
                throw TypeMismatchException("Type mismatch: argument '" + name + "' is not of type " + typeid(T).name() + ".");
            }
            else {
                if (std::holds_alternative<std::monostate>(arg.parsedValue)) {
                    if (!std::holds_alternative<std::monostate>(arg.defaultValue) && std::holds_alternative<T>(arg.defaultValue)) {
                        return std::get<T>(arg.defaultValue);
                    }
                    throw MissingArgumentException("Missing required argument: " + name);
                }
                if (std::holds_alternative<T>(arg.parsedValue)) {
                    return std::get<T>(arg.parsedValue);
                }
                throw TypeMismatchException("Type mismatch: argument '" + name + "' is not of type " + typeid(T).name() + ".");
            }
        }

        /// @brief Check if an argument was provided on the command line.
        /// @param name Argument name.
        /// @return True if the argument is present, false otherwise.
        bool has(const std::string& name) const {
            std::string normName = normalizeName(name);
            auto lookupIt = m_nameLookup.find(normName);
            if (lookupIt == m_nameLookup.end()) return false;
            const Arg& arg = m_arguments.at(lookupIt->second);
            return !std::holds_alternative<std::monostate>(arg.parsedValue);
        }

        /// @name Convenience getters for specific types
        /// @{
        int getInt(const std::string& name) const { return get<int>(name); }
        float getFloat(const std::string& name) const { return get<float>(name); }
        bool getBool(const std::string& name) const { return get<bool>(name); }
        std::string getString(const std::string& name) const { return get<std::string>(name); }

        std::vector<int> getInts(const std::string& name) const { return get<std::vector<int>>(name); }
        std::vector<float> getFloats(const std::string& name) const { return get<std::vector<float>>(name); }
        std::vector<bool> getBools(const std::string& name) const { return get<std::vector<bool>>(name); }
        std::vector<std::string> getStrings(const std::string& name) const { return get<std::vector<std::string>>(name); }
        /// @}

        /// @brief Print help message to stdout.
        /// @param programName The program's executable name (usually argv[0]).
        /// This prints a usage summary and all registered arguments, including their help text and default values.
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
                    std::string pos = !argument.longForms.empty() ? argument.longForms[0] : (argument.names.empty() ? key : argument.names[0]);
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
                    // Prefer showing short and long forms if available
                    if (!argument.shortForms.empty() && !argument.longForms.empty()) {
                        opt = "-" + argument.shortForms[0] + ", --" + argument.longForms[0];
                    }
                    else if (!argument.shortForms.empty()) {
                        opt = "-" + argument.shortForms[0];
                    }
                    else if (!argument.longForms.empty()) {
                        opt = "    --" + argument.longForms[0]; // 4 spaces for alignment
                    }
                    else if (!argument.names.empty()) {
                        // fallback to first registered name
                        std::string n = argument.names[0];
                        if (startsWith(n, "--")) opt = "    " + n; else if (startsWith(n, "-")) opt = n; else opt = n;
                    }
                    else {
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
                    }
                    else {
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
                    // Print aliases (remaining registered names) beneath the main option line
                    std::vector<std::string> aliases;
                    std::string displayedShort = !argument.shortForms.empty() ? argument.shortForms[0] : std::string();
                    std::string displayedLong = !argument.longForms.empty() ? argument.longForms[0] : std::string();
                    // collect short form aliases (preserve single-dash)
                    for (const auto& s : argument.shortForms) {
                        if (s == displayedShort) continue;
                        aliases.push_back(std::string("-") + s);
                    }
                    // collect long form aliases (preserve double-dash)
                    for (const auto& l : argument.longForms) {
                        if (l == displayedLong) continue;
                        aliases.push_back(std::string("--") + l);
                    }
                    // any remaining names not captured in shortForms/longForms: guess dash based on length
                    for (const auto& n : argument.names) {
                        bool inShort = std::find(argument.shortForms.begin(), argument.shortForms.end(), n) != argument.shortForms.end();
                        bool inLong = std::find(argument.longForms.begin(), argument.longForms.end(), n) != argument.longForms.end();
                        if (inShort || inLong) continue;
                        if (n.size() == 1) aliases.push_back(std::string("-") + n);
                        else aliases.push_back(std::string("--") + n);
                    }
                    if (!aliases.empty()) {
                        std::string aliasList;
                        for (size_t ai = 0; ai < aliases.size(); ++ai) {
                            if (ai > 0) aliasList += ", ";
                            aliasList += aliases[ai];
                        }
                        std::cout << gray << "  alias: [" << green << aliasList << reset << "] \n";
                    }
                }
            }
            // Help flag, aligned
            size_t helpPadName = maxOptNameLen > helpFlag.size() ? maxOptNameLen - helpFlag.size() : 0;
            std::cout << "  " << green << helpFlag << reset << std::string(helpPadName, ' ') << std::string(maxTypeLen + 1, ' ') << "  Show this help message\n";
        }

    private:
        bool m_useColors = true;

        /// @brief Variant to hold any supported argument value type.
        /// This variant is used for storing argument values of different types, including lists.
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

        /// @brief Supported argument types for validation and parsing.
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


        /// @struct Arg
        /// @brief Represents one command-line argument and its metadata.
        struct Arg {
            std::vector<std::string> names; ///< All normalized names/aliases (no leading dashes)
            std::vector<std::string> shortForms; ///< short forms (without dash)
            std::vector<std::string> longForms;  ///< long forms (without dashes)
            std::string help;       ///< Help/description string.
            bool required{ true };  ///< True if argument must be provided by the user.
            ArgType type{ ArgType::String }; ///< Argument type.
            Value defaultValue;     ///< Default value if any.
            Value parsedValue;     ///< Parsed value if any.
            bool positional{ false }; ///< True if this is a positional argument.
            std::function<void(const Value&)> validator; ///< Optional value validator
        };


        // Lookup map: maps all forms to canonical key
        std::unordered_map<std::string, std::string> m_nameLookup; ///< Maps argument names to canonical keys.
        std::unordered_map<std::string, Arg> m_arguments; ///< Map of all arguments.
        std::vector<std::string> m_positionalOrder; ///< Order of positional arguments.
        std::function<void(std::string)> m_helpHandler; ///< Function to handle help requests.
        int m_argc; ///< Argument count from main().
        char** m_argv; ///< Argument vector from main().

        /// @brief checks if a string starts with a given prefix
        static bool startsWith(const std::string& str, const std::string& prefix) {
            return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
        }

        /// @brief checks if a string represents a negative number
        static bool isNegativeNumber(const std::string& str) {
            if (!startsWith(str, "-") || str.size() <= 1) return false;
            
            // Check if the rest is a valid number (integer or float)
            std::string numberPart = str.substr(1);
            
            // Try to parse as float (which also handles integers)
            try {
                auto num = std::stof(numberPart);
                return true;
            } catch (const std::invalid_argument&) {
                return false;
            } catch (const std::out_of_range&) {
                return false;
            }
        }

        /// @brief normalize argument name (strip leading dashes)
        static std::string normalizeName(const std::string& name) {
            if (startsWith(name, "--")) return name.substr(2);
            if (startsWith(name, "-")) return name.substr(1);
            return name;
        }

        /// @brief Converts Value variant to string for defaults and printing.
        /// @param value The Value to convert.
        /// @return String representation of the value.
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

        /// @brief Check if a type is a std::vector.
        /// This is a type trait to determine if a type is a std::vector.
        /// @tparam T Type to check.
        template<typename T>
        struct is_vector : std::false_type {};
        /// @brief Specialization for std::vector types.
        /// @tparam T Element type of the vector.
        /// @tparam A Allocator type of the vector.
        template<typename T, typename A>
        struct is_vector<std::vector<T, A>> : std::true_type {};

        /// @brief Check if an ArgType represents a list type.
        /// @param type The ArgType to check.
        /// @return True if the type is a list type, false otherwise.
        static bool isListType(ArgType type) {
            return type == ArgType::StringList || type == ArgType::IntList ||
                type == ArgType::FloatList || type == ArgType::BoolList;
        }

        /// @brief Deduce ArgType enum from C++ type.
        /// @tparam T C++ type to deduce from.
        /// @return Corresponding ArgType value.
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