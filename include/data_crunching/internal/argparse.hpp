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

namespace dacr::internal {

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
template <typename FirstSpec, typename ...RestSpecs>
std::optional<std::string> getMnemonic (FirstSpec first_spec, RestSpecs ...rest_specs) {
    if constexpr (sizeof...(RestSpecs) > 0) {
        return getMnemonic(rest_specs...);
    }
    else {
        return std::nullopt;
    }
}

template <typename ...RestSpecs>
std::optional<std::string> getMnemonic(Mnemonic mnemonic, RestSpecs ...rest_specs) {
    return std::string("-") + mnemonic.short_arg;
}

// ############################################################################
// Trait: Get Help
// ############################################################################
template <typename FirstSpec, typename ...RestSpecs>
std::optional<std::string> getHelp (FirstSpec first_spec, RestSpecs ...rest_specs) {
    if constexpr (sizeof...(RestSpecs) > 0) {
        return getHelp(rest_specs...);
    }
    else {
        return std::nullopt;
    }
}

template <typename ...RestSpecs>
std::optional<std::string> getHelp(Help help, RestSpecs ...rest_specs) {
    return help.text;
}

// ############################################################################
// Trait: Get Optional
// ############################################################################
template <typename FirstSpec, typename ...RestSpecs>
auto getOptional (FirstSpec first_spec, RestSpecs ...rest_specs) {
    if constexpr (sizeof...(RestSpecs) > 0) {
        return getOptional(rest_specs...);
    }
    else {
        return std::nullopt;
    }
}

template <typename T, typename ...RestSpecs>
auto getOptional(Optional<T> optional, RestSpecs ...rest_specs) {
    return optional.value;
}

// template <typename F, typename ...Rest>
// auto get_store (F f, Rest ...rest) {
//     if constexpr (sizeof...(Rest) > 0) {
//         return get_store(rest...);
//     }
//     else {
//         return true;
//     }
// }

// template <typename ...Rest>
// auto get_store(Store m, Rest ...rest) {
//     return m.store;
// }


} // namespace dacr::internal

#endif // DATA_CRUNCHING_INTERNAL_ARGPARSE_HPP