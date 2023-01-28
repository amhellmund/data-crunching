// Copyright 2022 Andi Hellmund
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#ifndef DATA_CRUNCHING_ARGPARSE_HPP
#define DATA_CRUNCHING_ARGPARSE_HPP

#include <optional>
#include <string>
#include <string_view>

#include "data_crunching/internal/argparse.hpp"

#include <exception>

namespace dacr {

class ArgumentException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// ############################################################################
// Trait: Spec functions
// ############################################################################
inline auto mnemonic(std::string_view str) {
    return internal::Mnemonic{.short_arg {str}};
}

inline auto positional() {
    return internal::Positional{};
}

inline auto required() {
    return internal::Required{};
}

inline auto help (std::string_view str) {
    return internal::Help{.text {str}};
}

template <typename T>
inline auto optional(T&& value) {
    return internal::Optional<T>{.value = std::forward<T>(value)};
}

inline auto store(bool value) {
    return internal::Store{.value = value};
}

template <FixedString Name, typename T, typename ...Specs>
inline auto Arg (Specs&& ...specs) {
    return internal::ArgImpl<Name, T>(std::forward<Specs>(specs)...);
}

inline void exitWithError (const std::string& error_message) {
    std::cerr << "[ERROR] " << error_message << "\n";
    std::exit(EXIT_FAILURE);
}

template <typename ...Args>
requires (sizeof...(Args) > 0)
class ArgumentParser {
public:
    template <typename ...CtorArgs>
    ArgumentParser (const std::string& program_description, CtorArgs&& ...args) : program_description_{program_description}, arg_desc_{std::make_tuple(std::forward<CtorArgs>(args)...)} {}

    auto parse (int argc, char *argv[]) {
        if (containsHelpOption(argc, argv)) {
            printHelpText(argv[0]);
        }
        std::vector<std::string> arguments {argv + 1, argv + argc};

        auto validation_result = std::apply(
            [](const auto& ...args) {
                return internal::validateArgs(args...);
            },
            arg_desc_
        );
        if (not validation_result.success) {
            exitWithError(validation_result.error_message);
        }

        for (auto i = 0LU; i < arguments.size(); /* no auto increment */) {
            auto arg_consumption = std::apply(
                [arguments, i](auto& ...args) {
                    return internal::consumeArgument(arguments, i, args...);
                },
                arg_desc_
            );
            switch (arg_consumption.status) {
                case internal::ArgConsumption::Status::MATCH:
                    i += arg_consumption.consume_count;
                    break;
                case internal::ArgConsumption::Status::NO_MATCH:
                    exitWithError("unmatched argument: " + arguments[i]);
                    break;
                case internal::ArgConsumption::Status::ERROR:
                    exitWithError(arg_consumption.error_message);
                    break;
            }
        }

        using ResultType = typename internal::ConstructArgumentData<Args...>::template To<NamedTuple>;
        ResultType result{};
        auto store_result = std::apply(
            [&result](const auto& ...args) {
                return internal::storeValue(result, args...);
            },
            arg_desc_
        );
        if (not store_result.success) {
            exitWithError(store_result.error_message);
        }
        return result;
    }

private:
    using ArgumentDescriptions = std::tuple<Args...>;

    bool containsHelpOption (int argc, char *argv[]) const {
        for (int i = 0; i < argc; ++i) {
            if (std::strcmp(argv[i], "--help") == 0 or std::strcmp(argv[i], "-h") == 0) {
                return true;
            }
        }
        return false;
    }

    void printHelpText (const std::string& program_name) const {
        std::cout << program_description_ << "\n";
        auto common_args = std::apply(
            [](const auto& ...args) {
                return internal::collectArgCommonData(args...);
            },
            arg_desc_
        );

        std::cout << "\n";
        std::cout << program_name;
        for (const auto& data : common_args) {
            if (data.is_positional) {
                
            }
            else {
                
            }
        }
        

        std::cout << "\n";
        std::cout << "Positional\n";
        std::cout << "----------\n";
        for (const auto& data : common_args) {
            if (data.is_positional) {
                std::cout << std::string(2, ' ') << data.arg_name << ": " << (data.help.has_value() ? *data.help : "");
                if (data.is_required) {
                    std::cout << " [required]";
                }
                if (data.is_n_ary) {
                    std::cout << " [n-ary]";
                }
                std::cout << "\n";
            }
        }

        std::cout << "\n";
        std::cout << "Arguments\n";
        std::cout << "---------\n";
        for (const auto& data : common_args) {
            if (not data.is_positional) {
                std::cout << std::string(2, ' ') << "--" << data.arg_name; 
                if (data.mnemonic.has_value()) {
                    std::cout << " (-" << *data.mnemonic << ")";
                }
                std::cout << ": " << (data.help.has_value() ? *data.help : "");
                if (data.is_required) {
                    std::cout << " [required]";;
                }
                if (data.is_n_ary) {
                    std::cout << " [n-ary]";
                }
                std::cout << "\n";
            }
        }


        std::exit(EXIT_SUCCESS);
    }

    std::string program_description_;
    ArgumentDescriptions arg_desc_;
};

template <typename ...CtorArgs>
ArgumentParser(const std::string&, CtorArgs&& ...args) -> ArgumentParser<CtorArgs...>;

} // namespace dacr

#endif // DATA_CRUNCHING_ARGPARSE_HPP