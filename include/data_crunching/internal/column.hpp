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

#ifndef DATA_CRUNCHING_INTERNAL_COLUMN_HPP
#define DATA_CRUNCHING_INTERNAL_COLUMN_HPP

#include "data_crunching/internal/fixed_string.hpp"

namespace dacr {

template <internal::FixedString Name, typename Type>
struct Column {};

namespace internal {

// ############################################################################
// Concept: IsColumn
// ############################################################################
template <typename ...>
struct IsColumnImpl : std::true_type {};

template <typename FirstType, typename ...RestTypes>
struct IsColumnImpl<FirstType, RestTypes...> : std::false_type {};

template <FixedString FirstColName, typename FirstColType, typename ...RestColumns>
struct IsColumnImpl<Column<FirstColName, FirstColType>, RestColumns...> {
    static constexpr bool value = IsColumnImpl<RestColumns...>::value;
};

template <typename T>
constexpr bool is_column_v = IsColumnImpl<T>::value;

template <typename T>
concept IsColumn = is_column_v<T>;

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_COLUMN_HPP