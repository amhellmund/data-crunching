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
#include <array>

#include "data_crunching/internal/type_list.hpp"
#include "data_crunching/internal/utils.hpp"

using dacr::internal::TuplePrepend;
using dacr::internal::TypeList;
using dacr::internal::is_convertible_to;
using dacr::internal::ExtractValueTypesFromRanges;
using dacr::internal::getMinSizeFromRanges;

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

TEST(TypeList, IsConvertibleTo) {
    EXPECT_TRUE((is_convertible_to<TypeList<>, TypeList<>>));
    EXPECT_TRUE((is_convertible_to<TypeList<short>, TypeList<int>>));
    EXPECT_TRUE((is_convertible_to<TypeList<int, float>, TypeList<long, double>>));

    struct NonConvertibleTo{};
    EXPECT_FALSE((is_convertible_to<TypeList<int>, TypeList<NonConvertibleTo>>));
}

TEST(Ranges, ExtractValueTypes) {
    EXPECT_TRUE((std::is_same_v<
        ExtractValueTypesFromRanges<>,
        TypeList<>
    >));

    EXPECT_TRUE((std::is_same_v<
        ExtractValueTypesFromRanges<std::array<int, 1>>,
        TypeList<int>
    >));

    EXPECT_TRUE((std::is_same_v<
        ExtractValueTypesFromRanges<std::array<int, 1>, std::array<double, 1>>,
        TypeList<int, double>
    >));
}

TEST(Ranges, GetMinSizeFromRanges) {
    std::array<int, 1> first{0};
    std::array<double, 2> second{1.0, 2.0};
    EXPECT_EQ(getMinSizeFromRanges(first, second), 1);
    std::array<float, 3> third{3.0f, 4.0f, 5.0f};
    EXPECT_EQ(getMinSizeFromRanges(third, second, first), 1);
}