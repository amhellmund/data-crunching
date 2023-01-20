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
#include "data_crunching/internal/utils.hpp"

#include <exception>

namespace dacr {

struct Mnemonic {
    std::string short_arg;
};

inline auto mnemonic(std::string_view str) {
    return Mnemonic{.short_arg {str}};
}

struct Positional {
};

inline auto positional() {
    return Positional{};
}

struct Required {  
};

inline auto required() {
    return Required{};
}

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

template <typename T>
inline auto optional(T&& value) {
    return Optional<T>{.value = std::forward<T>(value)};
}

struct Store {
    bool store{true};
};

inline auto store(bool value) {
    return Store{.store = value};
}

template <typename F, typename ...Rest>
auto get_mnemonic (F f, Rest ...rest) {
    if constexpr (sizeof...(Rest) > 0) {
        return get_mnemonic(rest...);
    }
    else {
        return std::nullopt;
    }
}

template <typename ...Rest>
auto get_mnemonic(Mnemonic m, Rest ...rest) {
    return m.short_arg;
}


template <typename F, typename ...Rest>
auto get_help (F f, Rest ...rest) {
    if constexpr (sizeof...(Rest) > 0) {
        return get_mnemonic(rest...);
    }
    else {
        return std::nullopt;
    }
}

template <typename ...Rest>
auto get_help(Help m, Rest ...rest) {
    return m.text;
}

template <typename F, typename ...Rest>
auto get_optional (F f, Rest ...rest) {
    if constexpr (sizeof...(Rest) > 0) {
        return get_optional(rest...);
    }
    else {
        return std::nullopt;
    }
}

template <typename T, typename ...Rest>
auto get_optional(Optional<T> m, Rest ...rest) {
    return m.value;
}

template <typename SearchType, typename ...Specs>
struct SpecIsContainedImpl : std::false_type {};

template <typename SearchType, typename FirstSpec, typename ...RestSpecs>
struct SpecIsContainedImpl<SearchType, FirstSpec, RestSpecs...> {
    static constexpr bool value = std::is_same_v<SearchType, FirstSpec> || SpecIsContainedImpl<SearchType, RestSpecs...>::value;
};

template <typename SearchType, typename ...Specs>
constexpr bool is_spec_contained = SpecIsContainedImpl<SearchType, Specs...>::value;



template <typename>
struct IsSpecForArgImpl : std::false_type {};

template<>
struct IsSpecForArgImpl<Positional> : std::true_type {};

template <>
struct IsSpecForArgImpl<Mnemonic> : std::true_type {};

template <>
struct IsSpecForArgImpl<Help> : std::true_type {};

template <typename T>
struct IsSpecForArgImpl<Optional<T>> : std::true_type {};

template <typename T>
concept IsSpecForArg = IsSpecForArgImpl<T>::value;



template <typename>
struct IsSpecForOptionalArgImpl : std::false_type {};

template<>
struct IsSpecForOptionalArgImpl<Positional> : std::true_type {};

template <>
struct IsSpecForOptionalArgImpl<Mnemonic> : std::true_type {};

template <>
struct IsSpecForOptionalArgImpl<Help> : std::true_type {};

template <typename T>
concept IsSpecForOptionalArg = IsSpecForOptionalArgImpl<T>::value;



template <typename>
struct IsSpecForSwitchImpl : std::false_type {};

template <>
struct IsSpecForSwitchImpl<Mnemonic> : std::true_type {};

template <>
struct IsSpecForSwitchImpl<Help> : std::true_type {};

template <>
struct IsSpecForSwitchImpl<Store> : std::true_type {};

template <typename T>
concept IsSpecForSwitch = IsSpecForSwitchImpl<T>::value;



template <typename>
struct IsSpecForNAryImpl : std::false_type {};

template <>
struct IsSpecForNAryImpl<Mnemonic> : std::true_type {};

template <>
struct IsSpecForNAryImpl<Help> : std::true_type {};

template <>
struct IsSpecForNAryImpl<Required> : std::true_type {};

template <>
struct IsSpecForNAryImpl<Positional> : std::true_type {};

template <typename T>
concept IsSpecForNAry = IsSpecForNAryImpl<T>::value;


template <typename T>
concept IsValidTypeForArg = (
    internal::IsArithmetic<T> ||
    requires {
        T{std::declval<std::string>()};
    }
);


struct ArgCommonData {
    bool is_required {false};
    bool is_positional {false};
    bool is_n_ary {false};
    std::optional<std::string> mnemonic;
    std::optional<std::string> help;

    bool has_match {false};
};

template <typename ...Opts>
auto getArgCommonData (Opts&& ...options) {
    ArgCommonData result{};
    if constexpr (is_spec_contained<Positional, Opts...>) {
        result.is_positional = true;
    }
    if constexpr (is_spec_contained<Mnemonic, Opts...>) {
        result.mnemonic = get_mnemonic(options...);
    }
    if constexpr (is_spec_contained<Help, Opts...>) {
        result.help = get_help(options...);
    }
    if constexpr (is_spec_contained<Required, Opts...>) {
        result.is_required = true;
    }
    return result;
}

template <FixedString Name, typename Type>
struct Arg;

template <FixedString Name, IsValidTypeForArg Type>
struct Arg<Name, Type> {
    static_assert(IsValidTypeForArg<Type>);
    // concept for ordinary args
    template <IsSpecForArg ...Opts>
    Arg (Opts&& ...options) : common_data{getArgCommonData(options...)} {
        auto optional = get_optional(options...);
        if constexpr (not std::is_same_v<decltype(optional), std::nullopt_t>) {
            value = optional;
        }
        else {
            common_data.is_required = true;
        }
    }

    ArgCommonData common_data{};
    Type value {};
};

// concept for optional types
template <FixedString Name, IsValidTypeForArg Type>
struct Arg<Name, std::optional<Type>> {

    template <IsSpecForOptionalArg ...Opts>
    Arg (Opts&& ...options) : common_data{getArgCommonData(options...)} {}

    ArgCommonData common_data{};
    std::optional<Type> value {};
};

// concept for switches
template <FixedString Name>
struct Arg<Name, bool> {
    template <IsSpecForSwitch ...Opts>
    Arg (Opts&& ...options) : common_data{getArgCommonData(options...)} {

    }

    ArgCommonData common_data;
    bool value {};
};

// concept for n-ary
template <FixedString Name, IsValidTypeForArg Type>
struct Arg<Name, std::vector<Type>> {
    template <IsSpecForNAry ...Opts>
    Arg (Opts&& ...options) : common_data{getArgCommonData(options...)} {
        common_data.is_n_ary = true;
    }

    ArgCommonData common_data;
    std::vector<Type> values;
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