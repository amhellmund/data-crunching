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

#ifndef DATA_CRUNCHING_INTERNAL_UTILS_HPP
#define DATA_CRUNCHING_INTERNAL_UTILS_HPP

#include <ranges>
#include <tuple>

#include "data_crunching/internal/type_list.hpp"

namespace dacr::internal {

// ############################################################################
// Trait: Tuple Prepend
// ############################################################################
template <typename ...>
struct TuplePrependImpl {};

template <typename TypeToPrepend, typename ...TupleTypes>
struct TuplePrependImpl<TypeToPrepend, std::tuple<TupleTypes...>> {
    using type = std::tuple<TypeToPrepend, TupleTypes...>;
};

template <typename TypeToPrepend, typename Tuple>
using TuplePrepend = typename TuplePrependImpl<TypeToPrepend, Tuple>::type;

// ############################################################################
// Trait: Is Convertible To Types
// ############################################################################
template <typename ...>
struct IsConvertibleToTypeImpl : std::true_type {};

template <typename FirstTypeToConvert, typename ...RestTypesToConvert, typename FirstTypeConvertedTo, typename ...RestTypesConvertedTo>
struct IsConvertibleToTypeImpl<TypeList<FirstTypeToConvert, RestTypesToConvert...>, TypeList<FirstTypeConvertedTo, RestTypesConvertedTo...>> {
    static constexpr bool value = std::is_convertible_v<FirstTypeToConvert, FirstTypeConvertedTo> && IsConvertibleToTypeImpl<TypeList<RestTypesToConvert...>, TypeList<RestTypesConvertedTo...>>::value;
};

template <typename TypesToConvertList, typename ColumnList>
constexpr bool is_convertible_to = IsConvertibleToTypeImpl<TypesToConvertList, ColumnList>::value;

// ############################################################################
// Trait: Extract Types From Range List
// ############################################################################
template <std::ranges::range ...>
struct ExtractValueTypesFromRangesImpl {
    using type = TypeList<>;
};

template <std::ranges::range FirstRange, std::ranges::range ...RestRanges>
struct ExtractValueTypesFromRangesImpl<FirstRange, RestRanges...> {
    using type = TypeListPrepend<
        std::ranges::range_value_t<FirstRange>,
        typename ExtractValueTypesFromRangesImpl<RestRanges...>::type
    >;
};

template <std::ranges::range ...Ranges>
using ExtractValueTypesFromRanges = typename ExtractValueTypesFromRangesImpl<Ranges...>::type;


// ############################################################################
// Concept: Range With Size
// ############################################################################
template <typename T>
concept IsRangeWithSize = (
    std::ranges::range<T> && requires(T &t) {
        std::ranges::size(t);
    }
);

// ############################################################################
// Util: Get Min Size From Range List
// ############################################################################
template <IsRangeWithSize ...Ranges>
inline std::size_t getMinSizeFromRanges (Ranges&& ...ranges) {
    return std::ranges::min(
        {std::ranges::size(ranges)...}
    );
}

} // namespace dacr::internal

#endif // DATA_CRUNCHING_INTERNAL_UTILS_HPP