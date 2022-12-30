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

} // namespace dacr::internal

#endif // DATA_CRUNCHING_INTERNAL_NAME_LIST_HPP