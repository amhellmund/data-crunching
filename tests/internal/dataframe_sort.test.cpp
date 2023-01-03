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
#include <vector>
#include <tuple>

#include "data_crunching/internal/dataframe_sort.hpp"

using namespace dacr;
using namespace dacr::internal;

TEST(DataFrameSort, CompareElementsForSort) {
    auto column_store_data = std::make_tuple(
        std::vector<int>{10, 5, 5},
        std::vector<char>{'Z', 'B', 'E'}
    );

    using ComparisonOneColumnAsc = CompareElementsForSort<SortOrder::Ascending, 0, void>;
    EXPECT_FALSE(ComparisonOneColumnAsc::compare(column_store_data, 0, 1));
    EXPECT_FALSE(ComparisonOneColumnAsc::compare(column_store_data, 0, 2));
    EXPECT_FALSE(ComparisonOneColumnAsc::compare(column_store_data, 1, 2));

    using ComparisonOneColumnDesc = CompareElementsForSort<SortOrder::Descending, 0, void>;
    EXPECT_TRUE(ComparisonOneColumnDesc::compare(column_store_data, 0, 1));
    EXPECT_TRUE(ComparisonOneColumnDesc::compare(column_store_data, 0, 2));
    EXPECT_FALSE(ComparisonOneColumnDesc::compare(column_store_data, 1, 2));

    using ComparisonTwoColumnsAsc = CompareElementsForSort<SortOrder::Ascending, 0, 
        CompareElementsForSort<SortOrder::Ascending, 1, void>>;
    EXPECT_FALSE(ComparisonTwoColumnsAsc::compare(column_store_data, 0, 1));
    EXPECT_FALSE(ComparisonTwoColumnsAsc::compare(column_store_data, 0, 2));
    EXPECT_TRUE(ComparisonTwoColumnsAsc::compare(column_store_data, 1, 2));

    using ComparisonTwoColumnsDesc = CompareElementsForSort<SortOrder::Descending, 0, 
        CompareElementsForSort<SortOrder::Descending, 1, void>>;
    EXPECT_TRUE(ComparisonTwoColumnsDesc::compare(column_store_data, 0, 1));
    EXPECT_TRUE(ComparisonTwoColumnsDesc::compare(column_store_data, 0, 2));
    EXPECT_FALSE(ComparisonTwoColumnsDesc::compare(column_store_data, 1, 2));
}

TEST(DataFramePrint, ConstructElementComparison) {
    EXPECT_TRUE((std::is_same_v<
        ConstructElementComparison<SortOrder::Ascending, std::integer_sequence<std::size_t, 0>>,
        CompareElementsForSort<SortOrder::Ascending, 0, void>
    >));

    EXPECT_TRUE((std::is_same_v<
        ConstructElementComparison<SortOrder::Descending, std::integer_sequence<std::size_t, 0>>,
        CompareElementsForSort<SortOrder::Descending, 0, void>
    >));

    EXPECT_TRUE((std::is_same_v<
        ConstructElementComparison<SortOrder::Ascending, std::integer_sequence<std::size_t, 0, 4>>,
        CompareElementsForSort<SortOrder::Ascending, 0, 
            CompareElementsForSort<SortOrder::Ascending, 4, void>
        >
    >));

    EXPECT_TRUE((std::is_same_v<
        ConstructElementComparison<SortOrder::Descending, std::integer_sequence<std::size_t, 0, 2>>,
        CompareElementsForSort<SortOrder::Descending, 0, 
            CompareElementsForSort<SortOrder::Descending, 2, void>
        >
    >));
}
