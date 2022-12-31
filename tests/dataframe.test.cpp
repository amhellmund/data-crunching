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

#include "data_crunching/dataframe.hpp"

using dacr::Column;
using dacr::DataFrame;
using dacr::Join;
using dacr::PrintOptions;
using dacr::Select;
using dacr::SelectAll;

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

TEST(DataFrame, Print) {
    DataFrame<
        Column<"id1", int>,
        Column<"id2", char>,
        Column<"dbl", double>
    > testdf1;
    testdf1.print<Select<"id1", "dbl">>(PrintOptions{}, std::cout);
}