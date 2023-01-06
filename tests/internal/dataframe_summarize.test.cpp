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

#include "data_crunching/internal/dataframe_summarize.hpp"

using namespace dacr;
using namespace dacr::internal;

template <typename ColumnStoreData, typename Summarizer>
void applyToSummarizer (Summarizer& summarizer, const ColumnStoreData& data) {
    for (auto loop_index = 0LU; loop_index < std::get<0>(data).size(); ++loop_index) {
        summarizer.summarize(data, loop_index);
    }
}

TEST(DataFrameSummarize, SummarizeSum) {
    auto data = std::make_tuple(std::vector<int>{10, 5, 8});
    SummarizerSum<0, int> summarizer;
    applyToSummarizer(summarizer, data);
    EXPECT_EQ(summarizer.getState(), 23);
}

TEST(DataFrameSummarize, SummarizeMin) {
    auto data = std::make_tuple(std::vector<double>{10.0, 5.0, 8.0});
    SummarizerMin<0, double> summarizer;
    applyToSummarizer(summarizer, data);
    EXPECT_DOUBLE_EQ(summarizer.getState(), 5.0);
}

TEST(DataFrameSummarize, SummarizeMax) {
    auto data = std::make_tuple(std::vector<unsigned short>{10, 5, 8});
    SummarizerMax<0, unsigned short> summarizer;
    applyToSummarizer(summarizer, data);
    EXPECT_DOUBLE_EQ(summarizer.getState(), 10);
}

TEST(DataFrameSummarize, SummarizerAvg) {
    auto data = std::make_tuple(std::vector<float>{10.0f, 5.0f, 6.0f});
    SummarizerAvg<0, float> summarizer;
    applyToSummarizer(summarizer, data);
    EXPECT_FLOAT_EQ(summarizer.getState(), 7.0f);
}

TEST(DataFrameSummarize, SummarizerStdDev) {
    auto data = std::make_tuple(std::vector<float>{10.0f, 5.0f, 6.0f});
    SummarizerStdDev<0, float> summarizer;
    applyToSummarizer(summarizer, data);
    EXPECT_FLOAT_EQ(summarizer.getState(), 2.1602468994693f);
}

TEST(DataFrameSummarize, SummarizerCountIfCountIfNot) {
    auto data = std::make_tuple(std::vector<bool>{true, false, true});
    SummarizerCountIf<0, bool> summarizer;
    applyToSummarizer(summarizer, data);
    EXPECT_EQ(summarizer.getState(), 2);
    
    SummarizerCountIfNot<0, bool> summarizer_not;
    applyToSummarizer(summarizer_not, data);
    EXPECT_EQ(summarizer_not.getState(), 1);
}

TEST(DataFrameSummarize, AreValidSummarizeOps) {
    EXPECT_TRUE((are_valid_summarize_ops<
        TypeList<Avg<"first", "first_avg">, Sum<"second", "second_sum">, Max<"first", "first_max">, Min<"second", "second_min">>,
        Column<"first", int>, Column<"second", double>
    >));

    EXPECT_FALSE((are_valid_summarize_ops<
        TypeList<Avg<"first", "first_avg">, Sum<"second", "second_sum">, Max<"thrid", "third_max">, Min<"second", "second_min">>,
        Column<"first", int>, Column<"second", double>
    >));
}

TEST(DataFrameSummarize, GetColumnForOp) {
    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<Sum<"a", "a_sum">, Column<"a", int>>,
        Column<"a_sum", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<Min<"a", "a_min">, Column<"a", int>>,
        Column<"a_min", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<Max<"a", "a_max">, Column<"a", int>>,
        Column<"a_max", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<Avg<"a", "a_avg">, Column<"a", int>>,
        Column<"a_avg", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<StdDev<"a", "a_stddev">, Column<"a", int>>,
        Column<"a_stddev", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<CountIf<"a", "a_cntif">, Column<"a", int>>,
        Column<"a_cntif", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<CountIfNot<"a", "a_cntifnot">, Column<"a", int>>,
        Column<"a_cntifnot", int>
    >));
}

TEST(DataFrameSummarize, GetNewColumnsForOps) {
    EXPECT_TRUE((std::is_same_v<
        GetNewColumnsForOps<TypeList<CountIf<"a", "a_cntif">>, Column<"a", int>>,
        TypeList<Column<"a_cntif", int>>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetNewColumnsForOps<TypeList<CountIf<"a", "a_cntif">, Max<"a", "a_max">>, Column<"a", int>>,
        TypeList<Column<"a_cntif", int>, Column<"a_max", int>>
    >));
}

TEST(DataFrameSummarizer, GetSummarizerForOp) {
    EXPECT_TRUE((std::is_same_v<
        GetSummarizerForOp<Min<"a", "a_min">, Column<"a", int>>,
        SummarizerMin<0, int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetSummarizerForOp<Max<"a", "a_max">, Column<"0", double>, Column<"a", float>>,
        SummarizerMax<1, float>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetSummarizerForOp<Sum<"a", "a_sum">, Column<"0", double>, Column<"a", short>>,
        SummarizerSum<1, short>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetSummarizerForOp<Avg<"a", "a_avg">, Column<"a", int>, Column<"0", double>>,
        SummarizerAvg<0, int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetSummarizerForOp<StdDev<"a", "a_stddev">, Column<"a", int>, Column<"0", double>>,
        SummarizerStdDev<0, int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetSummarizerForOp<CountIf<"a", "a_cntif">, Column<"a", bool>, Column<"b", float>>,
        SummarizerCountIf<0, bool>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetSummarizerForOp<CountIfNot<"a", "a_cntifnot">, Column<"0", double>, Column<"a", bool>>,
        SummarizerCountIfNot<1, bool>
    >));
}

TEST(DataFrameSummarize, GetCompoundSummarizer) {
    EXPECT_TRUE((std::is_same_v<
        GetCompoundSummarizer<
            TypeList<
                Max<"b", "b_max">,
                CountIfNot<"a", "a_cntifnot">, 
                Avg<"c", "c_avg">
            >,
            Column<"a", bool>,
            Column<"b", double>,
            Column<"c", int>
        >,
        CompoundSummarizer<
            SummarizerMax<1, double>,
            SummarizerCountIfNot<0, bool>,
            SummarizerAvg<2, int>
        >
    >));
}