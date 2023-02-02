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

#include "data_crunching/internal/string.hpp"

using namespace dacr;
using namespace dacr::internal;

TEST(StringInternal, SplitStringIntoVector) {
    EXPECT_THAT(splitStringIntoVector("", ","), ::testing::ElementsAre());
    EXPECT_THAT(splitStringIntoVector("a", ","), ::testing::ElementsAre("a"));
    EXPECT_THAT(splitStringIntoVector("a,b", ","), ::testing::ElementsAre("a", "b"));
    EXPECT_THAT(splitStringIntoVector("a,b,c", ","), ::testing::ElementsAre("a", "b", "c"));
    EXPECT_THAT(splitStringIntoVector("a,,c", ","), ::testing::ElementsAre("a", "", "c"));
    EXPECT_THAT(splitStringIntoVector(",b,c", ","), ::testing::ElementsAre("", "b", "c"));
    EXPECT_THAT(splitStringIntoVector(",,c", ","), ::testing::ElementsAre("", "", "c"));
    EXPECT_THAT(splitStringIntoVector("10", ","), ::testing::ElementsAre("10"));
    EXPECT_THAT(splitStringIntoVector("10,20", ","), ::testing::ElementsAre("10", "20"));
    EXPECT_THAT(splitStringIntoVector("10,2000,30", ","), ::testing::ElementsAre("10", "2000", "30"));
    EXPECT_THAT(splitStringIntoVector("10,,30", ","), ::testing::ElementsAre("10", "", "30"));
    
    EXPECT_THAT(splitStringIntoVector("::30", "::"), ::testing::ElementsAre("", "30"));
    EXPECT_THAT(splitStringIntoVector("/-/30", "/-/"), ::testing::ElementsAre("", "30"));
}

TEST(StringInternal, ConstructTypeListForSplitWithoutSkip) {
    EXPECT_TRUE((std::is_same_v<
        ConstructTypeListForSplitWithoutSkip<int, Skip<1>, double, Skip<2>>,
        TypeList<int, double>
    >));
}

TEST(StringInternal, GetArgumentIndicesForSplitWithoutSkip) {
    EXPECT_TRUE((std::is_same_v<
        GetArgumentIndicesForSplitWithoutSkip<int, Skip<1>, double, Skip<3>, float>,
        std::integer_sequence<std::size_t, 0, 2, 6>
    >));
}

TEST(StringInternal, ConvertToTuple) {
    auto tuple = convertToTuple(
        {"10", "xxx", "xxx", "abc", "20.0", "xxx", "a"}, 
        TypeList<int, std::string, double, char>{}, 
        std::integer_sequence<std::size_t, 0, 3, 4, 6>{}
    );
    EXPECT_THAT(tuple, ::testing::FieldsAre(10, std::string{"abc"}, 20.0, 'a'));
}