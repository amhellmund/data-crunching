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

#include "data_crunching/internal/column.hpp"
#include "data_crunching/internal/dataframe.hpp"
#include "data_crunching/internal/type_list.hpp"

using dacr::Column;
using dacr::internal::ConstructColumnStoreDataType;
using dacr::internal::GetColumnTypes;
using dacr::internal::TypeList;
using dacr::internal::insertRangesIntoContainers;

template <typename T>
struct TestContainer {};

TEST(DataFrameInternal, ConstructColumnStoreDataType) {
    EXPECT_TRUE((std::is_same_v<
        ConstructColumnStoreDataType<TestContainer>,
        std::tuple<>
    >));
}

TEST(DataFrameInternal, ConstructColumnStoreDataTypeSingleColumn) {
    EXPECT_TRUE((std::is_same_v<
        ConstructColumnStoreDataType<TestContainer, Column<"int", int>>,
        std::tuple<TestContainer<int>>
    >)); 
}

TEST(DataFrameInternal, ConstructColumnStoreDataTypeTwoColumns) {
    EXPECT_TRUE((std::is_same_v<
        ConstructColumnStoreDataType<TestContainer, Column<"int", int>, Column<"dbl", double>>,
        std::tuple<TestContainer<int>, TestContainer<double>>
    >)); 
}

TEST(DataFrameInternal, GetColumnTypes) {
    EXPECT_TRUE((std::is_same_v<
        GetColumnTypes<Column<"int", int>, Column<"dbl", double>>,
        TypeList<int, double>
    >));
}

TEST(DataFrameInternal, RangeInsertionIntoContainer) {
    using ColumnStoreData = std::tuple<std::vector<int>, std::vector<double>>;
    ColumnStoreData column_store_data{};
    
    std::vector<int> input_range_int {1, 2, 3};
    std::vector<double> input_range_dbl {2.0, 3.0};

    insertRangesIntoContainers(column_store_data, std::make_index_sequence<2>{}, 2, input_range_int, input_range_dbl);
}