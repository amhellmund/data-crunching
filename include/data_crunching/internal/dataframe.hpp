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

#include "data_crunching/internal/column.hpp"
#include "data_crunching/internal/name_list.hpp"
#include "data_crunching/internal/fixed_string.hpp"
#include "data_crunching/internal/type_list.hpp"
#include "data_crunching/internal/utils.hpp"

namespace dacr {

template <internal::IsColumn ...Columns>
requires internal::are_names_unique_v<internal::GetColumnNames<Columns...>>
class DataFrame;

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

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_DATAFRAME_HPP