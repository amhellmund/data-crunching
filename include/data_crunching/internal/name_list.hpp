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
struct NameList {};

// ############################################################################
// TMP: Prepend To Name List
// ############################################################################
template <FixedString, typename>
struct NameListPrependImpl {};

template <FixedString NameToPrepend, FixedString ...Names>
struct NameListPrependImpl<NameToPrepend, NameList<Names...>> {
    using type = NameList<NameToPrepend, Names...>;
};

template <FixedString NameToPrepend, typename NameList>
using NameListPrepend = typename NameListPrependImpl<NameToPrepend, NameList>::type;

} // namespace dacr::internal

#endif // DATA_CRUNCHING_INTERNAL_NAME_LIST_HPP