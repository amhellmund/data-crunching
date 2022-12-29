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

using dacr::Column;
using dacr::DataFrame;

TEST(DataFrame, ValidDataFrameDefinition) {
    EXPECT_NO_THROW((DataFrame<>()));
    EXPECT_NO_THROW((DataFrame<Column<"1st", int>>()));
    EXPECT_NO_THROW((DataFrame<Column<"1st", int>, Column<"2nd", double>>()));
}

TEST(DataFrame, InsertScalarValues) {
    DataFrame<
        Column<"int", int>,
        Column<"dbl", double>
    > testdf;
    
    EXPECT_EQ(testdf.getSize(), 0);
    testdf.insert(10, 20.0);
    EXPECT_EQ(testdf.getSize(), 1);
    for (int i = 0; i < 9; ++i) {
        testdf.insert(i, static_cast<double>(i*20));
    }
    EXPECT_EQ(testdf.getSize(), 10);
}