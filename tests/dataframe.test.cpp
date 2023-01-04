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

#include <sstream>
#include <vector>
#include <ranges>

#include "data_crunching/dataframe.hpp"

using namespace dacr;

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
    testdf.insert(10, 30.0);
    EXPECT_EQ(testdf.getSize(), 1);
    testdf.insert(20, 60.0);
    EXPECT_THAT((testdf.getColumn<"int">()), ::testing::ElementsAre(10, 20));
    EXPECT_THAT((testdf.getColumn<"dbl">()), ::testing::ElementsAre(30.0, 60.0));
}

TEST(DataFrame, InsertRanges) {
    DataFrame<
        Column<"int", int>,
        Column<"dbl", double>
    > testdf;

    std::vector<int> rng_int {1, 2, 3};
    std::vector<double> rng_dbl {1.5, 2.5, 3.5};

    EXPECT_EQ(testdf.getSize(), 0);
    testdf.insertRanges(rng_int, rng_dbl);
    EXPECT_THAT((testdf.getColumn<"int">()), ::testing::ElementsAre(1, 2, 3));
    EXPECT_THAT((testdf.getColumn<"dbl">()), ::testing::ElementsAre(1.5, 2.5, 3.5));
}

TEST(DataFrame, Append) {
    DataFrame<
        Column<"int", int>,
        Column<"dbl", double>,
        Column<"chr", char>
    > testdf1, testdf2;
    
    testdf1.insert(10, 20.0, 'A');
    testdf2.insert(100, 200.0, 'B');
    testdf1.append(testdf2);
    EXPECT_EQ(testdf1.getSize(), 2);
    EXPECT_THAT(testdf1.getColumn<"int">(), ::testing::ElementsAre(10, 100));
    EXPECT_THAT(testdf1.getColumn<"dbl">(), ::testing::ElementsAre(20.0, 200.0));
    EXPECT_THAT(testdf1.getColumn<"chr">(), ::testing::ElementsAre('A', 'B'));
}

TEST(DataFrame, Select) {
    DataFrame<
        Column<"int", int>,
        Column<"dbl", double>,
        Column<"chr", char>
    > testdf;
    testdf.insert(10, 20.0, 'A');
    testdf.insert(100, 200.0, 'B');

    auto newdf = testdf.select<"int", "chr">();
    EXPECT_TRUE((std::is_same_v<
        decltype(newdf),
        DataFrame<Column<"int", int>, Column<"chr", char>>
    >));
    EXPECT_THAT(newdf.getColumn<"int">(), ::testing::ElementsAre(10, 100));
    EXPECT_THAT(newdf.getColumn<"chr">(), ::testing::ElementsAre('A', 'B'));
}

TEST(DataFrame, Apply) {
    DataFrame<
        Column<"int", int>,
        Column<"dbl", double>,
        Column<"chr", char>
    > testdf;
    testdf.insert(10, 20.0, 'A');
    testdf.insert(100, 200.0, 'B');

    auto testdfwithselect = testdf.apply<"flt", Select<"dbl">>([](dacr_param) {
        return static_cast<float>(dacr_value("dbl") * 2.0);
    });

    EXPECT_TRUE((std::is_same_v<
        decltype(testdfwithselect),
        DataFrame<Column<"dbl", double>, Column<"flt", float>>
    >));

    EXPECT_THAT(testdfwithselect.getColumn<"dbl">(), ::testing::ElementsAre(20.0, 200.0));
    EXPECT_THAT(testdfwithselect.getColumn<"flt">(), ::testing::ElementsAre(40.0, 400.0));

    auto testdfallselect = testdf.apply<"flt">([](dacr_param) {
        return static_cast<float>(dacr_value("dbl") * 2.0);
    });

    EXPECT_TRUE((std::is_same_v<
        decltype(testdfallselect),
        DataFrame<Column<"int", int>, Column<"dbl", double>, Column<"chr", char>, Column<"flt", float>>
    >));

    EXPECT_THAT(testdfallselect.getColumn<"int">(), ::testing::ElementsAre(10, 100));
    EXPECT_THAT(testdfallselect.getColumn<"dbl">(), ::testing::ElementsAre(20.0, 200.0));
    EXPECT_THAT(testdfallselect.getColumn<"chr">(), ::testing::ElementsAre('A', 'B'));
    EXPECT_THAT(testdfallselect.getColumn<"flt">(), ::testing::ElementsAre(40.0, 400.0));
}

