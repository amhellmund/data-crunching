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

#include "data_crunching/internal/fixed_string.hpp"
#include "data_crunching/namedtuple.hpp"
#include "data_crunching/internal/type_list.hpp"

namespace dacr {

struct Mnemonic {
    std::string short_arg;
};

inline auto mnemonic(std::string_view str) {
    return Mnemonic{.short_arg {str}};
}

struct Positional {
    bool value{false};
};

inline auto positional = Positional{.value = true};

struct Help {
    std::string text;
};

inline auto help (std::string_view str) {
    return Help{.text {str}};
}

template <typename T>
struct Optional {
    T value;
};

struct Store {
    bool store{true};
};

inline auto store(bool value) {
    return Store{.store = value};
}

template <typename T>
inline auto optional(T&& value) {
    return Optional<T>{.value = std::forward<T>(value)};
}

enum ArgSpec {
    PositionArgument = 0,
    OptionArgument = 1,
};

template <FixedString Name, typename Type, ArgSpec Spec = ArgSpec::OptionArgument>
struct Arg {
    template <typename ...Opts>
    Arg (Opts&& ...options) {
    }

    bool is_positional{false};
    std::optional<std::string> mnemonic;
    std::optional<std::string> help;
    Type value {};
};

namespace internal {

// ############################################################################
// Trait: Construct Argument Data (TypeList)
// ############################################################################
template <typename ...>
struct ConstructArgumentDataImpl {
    using type = TypeList<>;
};

template <FixedString FirstArgName, typename FirstArgType, typename ...RestArgs>
struct ConstructArgumentDataImpl<Arg<FirstArgName, FirstArgType>, RestArgs...> {
    using type = TypeListPrepend<
        Field<FirstArgName, FirstArgType>,
        typename ConstructArgumentDataImpl<RestArgs...>::type
    >;
};

template <typename ...Args>
using ConstructArgumentData = typename ConstructArgumentDataImpl<Args...>::type;


} // internal

template <typename ...Args>
class ArgumentParser {
public:
    template <typename ...CtorArgs>
    ArgumentParser (CtorArgs&& ...args) : arg_desc_{std::make_tuple(std::forward<CtorArgs>(args)...)} {}

    auto parse (int argc, char *argv[]) {
        if (containsHelpOption(argc, argv)) {
            printHelpText();
        }
        std::vector<std::string> args {argv, argv + argc};

        // iterate the arguments and pass
        // ArgumentProcessor processor;
        // for (int i = 0; i < argc; /* no auto increment */) {
        //     i += processor.processArg(arg_desc, args, i);
        // }
        // processor.validate();
        // processor.constructResult();

        using Result = typename internal::ConstructArgumentData<Args...>::template To<NamedTuple>;
        return Result{};
    }

private:
    //using ArgumentProcessor = ...;

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

    void printHelpText () const {}
};

template <typename ...CtorArgs>
ArgumentParser(CtorArgs&& ...args) -> ArgumentParser<CtorArgs...>;

} // namespace dacr

#endif // DATA_CRUNCHING_ARGPARSE_HPP