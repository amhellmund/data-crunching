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

internal::ArgConsumption consumeArgument (std::vector<std::string>&, int) {
    return {.status = internal::ArgConsumption::Status::NO_MATCH};
}

template <typename FirstArg, typename ...RestArgs>
internal::ArgConsumption consumeArgument(std::vector<std::string>& args, int pos, FirstArg& first_arg, RestArgs& ...rest_args) {
    auto result = first_arg.consume(args, pos);
    if (result.status == internal::ArgConsumption::Status::MATCH) {
        return result;
    }
    return consumeArgument(args, pos, rest_args...);
}

inline void exitWithError (const std::string& error_message) {
    std::cerr << "Error: " << error_message << "\n";
    std::exit(EXIT_FAILURE);
}

template <typename ...Args>
requires (sizeof...(Args) > 0)
class ArgumentParser {
public:
    template <typename ...CtorArgs>
    ArgumentParser (CtorArgs&& ...args) : arg_desc_{std::make_tuple(std::forward<CtorArgs>(args)...)} {}

    auto parse (int argc, char *argv[]) {
        if (containsHelpOption(argc, argv)) {
            printHelpText();
        }
        std::vector<std::string> args {argv + 1, argv + argc};

        auto validation_result = std::apply(internal::validateArgs, arg_desc_);
        if (not validation_result.success) {
            exitWithError(validation_result.error_message);
        }

        for (auto i = 0LU; i < args.size(); /* no auto increment */) {
            auto arg_consumption = std::apply(consumeArgument, arg_desc_);
            switch (arg_consumption.status) {
                case internal::ArgConsumption::Status::MATCH:
                    i += arg_consumption.consume_count;
                    break;
                case internal::ArgConsumption::Status::NO_MATCH:
                    exitWithError("unmatched argument: " + args[i]);
                    break;
                case internal::ArgConsumption::Status::ERROR:
                    exitWithError(arg_consumption.error_message);
                    break;
            }

        }

        using ReseultType = typename internal::ConstructArgumentData<Args...>::template To<NamedTuple>;
        ReseultType result{};
        auto store_result = std::apply(internal::storeValue, arg_desc_);
        return result;
    }

private:
    using ArgumentDescriptions = std::tuple<Args...>;
    ArgumentDescriptions arg_desc_;

    bool containsHelpOption (int argc, char *argv[]) const {
        for (int i = 0; i < argc; ++i) {
            if (std::strcmp(argv[i], "--help") == 0 or std::strcmp(argv[i], "-h") == 0) {
                return true;
            }
        }
        return false;
    }

    void printHelpText () const {
        std::cout << "HELP\n";
    }
};

template <typename ...CtorArgs>
ArgumentParser(CtorArgs&& ...args) -> ArgumentParser<CtorArgs...>;

} // namespace dacr

#endif // DATA_CRUNCHING_ARGPARSE_HPP