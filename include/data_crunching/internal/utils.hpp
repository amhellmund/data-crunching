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

#include <tuple>

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

} // namespace dacr::internal

#endif // DATA_CRUNCHING_INTERNAL_UTILS_HPP