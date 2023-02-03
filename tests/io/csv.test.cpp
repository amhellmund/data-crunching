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

#include "data_crunching/dataframe.hpp"
#include "data_crunching/io/csv.hpp"

using namespace dacr;

TEST(IoCsv, LoadFromFile) {
    using DF = DataFrame<Column<"id", int>, Column<"number", double>, Column<"string", std::string>>;
    auto df = load_from_csv<DF>("tests/data/simple.csv", ",", false);
    EXPECT_EQ(df.getSize(), 1);
    EXPECT_THAT(df.getColumn<"id">(), ::testing::ElementsAre(10));
    EXPECT_THAT(df.getColumn<"number">(), ::testing::ElementsAre(::testing::DoubleEq(20.433)));
    EXPECT_THAT(df.getColumn<"string">(), ::testing::ElementsAre(::testing::StrEq("abc")));
}

TEST(IoCsv, LoadFromFileWithHeader) {
    using DF = DataFrame<Column<"id", int>, Column<"number", double>, Column<"string", std::string>>;
    auto df = load_from_csv<DF>("tests/data/simple_with_header.csv", ",", true);
    EXPECT_EQ(df.getSize(), 1);
    EXPECT_THAT(df.getColumn<"id">(), ::testing::ElementsAre(10));
    EXPECT_THAT(df.getColumn<"number">(), ::testing::ElementsAre(::testing::DoubleEq(20.433)));
    EXPECT_THAT(df.getColumn<"string">(), ::testing::ElementsAre(::testing::StrEq("abc")));
}