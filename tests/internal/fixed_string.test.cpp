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

using dacr::FixedString;
using dacr::internal::areFixedStringsEqual;

using ::testing::ElementsAre;

TEST(FixedString, HasCorrectLength) {
    EXPECT_EQ(FixedString("test").getLength(), 4);
}

TEST(FixedString, Append) {
    auto str = FixedString("test").append("_impl");
    EXPECT_EQ(str.getLength(), 9);
    EXPECT_THAT(str.data, ElementsAre('t', 'e', 's', 't', '_', 'i', 'm', 'p', 'l', '\0'));
}

TEST(FixedString, Equality) {
    EXPECT_TRUE(areFixedStringsEqual(FixedString(""), FixedString("")));
    EXPECT_TRUE(areFixedStringsEqual(FixedString("test"), FixedString("test")));
}

TEST(FixedString, Inequality) {
    EXPECT_FALSE(areFixedStringsEqual(FixedString("test"), FixedString("test1")));
    EXPECT_FALSE(areFixedStringsEqual(FixedString("test"), FixedString("")));
}
