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

#include "data_crunching/internal/name_list.hpp"

using dacr::internal::NameList;
using dacr::internal::NameListPrepend;
using dacr::internal::are_names_unique_v;

TEST(NameList, Size) {
    EXPECT_EQ((NameList<>::getSize()), 0);
    EXPECT_EQ((NameList<"first">::getSize()), 1);
}

TEST(NameList, AppendToEmptyNameList) {
    EXPECT_TRUE((
        std::is_same_v<
            NameListPrepend<"first", NameList<>>,
            NameList<"first">
        >
    ));

    EXPECT_TRUE((
        std::is_same_v<
            NameListPrepend<"first", NameList<"second">>,
            NameList<"first", "second">
        >
    ));
}

TEST(NameList, NamesAreUnique) {
    EXPECT_TRUE((are_names_unique_v<NameList<>>));
    EXPECT_TRUE((are_names_unique_v<NameList<"first">>));
    EXPECT_TRUE((are_names_unique_v<NameList<"first", "second">>));
    EXPECT_TRUE((are_names_unique_v<NameList<"first", "second", "third">>));
}

TEST(NameList, NamesAreUniqueFailure) {
    EXPECT_FALSE((are_names_unique_v<NameList<"first", "first">>));
    EXPECT_FALSE((are_names_unique_v<NameList<"first", "second", "first">>));
    EXPECT_FALSE((are_names_unique_v<NameList<"first", "second", "second">>));
    EXPECT_FALSE((are_names_unique_v<NameList<"first", "second", "third", "second">>));
}