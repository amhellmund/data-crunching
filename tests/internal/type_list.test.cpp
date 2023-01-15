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

using dacr::TypeList;

using namespace dacr::internal;

TEST(TypeList, Size) {
    EXPECT_EQ((TypeList<>::getSize()), 0);
    EXPECT_EQ((TypeList<int>::getSize()), 1);
}

TEST(TypeList, Prepend) {
    EXPECT_TRUE((std::is_same_v<TypeListPrepend<int, TypeList<>>, TypeList<int>>));
    EXPECT_TRUE((std::is_same_v<TypeListPrepend<double, TypeList<int>>, TypeList<double, int>>));
}

TEST(TypeList, ConvertTypeListToTuple) {
    EXPECT_TRUE((std::is_same_v<
        ConvertTypeListToTuple<TypeList<>>, 
        std::tuple<>
    >));

    EXPECT_TRUE((std::is_same_v<
        ConvertTypeListToTuple<TypeList<int>>, 
        std::tuple<int>
    >));

    EXPECT_TRUE((std::is_same_v<
        ConvertTypeListToTuple<TypeList<int, double>>, 
        std::tuple<int, double>
    >));
}
