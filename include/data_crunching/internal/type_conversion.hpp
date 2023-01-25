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

#include "data_crunching/internal/fixed_string.hpp"

namespace dacr {

namespace internal {

// ############################################################################
// Trait: Conversion From String
// ############################################################################{
template <typename T>
struct TypeConversion {
    static T fromString (const std::string& str) {
        return T{str};
    }
};

template <>
struct TypeConversion<int> {
    static int fromString (const std::string& str) {
        return std::stoi(str);
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

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_TYPE_CONVERSION_HPP