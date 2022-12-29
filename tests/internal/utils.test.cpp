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

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <type_traits>

#include "data_crunching/internal/type_list.hpp"
#include "data_crunching/internal/utils.hpp"

using dacr::internal::TuplePrepend;
using dacr::internal::TypeList;
using dacr::internal::is_convertible_to_v;

TEST(Tuple, TuplePrepend) {
    EXPECT_TRUE((std::is_same_v<
        TuplePrepend<int, std::tuple<>>,
        std::tuple<int>
    >));

    EXPECT_TRUE((std::is_same_v<
        TuplePrepend<double, std::tuple<int>>,
        std::tuple<double, int>
    >));
}

TEST(DataFrameInternal, IsConvertibleTo) {
    EXPECT_TRUE((is_convertible_to_v<TypeList<>, TypeList<>>));
    EXPECT_TRUE((is_convertible_to_v<TypeList<short>, TypeList<int>>));
    EXPECT_TRUE((is_convertible_to_v<TypeList<int, float>, TypeList<long, double>>));

    struct NonConvertibleTo{};
    EXPECT_FALSE((is_convertible_to_v<TypeList<int>, TypeList<NonConvertibleTo>>));
}