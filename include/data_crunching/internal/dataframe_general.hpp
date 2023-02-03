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

#ifndef DATA_CRUNCHING_INTERNAL_DATAFRAME_HPP
#define DATA_CRUNCHING_INTERNAL_DATAFRAME_HPP

#include <tuple>

#include "data_crunching/namedtuple.hpp"
#include "data_crunching/internal/column.hpp"
#include "data_crunching/internal/name_list.hpp"
#include "data_crunching/internal/fixed_string.hpp"
#include "data_crunching/internal/type_list.hpp"
#include "data_crunching/internal/utils.hpp"

namespace dacr {

template <internal::IsColumn ...Columns>
requires (
    internal::are_names_unique<internal::GetColumnNames<Columns...>> and
    internal::are_names_valid_identifiers<internal::GetColumnNames<Columns...>>
)
class DataFrame;

enum class Join : int {
    Inner = 0,
};

namespace internal {

// ############################################################################
// Trait: Column Store Data Type
// ############################################################################
template <template <typename, typename ...> typename, typename ...T>
struct ConstructColumnStoreDataTypeImpl {
    using type = std::tuple<>;
};

template <template <typename, typename ...> typename Container, FixedString FirstColName, typename FirstColType, typename ...RestColumns>
struct ConstructColumnStoreDataTypeImpl<Container, Column<FirstColName, FirstColType>, RestColumns...> {
    using type = TuplePrepend<
        Container<FirstColType>,
        typename ConstructColumnStoreDataTypeImpl<Container, RestColumns...>::type
    >;
};

template <template <typename, typename ...> typename Container, typename ...Columns>
using ConstructColumnStoreDataType = typename ConstructColumnStoreDataTypeImpl<Container, Columns...>::type;

// ############################################################################
// Trait: Get Column Types
// ############################################################################
template <typename ...>
struct GetColumnTypesImpl {
    using type = TypeList<>;
};

template <FixedString FirstColName, typename FirstColType, typename ...RestColumns>
struct GetColumnTypesImpl<Column<FirstColName, FirstColType>, RestColumns...> {
    using type = TypeListPrepend<
        FirstColType,
        typename GetColumnTypesImpl<RestColumns...>::type
    >;
};

template <typename ...Columns>
using GetColumnTypes = typename GetColumnTypesImpl<Columns...>::type;

// ############################################################################
// Range Insertion
// ############################################################################
template <typename Container, internal::IsRangeWithSize Range>
requires (
    requires(Container& c) {
        c.push_back(std::declval<std::ranges::range_value_t<Range>>());
    }
)
inline void insertRangeIntoContainerImpl (Container& container, std::size_t element_count, Range&& range) {
    auto iterator = std::forward<Range>(range).begin();
    for (auto i = 0L; i < element_count; ++i) {
        container.push_back(*iterator);
        ++iterator;
    }
}

template <typename ColumnStoreData, std::size_t ...Indices, internal::IsRangeWithSize ...Ranges>
inline void insertRangesIntoContainers (ColumnStoreData& column_store_data, std::integer_sequence<std::size_t, Indices...>, std::size_t element_count, Ranges&& ...ranges) {
    ((insertRangeIntoContainerImpl(std::get<Indices>(column_store_data), element_count, std::forward<Ranges>(ranges))), ...);
}

// ############################################################################
// Trait: Prepend DataFrame
// ############################################################################
template <typename ...>
struct DataFramePrependImpl {};

template <typename TypeToPrepend, typename ...TypesInDataFrame>
struct DataFramePrependImpl<TypeToPrepend, DataFrame<TypesInDataFrame...>> {
    using type = DataFrame<TypeToPrepend, TypesInDataFrame...>;
};

template <typename TypeToPrepend, typename DataFrameType>
using DataFramePrepend = typename DataFramePrependImpl<TypeToPrepend, DataFrameType>::type;

// ############################################################################
// Trait: Merge DataFrame
// ############################################################################
template <typename ...>
struct DataFrameMergeImpl {};

template <typename ...TypesInDataFrame1, typename ...TypesInDataFrame2>
struct DataFrameMergeImpl<DataFrame<TypesInDataFrame1...>, DataFrame<TypesInDataFrame2...>> {
    using type = DataFrame<TypesInDataFrame1..., TypesInDataFrame2...>;
};

template <typename DataFrame1, typename DataFrame2>
using DataFrameMerge = typename DataFrameMergeImpl<DataFrame1, DataFrame2>::type;

// ############################################################################
// Trait: Append DataFrame
// ############################################################################
template <typename ...>
struct DataFrameAppendImpl {};

template <typename TypeToAppend, typename ...TypesInDataFrame>
struct DataFrameAppendImpl<TypeToAppend, DataFrame<TypesInDataFrame...>> {
    using type = DataFrame<TypesInDataFrame..., TypeToAppend>;
};

template <typename TypeToAppend, typename DataFrameType>
using DataFrameAppend = typename DataFrameAppendImpl<TypeToAppend, DataFrameType>::type; 

// ############################################################################
// Trait: Get DataFrame With Columns By Names
// ############################################################################
template <typename, typename ...>
struct GetDataFrameWithColumnsByNameImpl {
    using type = DataFrame<>;
};

template <FixedString FirstNameToSelect, FixedString ...RestNamesToSelect, typename ...Columns>
struct GetDataFrameWithColumnsByNameImpl<NameList<FirstNameToSelect, RestNamesToSelect...>, Columns...> {
    using type = DataFramePrepend<
        GetColumnByName<FirstNameToSelect, Columns...>,
        typename GetDataFrameWithColumnsByNameImpl<NameList<RestNamesToSelect...>, Columns...>::type
    >;
};

template <typename Names, typename ...Columns>
using GetDataFrameWithColumnsByName = typename GetDataFrameWithColumnsByNameImpl<Names, Columns...>::type;

// ############################################################################
// Trait: Construct Named Tuple
// ############################################################################
template<typename, typename>
struct ConstructNamedTupleImpl {};

template <FixedString ...Names, typename ...Types>
struct ConstructNamedTupleImpl<NameList<Names...>, TypeList<Types...>> {
    using type = NamedTuple<Field<Names,Types>...>;
};

template <typename Names, typename Types>
using ConstructNamedTuple = typename ConstructNamedTupleImpl<Names, Types>::type;

// ############################################################################
// Trait: Construct DataFrame For Apply
// ############################################################################
template <typename, typename ...>
struct ConstructDataFrameForApplyImpl {
    using type = DataFrame<>;
};

template <FixedString FirstNameToSearch, FixedString ...RestNamesToSearch, typename ...Columns>
struct ConstructDataFrameForApplyImpl<NameList<FirstNameToSearch, RestNamesToSearch...>, Columns...> {
    using type = DataFramePrepend<
        GetColumnByName<FirstNameToSearch, Columns...>,
        typename ConstructDataFrameForApplyImpl<NameList<RestNamesToSearch...>, Columns...>::type
    >;
};

template <typename SelectNames, FixedString NewColName, typename NewColType, typename ...Columns>
using ConstructDataFrameForApply = DataFrameAppend<
    Column<NewColName, NewColType>,
    typename ConstructDataFrameForApplyImpl<SelectNames, Columns...>::type
>; 

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_DATAFRAME_HPP