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
        TypeList<Avg<"first">, Sum<"second">, Max<"first">, Min<"second">>,
        Column<"first", int>, Column<"second", double>
    >));

    EXPECT_FALSE((are_valid_summarize_ops<
        TypeList<Avg<"first">, Sum<"second">, Max<"thrid">, Min<"second">>,
        Column<"first", int>, Column<"second", double>
    >));
}

TEST(DataFrameSummarize, GetColumnForOp) {
    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<Sum<"a">, Column<"a", int>>,
        Column<"a_sum", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<Min<"a">, Column<"a", int>>,
        Column<"a_min", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<Max<"a">, Column<"a", int>>,
        Column<"a_max", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<Avg<"a">, Column<"a", int>>,
        Column<"a_avg", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<StdDev<"a">, Column<"a", int>>,
        Column<"a_stddev", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<CountIf<"a">, Column<"a", int>>,
        Column<"a_cntif", int>
    >));

    EXPECT_TRUE((std::is_same_v<
        GetColumnForOp<CountIfNot<"a">, Column<"a", int>>,
        Column<"a_cntifnot", int>
    >));
}