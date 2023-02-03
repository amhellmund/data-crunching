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
    auto df = load_from_csv<DF>("simple.csv", ",", false);
    df.print();
    EXPECT_EQ(df.getSize(), 1);
}