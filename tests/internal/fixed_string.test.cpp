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

#include "data_crunching/internal/fixed_string.hpp"

using dacr::internal::FixedString;
using dacr::internal::areFixedStringsEqual;

using ::testing::ElementsAre;

TEST(FixedString, HasCorrectLength) {
    auto str = FixedString("test");
    EXPECT_EQ(str.getLength(), 4);
}

TEST(FixedString, Append) {
    auto str = FixedString("test");
    auto append_str = str.append("_impl");
    EXPECT_EQ(append_str.getLength(), 9);
    EXPECT_THAT(append_str.data, ElementsAre('t', 'e', 's', 't', '_', 'i', 'm', 'p', 'l', '\0'));
}

TEST(FixedString, Equality) {
    auto str1 = FixedString("test");
    auto str2 = FixedString("test");
    EXPECT_TRUE(areFixedStringsEqual(str1, str2));
}

TEST(FixedString, Inequality) {
    auto str1 = FixedString("test");
    auto str2 = FixedString("test1");
    EXPECT_FALSE(areFixedStringsEqual(str1, str2));
}
