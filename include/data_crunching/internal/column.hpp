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
#include "data_crunching/internal/name_list.hpp"
#include "data_crunching/internal/utils.hpp"

namespace dacr {

template <FixedString Name, typename Type>
struct Column {};

template <FixedString ...Names>
struct Select {};

struct SelectAll{};

namespace internal {

// ############################################################################
// Trait & Concept: Is Column
// ############################################################################
template <typename ...>
struct IsColumnImpl : std::true_type {};

template <typename FirstType, typename ...RestTypes>
struct IsColumnImpl<FirstType, RestTypes...> : std::false_type {};

template <FixedString FirstName, typename FirstType, typename ...RestColumns>
struct IsColumnImpl<Column<FirstName, FirstType>, RestColumns...> {
    static constexpr bool value = IsColumnImpl<RestColumns...>::value;
};

template <typename T>
constexpr bool is_column = IsColumnImpl<T>::value;

template <typename T>
concept IsColumn = is_column<T>;

// ############################################################################
// Trait: Get Column Names
// ############################################################################
template <typename ...>
struct GetColumnNamesImpl {
    using type = NameList<>;
};

template <FixedString FirstName, typename FirstType, typename ...RestColumns>
struct GetColumnNamesImpl<Column<FirstName, FirstType>, RestColumns...> {
    using type = NameListPrepend<
        FirstName,
        typename GetColumnNamesImpl<RestColumns...>::type
    >;
};

template <typename ...Columns>
using GetColumnNames = typename GetColumnNamesImpl<Columns...>::type;

// ############################################################################
// Trait: Are Names In Column List
// ############################################################################
template <FixedString, typename ...>
struct IsNameInColumnsImpl : std::false_type {};

template <FixedString NameToCheck, FixedString FirstColName, typename FirstColType, typename ...RestColumns>
struct IsNameInColumnsImpl<NameToCheck, Column<FirstColName, FirstColType>, RestColumns...> {
    static constexpr bool value = areFixedStringsEqual(NameToCheck, FirstColName) || IsNameInColumnsImpl<NameToCheck, RestColumns...>::value;
};

template <FixedString NameToCheck, typename ...Columns>
constexpr bool is_name_in_columns = IsNameInColumnsImpl<NameToCheck, Columns...>::value;

template <typename, typename ...>
struct AreNamesInColumnsImpl : std::true_type {};

template <FixedString FirstNameToCheck, FixedString ...RestNamesToCheck, typename ...Columns>
struct AreNamesInColumnsImpl<NameList<FirstNameToCheck, RestNamesToCheck...>, Columns...> {
    static constexpr bool value = is_name_in_columns<FirstNameToCheck, Columns...> && 
        AreNamesInColumnsImpl<NameList<RestNamesToCheck...>, Columns...>::value;
};

template <typename NameList, typename ...Columns>
constexpr bool are_names_in_columns = AreNamesInColumnsImpl<NameList, Columns...>::value;

// ############################################################################
// Trait: Get Column Index/Indices By Name
// ############################################################################
template <std::size_t LoopIndex, FixedString, typename ...>
struct GetColumnIndexByNameImpl {
    static constexpr std::size_t index = LoopIndex;
};

template <std::size_t LoopIndex, FixedString NameToSearch, FixedString FirstColName, typename FirstColType, typename ...RestColumns>
struct GetColumnIndexByNameImpl<LoopIndex, NameToSearch, Column<FirstColName, FirstColType>, RestColumns...>
{
    static constexpr std::size_t index = (
        areFixedStringsEqual(NameToSearch, FirstColName)
        ? LoopIndex
        : GetColumnIndexByNameImpl<LoopIndex + 1, NameToSearch, RestColumns...>::index
    );
};

template <FixedString NameToSearch, typename ...Columns>
constexpr std::size_t get_column_index_by_name = GetColumnIndexByNameImpl<0, NameToSearch, Columns...>::index;

template <typename ...>
struct GetColumnIndicesByNamesImpl {
    using type = std::integer_sequence<std::size_t>;
};

template <FixedString FirstNameToSearch, FixedString ...RestNamesToSearch, typename ...Columns>
struct GetColumnIndicesByNamesImpl<NameList<FirstNameToSearch, RestNamesToSearch...>, Columns...> {
    using type = IntegerSequencePrepend<
        get_column_index_by_name<FirstNameToSearch, Columns...>,
        typename GetColumnIndicesByNamesImpl<NameList<RestNamesToSearch...>, Columns...>::type
    >;
};

template <typename Names, typename ...Columns>
using GetColumnIndicesByNames = typename GetColumnIndicesByNamesImpl<Names, Columns...>::type;

// ############################################################################
// Trait: Get Column Type/Types By Name
// ############################################################################
template <FixedString, typename ...>
struct GetColumnTypeByNameImpl {
    using type = void;
};

template <FixedString NameToSearch, FixedString FirstColName, typename FirstColType, typename ...RestColumns>
struct GetColumnTypeByNameImpl<NameToSearch, Column<FirstColName, FirstColType>, RestColumns...>
{
    using type = std::conditional_t<
        areFixedStringsEqual(NameToSearch, FirstColName),
        FirstColType,
        typename GetColumnTypeByNameImpl<NameToSearch, RestColumns...>::type
    >;
};

template <FixedString NameToSearch, typename ...Columns>
using GetColumnTypeByName = typename GetColumnTypeByNameImpl<NameToSearch, Columns...>::type;

template <typename ...>
struct GetColumnTypesByNamesImpl {
    using type = TypeList<>;
};

template <FixedString FirstNameToSearch, FixedString ...RestNamesToSearch, typename ...Columns>
struct GetColumnTypesByNamesImpl<NameList<FirstNameToSearch, RestNamesToSearch...>, Columns...> {
    using type = TypeListPrepend<
        GetColumnTypeByName<FirstNameToSearch, Columns...>,
        typename GetColumnTypesByNamesImpl<NameList<RestNamesToSearch...>, Columns...>::type
    >;
};

template <typename Names, typename ...Columns>
using GetColumnTypesByNames = typename GetColumnTypesByNamesImpl<Names, Columns...>::type;

// ############################################################################
// Trait: Get Column By Name
// ############################################################################
template <FixedString, typename ...>
struct GetColumnByNameImpl {
    using type = void;
};

template <FixedString NameToSearch, FixedString FirstColName, typename FirstColType, typename ...RestColumns>
struct GetColumnByNameImpl<NameToSearch, Column<FirstColName, FirstColType>, RestColumns...> {
    using type = std::conditional_t<
        areFixedStringsEqual(NameToSearch, FirstColName),
        Column<FirstColName, FirstColType>,
        typename GetColumnByNameImpl<NameToSearch, RestColumns...>::type
    >;
};

template <FixedString NameToSearch, typename ...Columns>
using GetColumnByName = typename GetColumnByNameImpl<NameToSearch, Columns...>::type;

// ############################################################################
// Trait: Is Valid Select
// ############################################################################
template <typename ...>
struct IsValidSelectImpl : std::false_type {};

template <typename ...Columns>
struct IsValidSelectImpl<SelectAll, Columns...> : std::true_type {};

template <FixedString ...ColumnNames, typename ...Columns>
struct IsValidSelectImpl<Select<ColumnNames...>, Columns...> {
    static constexpr bool value = (
        sizeof...(ColumnNames) > 0 &&
        internal::are_names_unique<NameList<ColumnNames...>> &&
        internal::are_names_in_columns<NameList<ColumnNames...>, Columns...>
    );
};

template <typename SelectNames, typename ...Columns>
constexpr bool is_valid_select = IsValidSelectImpl<SelectNames, Columns...>::value;

// ############################################################################
// Trait: Get Select Names
// ############################################################################
template <typename ...>
struct GetSelectNameListImpl {};

template <typename ...Columns>
struct GetSelectNameListImpl<SelectAll, Columns...> {
    using type = GetColumnNames<Columns...>;
};

template <FixedString ...ColumnNames, typename ...Columns>
struct GetSelectNameListImpl<Select<ColumnNames...>, Columns...> {
    using type = NameList<ColumnNames...>;
};

template <typename SelectNames, typename ...Columns>
using GetSelectNameList = typename GetSelectNameListImpl<SelectNames, Columns...>::type;

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_COLUMN_HPP