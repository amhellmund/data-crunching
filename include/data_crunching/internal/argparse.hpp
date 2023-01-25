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

#ifndef DATA_CRUNCHING_INTERNAL_ARGPARSE_HPP
#define DATA_CRUNCHING_INTERNAL_ARGPARSE_HPP

#include <string>

#include "data_crunching/internal/fixed_string.hpp"
#include "data_crunching/namedtuple.hpp"
#include "data_crunching/internal/type_conversion.hpp"
#include "data_crunching/internal/type_list.hpp"
#include "data_crunching/internal/utils.hpp"

namespace dacr {

namespace internal {

template <FixedString Name, typename Type>
class ArgImpl;

// ############################################################################
// Trait: Spec classes
// ############################################################################
struct Mnemonic {
    std::string short_arg{};
};

struct Positional {
};

struct Required {  
};

struct Help {
    std::string text{};
};

template <typename T>
struct Optional {
    T value;
};

struct Store {
    bool value{true};
};

// ############################################################################
// Trait: Get Mnemonic
// ############################################################################
std::optional<std::string> getMnemonic () {
    return std::nullopt;
}

template <typename FirstSpec, typename ...RestSpecs>
std::optional<std::string> getMnemonic (FirstSpec first_spec, RestSpecs ...rest_specs) {
    return getMnemonic(rest_specs...);
}

template <typename ...RestSpecs>
std::optional<std::string> getMnemonic(Mnemonic mnemonic, RestSpecs ...rest_specs) {
    return mnemonic.short_arg;
}

// ############################################################################
// Trait: Get Help
// ############################################################################
std::optional<std::string> getHelp () {
    return std::nullopt;
}

template <typename FirstSpec, typename ...RestSpecs>
std::optional<std::string> getHelp (FirstSpec first_spec, RestSpecs ...rest_specs) {
    return getHelp(rest_specs...);
}

template <typename ...RestSpecs>
std::optional<std::string> getHelp(Help help, RestSpecs ...rest_specs) {
    return help.text;
}

// ############################################################################
// Trait: Get Optional
// ############################################################################
auto getOptional () {
    return std::nullopt;
}

template <typename FirstSpec, typename ...RestSpecs>
auto getOptional (FirstSpec first_spec, RestSpecs ...rest_specs) {
    return getOptional(rest_specs...);
}

template <typename T, typename ...RestSpecs>
auto getOptional(Optional<T> optional, RestSpecs ...rest_specs) {
    return optional.value;
}

// ############################################################################
// Trait: Get Store
// ############################################################################
template <typename FirstSpec, typename ...RestSpecs>
std::optional<bool> getStore (FirstSpec first_spec, RestSpecs ...rest_specs) {
    if constexpr(sizeof...(RestSpecs) > 0) {
        return getStore(rest_specs...);
    }
    else {
        return std::nullopt;
    }
}

template <typename ...RestSpecs>
std::optional<bool> getStore(Store store, RestSpecs ...rest_specs) {
    return store.value;
}

// ############################################################################
// Trait: Is Spec Contained In Specs
// ############################################################################
template <typename SearchType, typename ...Specs>
struct IsSpecContainedInSpecsImpl : std::false_type {};

template <typename SearchType, typename FirstSpec, typename ...RestSpecs>
struct IsSpecContainedInSpecsImpl<SearchType, FirstSpec, RestSpecs...> {
    static constexpr bool value = std::is_same_v<SearchType, std::remove_reference_t<FirstSpec>> || IsSpecContainedInSpecsImpl<SearchType, RestSpecs...>::value;
};

template <typename SearchType, typename ...Specs>
constexpr bool is_spec_contained_in_specs = IsSpecContainedInSpecsImpl<SearchType, Specs...>::value;

// ############################################################################
// Trait & Concept: Is Optional
// ############################################################################
template <typename T>
struct IsOptionalImpl : std::false_type {};

template <typename T>
struct IsOptionalImpl<std::optional<T>> : std::true_type {};

template <typename T>
constexpr bool is_optional = IsOptionalImpl<T>::value;

template <typename T>
concept IsOptional = is_optional<T>;

// ############################################################################
// Trait & Concept: Is Valid Spec for Arg
// ############################################################################
template <typename>
struct IsValidSpecForArgImpl : std::false_type {};

template<>
struct IsValidSpecForArgImpl<Positional> : std::true_type {};

template <>
struct IsValidSpecForArgImpl<Mnemonic> : std::true_type {};

template <>
struct IsValidSpecForArgImpl<Help> : std::true_type {};

template <typename T>
struct IsValidSpecForArgImpl<Optional<T>> : std::true_type {};

template <typename T>
constexpr bool is_valid_spec_for_arg = IsValidSpecForArgImpl<T>::value;

template <typename T>
concept SpecForArg = is_valid_spec_for_arg<T>;


// ############################################################################
// Trait & Concept: Is Valid Type For Arg
// ############################################################################


// ############################################################################
// Trait & Concept: Is Valid Spec for Optional Arg
// ############################################################################
template <typename>
struct IsValidSpecForOptionalArgImpl : std::false_type {};

template <>
struct IsValidSpecForOptionalArgImpl<Mnemonic> : std::true_type {};

template <>
struct IsValidSpecForOptionalArgImpl<Help> : std::true_type {};

template <typename T>
constexpr bool is_valid_spec_for_optional_arg = IsValidSpecForOptionalArgImpl<T>::value;

template <typename T>
concept SpecForOptionalArg = is_valid_spec_for_optional_arg<T>;

// ############################################################################
// Trait & Concept: Is Valid Spec for Switch Arg
// ############################################################################
template <typename>
struct IsValidSpecForSwitchArgImpl : std::false_type {};

template <>
struct IsValidSpecForSwitchArgImpl<Mnemonic> : std::true_type {};

template <>
struct IsValidSpecForSwitchArgImpl<Help> : std::true_type {};

template <>
struct IsValidSpecForSwitchArgImpl<Store> : std::true_type {};

template <typename T>
constexpr bool is_valid_spec_for_switch_arg = IsValidSpecForSwitchArgImpl<T>::value;

template <typename T>
concept SpecForSwitchArg = is_valid_spec_for_switch_arg<T>;

// ############################################################################
// Trait & Concept: Is Valid Spec for N-Ary Arg
// ############################################################################
template <typename>
struct IsValidSpecForNAryArgImpl : std::false_type {};

template <>
struct IsValidSpecForNAryArgImpl<Mnemonic> : std::true_type {};

template <>
struct IsValidSpecForNAryArgImpl<Help> : std::true_type {};

template <>
struct IsValidSpecForNAryArgImpl<Required> : std::true_type {};

template <>
struct IsValidSpecForNAryArgImpl<Positional> : std::true_type {};

template <typename T>
constexpr bool is_valid_spec_for_n_ary_arg = IsValidSpecForNAryArgImpl<T>::value;

template <typename T>
concept SpecForNAryArg = is_valid_spec_for_n_ary_arg<T>;

// ############################################################################
// Trait & Concept: Is Valid Type For Arg
// ############################################################################
template <typename T>
concept TypeForArg = (
    internal::IsArithmetic<T> ||
    requires {
        T{std::declval<std::string>()};
    }
);

template <typename T>
concept TypeForOptionalArg = (
    TypeForArg<T> &&
    not std::is_same_v<T, bool>
);

template <typename T>
concept TypeForNAryArg = (
    TypeForArg<T> &&
    not std::is_same_v<T, bool>
);

// ############################################################################
// Class: Common Arg Data
// ############################################################################
struct ArgCommonData {
    std::string arg_name;
    std::optional<std::string> mnemonic;

