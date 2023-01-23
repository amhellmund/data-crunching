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

template <FixedString Name, typename Type>
struct Arg;

template <FixedString Name, TypeForArg Type>
struct Arg<Name, Type> {
    template <SpecForArg ...Specs>
    Arg (Specs&& ...specs) : common_data{getArgCommonData(Name.toString(), specs...)} {
        auto optional = GetOptional(options...);
        if constexpr (not std::is_same_v<decltype(optional), std::nullopt_t>) {
            value = Type{optional};
        }
        else {
            common_data.is_required = true;
        }
    }

    // std::optional<int> consume (const std::vector<std::string>& args, int pos) {
    //     if (common_data.is_positional and isPositional(args[pos])) {
    //         value = Type{args[pos]};
    //         common_data.has_match = true;
    //         return 1;
    //     }
    //     else if (hasMatch(args[pos], common_data)) {
    //         if (pos + 1 >= args.size()) {
    //             // missing arguments
    //             // throw exception
    //         }
    //         value = Type{args[pos+1]};
    //         common_data.has_match = true;
    //         return 2;
    //     }
    //     return std::nullopt;
    // }
    
    // template <typename NamedTuple>
    // void storeResult (NamedTuple& nt) {
    //     nt.template get<Name>() = *value;
    // }

    ArgCommonData common_data{};
    std::optional<Type> value{};
};

template <FixedString Name, TypeForOptionalArg Type>
struct Arg<Name, std::optional<Type>> {

    template <SpecForOptionalArg ...Specs>
    Arg (Specs&& ...specs) : common_data{getArgCommonData(Name.toString(), specs...)} {}

    // std::optional<int> consume (const std::vector<std::string>& args, int pos) {
    //     if (hasMatch(args[pos], common_data)) {
    //         if (pos + 1 >= args.size()) {
    //             // missing arguments
    //             // throw exception
    //         }
    //         value = args[pos+1];
    //         common_data.has_match = true;
    //         return 2;
    //     }
    //     return std::nullopt;
    // }

    // template <typename NamedTuple>
    // void storeResult (NamedTuple& nt) {
    //     nt.template get<Name>() = value;
    // }

    ArgCommonData common_data{};
    std::optional<Type> value{};
};

template <FixedString Name>
struct Arg<Name, bool> {
    template <SpecForSwitch ...Specs>
    Arg (Specs&& ...specs) : common_data{getArgCommonData(Name.toString(), specs...)} {
        value = not GetStore(options...);
    }

    // std::optional<int> consume (const std::vector<std::string>& args, int pos) {
    //     if (hasMatch(args[pos], common_data)) {
    //         if (not common_data.has_match) {
    //             value = not value;
    //             common_data.has_match = true;
    //         }
    //         return 1;
    //     }
    //     return std::nullopt;
    // }

    // template <typename NamedTuple>
    // void storeResult (NamedTuple& nt) {
    //     nt.template get<Name>() = value;
    // }

    ArgCommonData common_data;
    bool value {};
};

template <FixedString Name, TypeForNAryArg Type>
struct Arg<Name, std::vector<Type>> : ArgBase{
    template <SpecForNAryArg ...Specs>
    Arg (Specs&& ...specs) : common_data{getArgCommonData(Name.toString(), options...)} {
        common_data.is_n_ary = true;
    }

    // std::optional<int> consume (const std::vector<std::string>& args, int pos) {
    //     if (common_data.is_positional and isPositional(args[pos])) {
    //         values.push_back(std::stoi(args[pos]));
    //         common_data.has_match = true;
    //         return 1;
    //     }
    //     else if (hasMatch(args[pos], common_data)) {
    //         if (pos + 1 >= args.size()) {
    //             // missing arguments
    //             // throw exception
    //         }
    //         values.push_back(std::stoi(args[pos]));
    //         return 2;
    //     }
    //     return std::nullopt;
    // }

    // template <typename NamedTuple>
    // void storeResult (NamedTuple& nt) {
    //     nt.template get<Name>() = values;
    // }

    ArgCommonData common_data;
    std::vector<Type> values;
};

template <typename ...Args>
class ArgumentParser {
public:
    template <typename ...CtorArgs>
    ArgumentParser (CtorArgs&& ...args) : arg_desc_{std::make_tuple(std::forward<CtorArgs>(args)...)} {}

    // auto parse (int argc, char *argv[]) {
    //     if (containsHelpOption(argc, argv)) {
    //         printHelpText();
    //     }
    //     std::vector<std::string> args {argv + 1, argv + argc};

    //     auto args_in_vector = getArgsInVector(std::index_sequence_for<Args...>{});
    //     for (int i = 0; i < args.size(); /* no auto increment */) {
    //         std::optional<int> consumed;
    //         for (int a = 0LU; a < args_in_vector.size(); ++a) {
    //             consumed = args_in_vector[a]->consume(args, i);
    //             if (consumed.has_value()) {
    //                 break;
    //             }
    //         }
    //         if (consumed.has_value()) {
    //             i += *consumed;
    //         }
    //         else {
    //             // error
    //         }
    //     }
    //     using Result = typename internal::ConstructArgumentData<Args...>::template To<NamedTuple>;
    //     Result result{};
    //     storeResult(result, std::index_sequence_for<Args...>{});
    //     return result;
    // }

private:
    // template <typename NamedTuple, std::size_t ...Indices>
    // void storeResult (NamedTuple& nt, std::integer_sequence<std::size_t, Indices...>) {
    //     ((std::get<Indices>(arg_desc_).storeResult(nt)), ...);
    // }

    // template <std::size_t ...Indices>
    // std::vector<ArgBase*> getArgsInVector (std::integer_sequence<std::size_t, Indices...>) {
    //     std::vector<ArgBase*> result;
    //     ((result.push_back(&std::get<Indices>(arg_desc_))), ...);
    //     return result;
    // }

    // using ArgumentDescriptions = std::tuple<Args...>;
    // ArgumentDescriptions arg_desc_;

    // bool containsHelpOption (int argc, char *argv[]) const {
    //     for (int i = 0; i < argc; ++i) {
    //         if (std::strcmp(argv[i], "--help") == 0 or std::strcmp(argv[i], "-h") == 0) {
    //             return true;
    //         }
    //     }
    //     return false;
    // }

    // void printHelpText () const {
    //     std::cout << "HELP\n";
    // }
};

template <typename ...CtorArgs>
ArgumentParser(CtorArgs&& ...args) -> ArgumentParser<CtorArgs...>;

} // namespace dacr

#endif // DATA_CRUNCHING_ARGPARSE_HPP