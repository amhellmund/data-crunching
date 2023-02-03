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

#ifndef DATA_CRUNCHING_INTERNAL_TYPE_CONVERSION_HPP
#define DATA_CRUNCHING_INTERNAL_TYPE_CONVERSION_HPP

#include <string>
#include <type_traits>

#include "data_crunching/internal/fixed_string.hpp"
#include "data_crunching/internal/type_list.hpp"

namespace dacr {

namespace internal {

// ############################################################################
// Trait: Conversion From String
// ############################################################################
template <typename T>
struct TypeConversion {
    static T fromString (const std::string& str) {
        return T{str};
    }
};

template <>
struct TypeConversion<char> {
    static char fromString (const std::string& str) {
        if (str.size() != 1) {
            throw std::invalid_argument("string cannot be converted to char");
        }
        return str[0];
    }
};

template <>
struct TypeConversion<int> {
    static int fromString (const std::string& str) {
        return std::stoi(str);
    }
};

template <>
struct TypeConversion<long int> {
    static long int fromString (const std::string& str) {
        return std::stol(str);
    }
};

template <>
struct TypeConversion<float> {
    static float fromString (const std::string& str) {
        return std::stof(str);
    }
};

template <>
struct TypeConversion<double> {
    static double fromString (const std::string& str) {
        return std::stod(str);
    }
};

template <typename T>
auto convertFromString (const std::string& str) -> std::optional<T> {
    try {
        return TypeConversion<T>::fromString(str);
    }
    catch (...) {
        return std::nullopt;
    }
}

template <typename T, typename Other>
requires (
    std::is_convertible_v<Other, T> ||
    requires {
        T{std::declval<Other>()};
    }
)
auto convertFromOther (const Other& other) {
    if constexpr (std::is_fundamental_v<T>) {
        return static_cast<T>(other);
    }
    else {
        return T{other};
    }
}

// ############################################################################
// Trait: Are Types Convertible From String
// ############################################################################
template <typename>
struct AreTypesConvertibleFromStringImpl : std::true_type {};

template <typename FirstType, typename ...RestTypes>
struct AreTypesConvertibleFromStringImpl<TypeList<FirstType, RestTypes...>> {
    static constexpr bool value = (
        std::is_arithmetic_v<FirstType> ||
        requires {
            FirstType{std::declval<std::string>()};
        }) && AreTypesConvertibleFromStringImpl<TypeList<RestTypes...>>::value;
};

template <typename Types>
constexpr bool are_types_convertible_from_string = AreTypesConvertibleFromStringImpl<Types>::value;

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_TYPE_CONVERSION_HPP