    bool is_required {false};
    bool is_positional {false};
    bool is_n_ary {false};
    
    std::optional<std::string> help;

    bool is_matched {false};
};

template <typename ...Specs>
auto getArgCommonData (const std::string& arg_name, Specs&& ...specs) {
    ArgCommonData result{};
    if constexpr (is_spec_contained_in_specs<Positional, Specs...>) {
        result.is_positional = true;
    }
    if constexpr (is_spec_contained_in_specs<Mnemonic, Specs...>) {
        result.mnemonic = getMnemonic(specs...);
    }
    if constexpr (is_spec_contained_in_specs<Help, Specs...>) {
        result.help = getHelp(specs...);
    }
    if constexpr (is_spec_contained_in_specs<Required, Specs...>) {
        result.is_required = true;
    }
    result.arg_name = arg_name;
    return result;
}

// ############################################################################
// Utilities
// ############################################################################
inline bool isPositionalArgument (const std::string& arg) {
    return (not arg.starts_with("-"));
}

inline bool isArgumentMatched (const std::string& arg, const ArgCommonData& common_arg_data) {
    if (arg.size() > 2 and arg.starts_with("--")) {
        return arg.substr(2) == common_arg_data.arg_name;
    }
    else if (arg.size() > 1 and arg.starts_with("-")) {
        return common_arg_data.mnemonic.has_value() and arg.substr(1) == *common_arg_data.mnemonic;
    }
    return false;
}

// ############################################################################
// Trait: Construct Argument Data (TypeList)
// ############################################################################
template <typename ...>
struct ConstructArgumentDataImpl {
    using type = TypeList<>;
};

template <FixedString FirstArgName, typename FirstArgType, typename ...RestArgs>
struct ConstructArgumentDataImpl<ArgImpl<FirstArgName, FirstArgType>, RestArgs...> {
    using type = TypeListPrepend<
        Field<FirstArgName, FirstArgType>,
        typename ConstructArgumentDataImpl<RestArgs...>::type
    >;
};

template <typename ...Args>
using ConstructArgumentData = typename ConstructArgumentDataImpl<Args...>::type;

// ############################################################################
// Class: Argument Consumption
// ############################################################################
struct ArgConsumption {
    enum class Status {
        ERROR = -1,
        NO_MATCH = 0,
        MATCH = 1,
    };

