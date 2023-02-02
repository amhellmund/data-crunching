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

#include <vector>

#include "data_crunching/string.hpp"

struct CustomData {
    std::string value;
};

auto isCustomData (const std::string& expected_value) {
    return ::testing::Field(&CustomData::value, ::testing::StrEq(expected_value));
}

TEST(String, SplitIntoVector) {
    std::vector<int> data_int = dacr::split("10,20,30,40", ",");
    EXPECT_THAT(data_int, ::testing::ElementsAre(10, 20, 30, 40));

    EXPECT_THROW([](){
        std::vector<int> data_int_failure = dacr::split("10,,30,40", ",");
    }(), dacr::StringException);

    EXPECT_THROW([](){
        std::vector<int> data_int_failure = dacr::split("10,abc,30,40", ",");
    }(), dacr::StringException);

    std::vector<CustomData> data_custom = dacr::split("abc::cde", "::");
    EXPECT_THAT(data_custom, ::testing::ElementsAre(isCustomData("abc"), isCustomData("cde")));
}

TEST(String, SplitIntoSet) {
    std::set<int> data_int = dacr::split("10,20,30,40, 10", ",");
    EXPECT_THAT(data_int, ::testing::ElementsAre(10, 20, 30, 40));

    EXPECT_THROW([](){
        std::set<int> data_int_failure = dacr::split("10,,30,40", ",");
    }(), dacr::StringException);

    EXPECT_THROW([](){
        std::set<int> data_int_failure = dacr::split("10,abc,30,40", ",");
    }(), dacr::StringException);
}

TEST(String, SplitIntoArgs) {
    auto tuple = dacr::split<int, double, std::string>("10,20.5,abc", ",");
    EXPECT_THAT(tuple, ::testing::FieldsAre(10, 20.5, std::string{"abc"}));
}

TEST(String, SplitIntoArgsWithIgnore) {
    auto tuple = dacr::split<int, dacr::Skip<2>, double, dacr::Skip<1>, std::string>("10,xxx,xxx,20.5,xxx,abc", ",");
    EXPECT_THAT(tuple, ::testing::FieldsAre(10, 20.5, std::string{"abc"}));
}