TEST(DataFrame, Query) {
    DataFrame<
        Column<"int", int>,
        Column<"dbl", double>,
        Column<"chr", char>
    > testdf;
    testdf.insert(10, 20.0, 'A');
    testdf.insert(100, 200.0, 'B');

    auto testdfwithselect = testdf.query<Select<"dbl">>([](dacr_param) {
        return dacr_value("dbl") > 100.0;
    });
    EXPECT_TRUE((std::is_same_v<decltype(testdf), decltype(testdfwithselect)>));
    EXPECT_THAT(testdfwithselect.getColumn<"int">(), ::testing::ElementsAre(100));
    EXPECT_THAT(testdfwithselect.getColumn<"dbl">(), ::testing::ElementsAre(200.0));
    EXPECT_THAT(testdfwithselect.getColumn<"chr">(), ::testing::ElementsAre('B'));

    auto testdfallselect = testdf.query([](dacr_param) {
        return dacr_value("dbl") > 100.0;
    });
    EXPECT_TRUE((std::is_same_v<decltype(testdf), decltype(testdfallselect)>));
    EXPECT_THAT(testdfallselect.getColumn<"int">(), ::testing::ElementsAre(100));
    EXPECT_THAT(testdfallselect.getColumn<"dbl">(), ::testing::ElementsAre(200.0));
    EXPECT_THAT(testdfallselect.getColumn<"chr">(), ::testing::ElementsAre('B'));
}

TEST(DataFrame, JoinWithType) {
    DataFrame<
        Column<"id1", int>,
        Column<"id2", char>,
        Column<"dbl", double>
    > testdf1;
    testdf1.insert(10, 'A', 100.0);

    DataFrame<
        Column<"id1", int>,
        Column<"id2", char>,
        Column<"flt", float>
    > testdf2;
    testdf2.insert(10, 'A', 50.0f);

    auto dfjoined = testdf1.join<Join::Inner, "id1", "id2">(testdf2);
    
    EXPECT_TRUE((std::is_same_v<
        decltype(dfjoined),
        DataFrame<Column<"id1", int>, Column<"id2", char>, Column<"dbl", double>, Column<"flt", float>>
    >));
    EXPECT_THAT(dfjoined.getColumn<"id1">(), ::testing::ElementsAre(10));
    EXPECT_THAT(dfjoined.getColumn<"id2">(), ::testing::ElementsAre('A'));
    EXPECT_THAT(dfjoined.getColumn<"dbl">(), ::testing::ElementsAre(100.0));
    EXPECT_THAT(dfjoined.getColumn<"flt">(), ::testing::ElementsAre(50.0f));
}

std::vector<std::string> getLines (std::stringstream& sstr) {
    std::string line;
    std::vector<std::string> lines;
    while (std::getline(sstr, line)) {
        lines.push_back(line);
    }
    return lines;
}

bool contains(std::string_view str_to_search_for, std::string_view str_to_search_in) {
    return str_to_search_in.find(str_to_search_for) != str_to_search_in.npos;
}

