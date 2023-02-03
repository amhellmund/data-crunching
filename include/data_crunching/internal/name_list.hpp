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

#ifndef DATA_CRUNCHING_INTERNAL_NAME_LIST_HPP
#define DATA_CRUNCHING_INTERNAL_NAME_LIST_HPP

#include "data_crunching/internal/fixed_string.hpp"

namespace dacr::internal {

template <FixedString ...Names>
struct NameList {
    static constexpr std::size_t getSize() {
        return sizeof...(Names);
    }
};

// ############################################################################
// Trait: Prepend To Name List
// ############################################################################
template <FixedString, typename>
struct NameListPrependImpl {};

template <FixedString NameToPrepend, FixedString ...Names>
struct NameListPrependImpl<NameToPrepend, NameList<Names...>> {
    using type = NameList<NameToPrepend, Names...>;
};

template <FixedString NameToPrepend, typename NameList>
using NameListPrepend = typename NameListPrependImpl<NameToPrepend, NameList>::type;

// ############################################################################
// Trait: Merge Name Lists
// ############################################################################
template <typename, typename>
struct NameListMergeImpl {};

template <FixedString ...Names1, FixedString ...Names2>
struct NameListMergeImpl<NameList<Names1...>, NameList<Names2...>> {
    using type = NameList<Names1..., Names2...>;
};

template <typename NameList1, typename NameList2>
using NameListMerge = typename NameListMergeImpl<NameList1, NameList2>::type;

// ############################################################################
// Trait: Are Names Unique
// ############################################################################
template <FixedString, FixedString ...>
struct AreNamesUniqueInnerIterator : std::true_type {};

template <FixedString NameToCheck, FixedString FirstName, FixedString ...RestNames>
struct AreNamesUniqueInnerIterator<NameToCheck, FirstName, RestNames...> {
    static constexpr bool value = (
        (not areFixedStringsEqual(NameToCheck, FirstName)) && 
        AreNamesUniqueInnerIterator<NameToCheck, RestNames...>::value
    );
};

template <typename ...T>
struct AreNamesUniqueImpl : std::true_type{};

template <FixedString FirstName, FixedString ...RestNames>
struct AreNamesUniqueImpl<NameList<FirstName, RestNames...>> {
    static constexpr bool value = (
        AreNamesUniqueInnerIterator<FirstName, RestNames...>::value && 
        AreNamesUniqueImpl<NameList<RestNames...>>::value
    );
};

template <typename NameList>
inline constexpr bool are_names_unique = (NameList::getSize() <= 1 || AreNamesUniqueImpl<NameList>::value);


// ############################################################################
// Trait: Are Names Valid Identifiers
// ############################################################################

template <std::size_t N, std::size_t ...Indices>
constexpr bool isNameValidIdentifier (FixedString<N> name, std::integer_sequence<std::size_t, Indices...>) {
    return (N > 0) and (
        (
            (name.data[Indices] >= 'a' and name.data[Indices] <= 'z') or 
            (name.data[Indices] >= 'A' and name.data[Indices] <= 'Z') or 
            (name.data[Indices] >= '0' and name.data[Indices] <= '9') or
            (name.data[Indices] == '_') or
            (name.data[Indices] == '-')
        ) and ...
    );
}

template <std::size_t N>
constexpr bool isNameValidIdentifier (FixedString<N> name) {
    return isNameValidIdentifier(name, std::make_index_sequence<N-1>{});
}

template <typename>
struct AreNamesValidIdentifiersImpl : std::true_type {};

template <FixedString FirstName, FixedString ...RestNames>
struct AreNamesValidIdentifiersImpl<NameList<FirstName, RestNames...>> {
    static constexpr bool value = (
        isNameValidIdentifier(FirstName) && AreNamesValidIdentifiersImpl<NameList<RestNames...>>::value
    );
};

template <typename NameList>
constexpr bool are_names_valid_identifiers = AreNamesValidIdentifiersImpl<NameList>::value;


// ############################################################################
// Trait: Name List Difference
// ############################################################################
template <FixedString, typename>
struct IsNameInNameListImpl : std::false_type {};

template <FixedString NameToSearch, FixedString FirstNameInList, FixedString ...RestNamesInList>
struct IsNameInNameListImpl<NameToSearch, NameList<FirstNameInList, RestNamesInList...>> {
    static constexpr bool value = areFixedStringsEqual(NameToSearch, FirstNameInList) || IsNameInNameListImpl<NameToSearch, NameList<RestNamesInList...>>::value;
};

template <FixedString NameToSearch, typename NameListToSearch>
constexpr bool is_name_in_name_list = IsNameInNameListImpl<NameToSearch, NameListToSearch>::value;

// ############################################################################
// Trait: Name List Difference
// ############################################################################
template <typename, typename>
struct NameListDifferenceImpl {
    using type = NameList<>;
};

template <FixedString FirstNameToRemoveFrom, FixedString ...RestNamesToRemoveFrom, typename NamesToRemove>
struct NameListDifferenceImpl<NameList<FirstNameToRemoveFrom, RestNamesToRemoveFrom...>, NamesToRemove> {
    using type = std::conditional_t<
        is_name_in_name_list<FirstNameToRemoveFrom, NamesToRemove>,
        typename NameListDifferenceImpl<NameList<RestNamesToRemoveFrom...>, NamesToRemove>::type,
        NameListPrepend<FirstNameToRemoveFrom, typename NameListDifferenceImpl<NameList<RestNamesToRemoveFrom...>, NamesToRemove>::type>
    >;
};

template <typename NameListToRemoveFrom, typename NamesToRemove>
using NameListDifference = typename NameListDifferenceImpl<NameListToRemoveFrom, NamesToRemove>::type;

} // namespace dacr::internal

#endif // DATA_CRUNCHING_INTERNAL_NAME_LIST_HPP