    Status status;
    int consume_count{-1};
    std::string error_message{};
};

// ############################################################################
// Class: Argument
// ############################################################################
template <FixedString Name, typename Type>
class ArgImpl;

template <typename T>
internal::ArgConsumption consumePositional (std::optional<T>& value, ArgCommonData& common_data, const std::string& arg) {
    if (not common_data.is_matched) {
        auto converted_arg = convertFromString<T>(arg);
        if (converted_arg.has_value()) {
            value = *converted_arg;
            common_data.is_matched = true;
            return {.status = ArgConsumption::Status::MATCH, .consume_count = 1};
        }
        else {
            return {.status = ArgConsumption::Status::ERROR, .error_message = "argument conversion failed"};
        }
        }
    else {
        return {.status = ArgConsumption::Status::NO_MATCH};
    }
}

template <typename T>
internal::ArgConsumption consumeArgument (std::optional<T>& value, ArgCommonData& common_data, const std::vector<std::string>& args, int pos) {
    if (pos + 1 >= args.size()) {
        return {.status = ArgConsumption::Status::ERROR, .error_message = "missing argument"};
    }
    auto converted_arg = convertFromString<T>(args[pos + 1]);
    if (converted_arg.has_value()) {
        value = *converted_arg;
        common_data.is_matched = true;
        return {.status = ArgConsumption::Status::MATCH, .consume_count = 2};
    }
    else {
        return {.status = ArgConsumption::Status::ERROR, .error_message = "argument conversion failed"};
    }
}

template <FixedString Name, internal::TypeForArg Type>
class ArgImpl<Name, Type> {
public:
    template <internal::SpecForArg ...Specs>
    ArgImpl (Specs&& ...specs) : common_data{getArgCommonData(Name.toString(), specs...)} {
        auto optional = getOptional(specs...);
        if constexpr (not std::is_same_v<decltype(optional), std::nullopt_t>) {
            value = optional;
        }
        else {
            common_data.is_required = true;
        }
    }

    internal::ArgConsumption consume (const std::vector<std::string>& args, int pos) {
        if (common_data.is_positional and isPositionalArgument(args[pos])) {
            return consumePositional(value, common_data, args[pos]);
        }
        else if (isArgumentMatched(args[pos], common_data)) {
            return consumeArgument(value, common_data, args, pos);
        }
        return {.status = ArgConsumption::Status::NO_MATCH};
    }
    
    template <typename NamedTuple>
    void storeResult (NamedTuple& nt) {
        nt.template get<Name>() = *value;
    }

    auto getValue () const {
        return value;
    }

private:
    internal::ArgCommonData common_data{};
    std::optional<Type> value{};
};

template <FixedString Name, TypeForOptionalArg Type>
class ArgImpl<Name, std::optional<Type>> {
public:
    template <SpecForOptionalArg ...Specs>
    ArgImpl (Specs&& ...specs) : common_data{getArgCommonData(Name.toString(), specs...)} {}

    internal::ArgConsumption consume (const std::vector<std::string>& args, int pos) {
        if (isArgumentMatched(args[pos], common_data)) {
            return consumeArgument(value, common_data, args, pos);
        }
        return {.status = ArgConsumption::Status::NO_MATCH};
    }

    template <typename NamedTuple>
    void storeResult (NamedTuple& nt) {
        nt.template get<Name>() = value;
    }

    auto getValue () const {
        return value;
    }

private:
    ArgCommonData common_data{};
    std::optional<Type> value{};
};

template <FixedString Name>
class ArgImpl<Name, bool> {
public:
    template <SpecForSwitchArg ...Specs>
    ArgImpl (Specs&& ...specs) : common_data{getArgCommonData(Name.toString(), specs...)} {
        value = not getStore(specs...);
    }

    internal::ArgConsumption consume (const std::vector<std::string>& args, int pos) {
        if (isArgumentMatched(args[pos], common_data)) {
            if (not common_data.is_matched) {
                value = not value;
                common_data.is_matched = true;
            }
            return {.status = ArgConsumption::Status::MATCH, .consume_count = 1};
        }
        return {.status = ArgConsumption::Status::NO_MATCH};
    }

    template <typename NamedTuple>
    void storeResult (NamedTuple& nt) {
        nt.template get<Name>() = value;
    }

private:
    ArgCommonData common_data;
    bool value {};
};

// template <FixedString Name, TypeForNAryArg Type>
// class Arg<Name, std::vector<Type>> {
//     template <SpecForNAryArg ...Specs>
//     Arg (Specs&& ...specs) : common_data{getArgCommonData(Name.toString(), specs...)} {
//         common_data.is_n_ary = true;
//     }

//     internal::ArgConsumption consume (const std::vector<std::string>& args, int pos) {
//         if (common_data.is_positional and isPositionalArgument(args[pos])) {
//             return consumePositional(value, common_data, args[pos]);
//         }
//         else if (isArgumentMatched(args[pos], common_data)) {
//             return consumeArgument(value, common_data, args, pos);
//         }
//         return {.status = ArgConsumption::Status::NO_MATCH};
//     }

//     template <typename NamedTuple>
//     void storeResult (NamedTuple& nt) {
//         nt.template get<Name>() = values;
//     }

// private:
//     ArgCommonData common_data;
//     std::vector<Type> values;
// };

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_ARGPARSE_HPP