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

#include "data_crunching/internal/column.hpp"

using dacr::Column;
using dacr::internal::is_column_v;
using dacr::internal::GetColumnNames;
using dacr::internal::NameList;

TEST(Column, IsColumn) {
    EXPECT_TRUE((is_column_v<dacr::Column<"name", int>>));
}

TEST(Column, NotIsColumn) {
    EXPECT_FALSE(is_column_v<int>);
}

TEST(Column, GetColumnNamesEmpty) {
    using ColumnNames = GetColumnNames<>;
    EXPECT_TRUE((std::is_same_v<ColumnNames, NameList<>>));
}

TEST(Column, GetColumnNamesOne) {
    using ColumnNames = GetColumnNames<Column<"first", int>>;
    EXPECT_TRUE((std::is_same_v<ColumnNames, NameList<"first">>));
}

TEST(Column, GetColumnNamesTwo) {
    using ColumnNames = GetColumnNames<Column<"first", int>, Column<"second", double>>;
    EXPECT_TRUE((std::is_same_v<ColumnNames, NameList<"first", "second">>));
}