TEST(DataFrame, SortBy) {
    DataFrame<
        Column<"a", int>,
        Column<"b", char>,
        Column<"c", double>
    > testdf;
    testdf.insert(10, 'A', 42.0);
    testdf.insert(5, 'Z', 43.0);
    testdf.insert(20, 'B', 44.0);
    testdf.insert(10, 'C', 45.0);

    auto sorted_by_single_asc = testdf.sortBy<SortOrder::Ascending, "a">();
    EXPECT_THAT(sorted_by_single_asc.getColumn<"a">(), ::testing::ElementsAre(5, 10, 10, 20));
    auto column_b = sorted_by_single_asc.getColumn<"b">();
    EXPECT_EQ(column_b[0], 'Z');
    EXPECT_EQ(column_b[3], 'B');
    auto column_c = sorted_by_single_asc.getColumn<"c">();
    EXPECT_DOUBLE_EQ(column_c[0], 43.0);
    EXPECT_DOUBLE_EQ(column_c[3], 44.0);

    auto sorted_by_single_desc = testdf.sortBy<SortOrder::Descending, "a">();
    EXPECT_THAT(sorted_by_single_desc.getColumn<"a">(), ::testing::ElementsAre(20, 10, 10, 5));
    column_b = sorted_by_single_desc.getColumn<"b">();
    EXPECT_EQ(column_b[0], 'B');
    EXPECT_EQ(column_b[3], 'Z');
    column_c = sorted_by_single_desc.getColumn<"c">();
    EXPECT_DOUBLE_EQ(column_c[0], 44.0);
    EXPECT_DOUBLE_EQ(column_c[3], 43.0);

    auto sorted_by_two_asc = testdf.sortBy<SortOrder::Ascending, "a", "b">();
    EXPECT_THAT(sorted_by_two_asc.getColumn<"a">(), ::testing::ElementsAre(5, 10, 10, 20));
    EXPECT_THAT(sorted_by_two_asc.getColumn<"b">(), ::testing::ElementsAre('Z', 'A', 'C', 'B'));
    EXPECT_THAT(sorted_by_two_asc.getColumn<"c">(), ::testing::ElementsAre(43.0, 42.0, 45.0, 44.0));
}

TEST(DataFrame, GroupBy) {
    DataFrame<
        Column<"a", int>,
        Column<"b", char>,
        Column<"c", double>,
        Column<"d", bool>
    > testdf;
    testdf.insert(10, 'A', 30.0, true);
    testdf.insert(20, 'A', 45.0, false);
    testdf.insert(20, 'A', 60.0, false);

    auto summarize_sum_no_group_by = testdf.summarize<GroupByNone, Sum<"c">>();
    auto summarize_min_group_by_single_column = testdf.summarize<GroupBy<"b">, Min<"c">>();
    auto summarize_avg_countif_group_by_two_columns = testdf.summarize<GroupBy<"a", "b">, Min<"c">, CountIf<"d">>();
}

TEST(DataFrame, Print) {
    DataFrame<
        Column<"a", int>,
        Column<"b", char>,
        Column<"c", double>,
        Column<"d", std::string>,
        Column<"e", bool>
    > testdf1;
    testdf1.insert(10, 'A', 20.0, "ABCDEFGHIHKL", true);

    std::stringstream sstr;
    testdf1.print<Select<"a", "b", "e">>(PrintOptions{}, sstr);
    const auto& lines = getLines(sstr);
    ASSERT_EQ(lines.size(), 6);

    const auto& table_header = lines[1];
    EXPECT_TRUE(contains("a", table_header));
    EXPECT_TRUE(contains("a", table_header));
    EXPECT_FALSE(contains("c", table_header));
    EXPECT_FALSE(contains("d", table_header));
    EXPECT_TRUE(contains("e", table_header));
    EXPECT_TRUE(std::ranges::all_of(
        lines | std::views::take(lines.size()-1), 
        [line_size=lines[0].size()](std::size_t width) {
            return width == line_size;
        },
        [](std::string_view str) {
            return str.size();
        }
    ));

    const auto& data_line = lines[3];
    EXPECT_TRUE(contains("10", data_line));
    EXPECT_TRUE(contains("A", data_line));
    EXPECT_TRUE(contains("true", data_line));

    const auto& row_summary_line = lines[5];
    EXPECT_TRUE(contains("Rows in DataFrame", row_summary_line));
    EXPECT_TRUE(contains("1", row_summary_line));
}