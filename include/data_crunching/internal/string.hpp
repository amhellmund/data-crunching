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

#include <vector>
#include <string_view>
#include <format>
#include <exception>

#include "data_crunching/internal/type_list.hpp"
#include "data_crunching/internal/utils.hpp"
#include "data_crunching/internal/type_conversion.hpp"

#ifndef DATA_CRUNCHING_INTERNAL_STRING_HPP
#define DATA_CRUNCHING_INTERNAL_STRING_HPP

namespace dacr {

class StringException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

template <std::size_t IgnoreCount>
struct Skip {};

namespace internal {

// ############################################################################
// Trait: Is Skip
// ############################################################################
template <typename T>
struct IsSkipImpl : std::false_type {};

template <std::size_t Count>
struct IsSkipImpl<Skip<Count>> : std::true_type {
};

template <typename T>
constexpr bool is_skip = IsSkipImpl<T>::value;

// ############################################################################
// Trait: Construct Type List for Split Without Skip
// ############################################################################
template <typename ...Args>
struct ConstructTypeListForSplitWithoutSkipImpl {
    using type = TypeList<>;
};

template <typename FirstArg, typename ...Args>
struct ConstructTypeListForSplitWithoutSkipImpl<FirstArg, Args...> {
    using type = std::conditional_t<
        is_skip<FirstArg>,
        typename ConstructTypeListForSplitWithoutSkipImpl<Args...>::type,
        TypeListPrepend<FirstArg, typename ConstructTypeListForSplitWithoutSkipImpl<Args...>::type>
    >;
};

template <typename ...Args>
using ConstructTypeListForSplitWithoutSkip = typename ConstructTypeListForSplitWithoutSkipImpl<Args...>::type;

// ############################################################################
// Trait: Get Argument Indices For Split Without Skip
// ############################################################################
template <std::size_t LoopVar, typename ...Args>
struct GetArgumentIndicesForSplitWithoutSkipImpl {
    using type = std::integer_sequence<std::size_t>;
};

template <std::size_t LoopVar, typename FirstArg, typename ...Rest>
struct GetArgumentIndicesForSplitWithoutSkipImpl<LoopVar, FirstArg, Rest...> {
    using type = IntegerSequencePrepend<LoopVar, typename GetArgumentIndicesForSplitWithoutSkipImpl<LoopVar + 1, Rest...>::type>;  
};

template <std::size_t LoopVar, std::size_t SkipCount, typename ...Rest>
struct GetArgumentIndicesForSplitWithoutSkipImpl<LoopVar, Skip<SkipCount>, Rest...> {
    using type = typename GetArgumentIndicesForSplitWithoutSkipImpl<LoopVar + SkipCount, Rest...>::type;
};

template <typename ...Args>
using GetArgumentIndicesForSplitWithoutSkip = typename GetArgumentIndicesForSplitWithoutSkipImpl<0, Args...>::type;


// ############################################################################
// Utility: Split String into Vector
// ############################################################################
inline std::vector<std::string> splitStringIntoVector(const std::string& str, const std::string& delim) {
    if (str.size() == 0) {
        return {};
    }

    std::vector<std::string> result;
    std::size_t curpos = 0;
    do {
        auto nextpos = str.find(delim, curpos);
        if (nextpos != std::string::npos) {
            if (nextpos > curpos) {
                result.push_back(str.substr(curpos, nextpos - curpos));
            }
            else {
                result.push_back({});
            }
            curpos = nextpos + delim.size();
        }
        else {
            result.push_back(str.substr(curpos));
            curpos = nextpos;
        }
    } while (curpos != std::string::npos);
    return result;
}

// ############################################################################
// Utility: Convert To Tuple
// ############################################################################
template <typename Type, std::size_t Index>
auto convertElementToTargetType (const std::vector<std::string>& split_str) {
    if (Index >= split_str.size()) {
        throw StringException(std::format("Requested index {} is out of bounds (max elements = {})", Index, split_str.size()));
    }
    auto converted_data = convertFromString<Type>(split_str.at(Index));
    if (converted_data.has_value()) {
        return *converted_data;
    }
    else {
        throw StringException(std::format("Failed to convert element: {}", split_str.at(Index)));
    }
}

template <typename ...ArgTypes, std::size_t ...ArgIndices>
auto convertToTuple (const std::vector<std::string>& split_str, TypeList<ArgTypes...>, std::integer_sequence<std::size_t, ArgIndices...>) {
    return std::make_tuple(convertElementToTargetType<ArgTypes, ArgIndices>(split_str)...);
}

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_STRING_HPP