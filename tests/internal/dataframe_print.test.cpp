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
#include "data_crunching/internal/dataframe_print.hpp"
#include "data_crunching/internal/type_list.hpp"

using namespace dacr;
using namespace dacr::internal;

TEST(DataFramePrint, PrintExecuterMerge) {
    EXPECT_TRUE((std::is_same_v<
        PrintExecuterMerge<PrintExecuter<RowStartPrinter>, PrintExecuter<>>,
        PrintExecuter<RowStartPrinter>
    >));

    EXPECT_TRUE((std::is_same_v<
        PrintExecuterMerge<PrintExecuter<RowStartPrinter>, PrintExecuter<RowEndPrinter>>,
        PrintExecuter<RowStartPrinter, RowEndPrinter>
    >));
}

TEST(DataFramePrint, ConstructPrintExecuter) {
    EXPECT_TRUE((std::is_same_v<
        ConstructPrintExecuter<NameList<"a", "b">, TypeList<int, double>, std::integer_sequence<std::size_t, 0, 1>>,
        PrintExecuter<
            RowStartPrinter,
            ColumnPrinter<"a", int, 0>,
            ColumnSeparatorPrinter,
            ColumnPrinter<"b", double, 1>,
            RowEndPrinter
        >
    >));
}

TEST(DataFramePrint, ColumnPrinterHeader) {
    using StringPrinter = ColumnPrinter<"abcdef", std::string, 0>;
    std::stringstream sstr;
    StringPrinter::printHeader(sstr, {.string_width = 3});
    EXPECT_EQ(sstr.str(), "abc");

    sstr.str(std::string());
    StringPrinter::printHeader(sstr, {.string_width = 4});
    EXPECT_EQ(sstr.str(), "ab..");

    sstr.str(std::string());
    StringPrinter::printHeader(sstr, {.string_width = 6});
    EXPECT_EQ(sstr.str(), "abcdef");
}

struct CustomData {
    int i{0};
};

std::ostream& operator<< (std::ostream& stream, const CustomData& data) {
    stream << "i: " << data.i;
    return stream;
}

TEST(DataFramePrint, ColumnPrinterDataCustom) {
    using CustomPrinter = ColumnPrinter<"custom", CustomData, 0>;
    auto column_store_data = std::make_tuple(std::vector<CustomData>{{}});

    std::stringstream sstr;
    CustomPrinter::printData(sstr, {.custom_width = 4}, column_store_data, 0);
    EXPECT_EQ(sstr.str(), "i: 0");
}