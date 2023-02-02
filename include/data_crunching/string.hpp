// Copyright 2023 Andi Hellmund
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

#include <format>
#include <string_view>
#include <vector>
#include <set>

#include "data_crunching/internal/type_conversion.hpp"
#include "data_crunching/internal/string.hpp"
#include "data_crunching/internal/utils.hpp"

#ifndef DATA_CRUNCHING_STRING_HPP
#define DATA_CRUNCHING_STRING_HPP

namespace dacr {

class StringSplitResult {
public:
    StringSplitResult (std::vector<std::string> data) : data_{std::move(data)} {}

    template <internal::IsConvertibleFromString T>
    operator std::vector<T>() const {
        std::vector<T> result;
        for (const auto& view : data_) {
            auto convert_result = internal::convertFromString<T>(std::string{view});
            if (convert_result.has_value()) {
                result.push_back(std::move(*convert_result));
            }
            else {
                throw StringException{std::format("Failed to convert element: {}", view)};
            }
        }
        return result;
    }

    template <internal::IsConvertibleFromString T>
    operator std::set<T>() const {
        std::set<T> result;
        for (const auto& view : data_) {
            auto convert_result = internal::convertFromString<T>(std::string{view});
            if (convert_result.has_value()) {
                result.insert(std::move(*convert_result));
            }
            else {
                throw StringException{std::format("Failed to convert element: {}", view)};
            }
        }
        return result;
    }

    template <typename T>
    operator T() const {
        static_assert(sizeof(T) == 0, "Invalid return type specified. Supported is only std::vector.");
    }

private:
    std::vector<std::string> data_;
};

template <typename ...Args>
auto split (const std::string& str, const std::string& delim) {
    const auto& split_str = internal::splitStringIntoVector(str, delim);
    if constexpr (sizeof...(Args) == 0) {
        return StringSplitResult{std::move(split_str)};
    }
    else {
        return internal::convertToTuple(
            split_str,
            internal::ConstructTypeListForSplitWithoutSkip<Args...>{},
            internal::GetArgumentIndicesForSplitWithoutSkip<Args...>{}
        );
    }
}

} // namespace dacr

#endif // DATA_CRUNCHING_STRING_HPP