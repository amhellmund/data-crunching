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

#ifndef DATA_CRUNCHING_INTERNAL_DATAFRDATA_CRUNCHING_INTERNAL_DATAFRAME_SUMMARIZE_HPPAME_SUMMARIZE_HPP
#define DATA_CRUNCHING_INTERNAL_DATAFRAME_SUMMARIZE_HPP

#include "data_crunching/internal/fixed_string.hpp"
#include "data_crunching/internal/utils.hpp"
#include "data_crunching/internal/name_list.hpp"
#include "data_crunching/internal/type_list.hpp"
#include "data_crunching/internal/column.hpp"

namespace dacr {

template <FixedString ...GroupByNames>
struct GroupBy {
    static constexpr std::size_t NumColumns = sizeof...(GroupByNames);
    using Names = internal::NameList<GroupByNames...>;
};

using GroupByNone = GroupBy<>;

template <FixedString ColumnName, FixedString NewColumnName>
struct Sum {};

template <FixedString ColumnName, FixedString NewColumnName>
struct Min {};

template <FixedString ColumnName, FixedString NewColumnName>
struct Max {};

template <FixedString ColumnName, FixedString NewColumnName>
struct Avg {};

template <FixedString ColumnName, FixedString NewColumnName>
struct StdDev {};

template <FixedString ColumnName, FixedString NewColumnName>
struct CountIf {};

template <FixedString ColumnName, FixedString NewColumnName>
struct CountIfNot {};

namespace internal {

// ############################################################################
// Concept: Is Group By
// ############################################################################
template <typename>
struct IsGroupBySpecImpl : std::false_type {};

template <FixedString ...GroupByNames>
struct IsGroupBySpecImpl<GroupBy<GroupByNames...>> : std::true_type {};

template <>
struct IsGroupBySpecImpl<GroupByNone> : std::true_type {};

template <typename T>
concept IsGroupBySpec = IsGroupBySpecImpl<T>::value;

// ############################################################################
// Concept: Is Summarize Op
// ############################################################################

template <typename>
struct IsSummarizeOpImpl : std::false_type {};

template <FixedString ColumnName, FixedString NewColumnName>
struct IsSummarizeOpImpl<Sum<ColumnName, NewColumnName>> : std::true_type {};

template <FixedString ColumnName, FixedString NewColumnName>
struct IsSummarizeOpImpl<Min<ColumnName, NewColumnName>> : std::true_type {};

template <FixedString ColumnName, FixedString NewColumnName>
struct IsSummarizeOpImpl<Max<ColumnName, NewColumnName>> : std::true_type {};

template <FixedString ColumnName, FixedString NewColumnName>
struct IsSummarizeOpImpl<Avg<ColumnName, NewColumnName>> : std::true_type {};

template <FixedString ColumnName, FixedString NewColumnName>
struct IsSummarizeOpImpl<StdDev<ColumnName, NewColumnName>> : std::true_type {};

template <FixedString ColumnName, FixedString NewColumnName>
struct IsSummarizeOpImpl<CountIf<ColumnName, NewColumnName>> : std::true_type {};

template <FixedString ColumnName, FixedString NewColumnName>
struct IsSummarizeOpImpl<CountIfNot<ColumnName, NewColumnName>> : std::true_type {};

template <typename T>
concept IsSummarizeOp = IsSummarizeOpImpl<T>::value;

// ############################################################################
// Concept: Trait Summarizer
// ############################################################################
template <std::size_t InIndex, IsArithmetic T>
class SummarizerSum {
public:
    using TargetType = T;

    template <typename DataIn>
    void summarize (const DataIn& in, std::size_t index) {
        sum_ += std::get<InIndex>(in)[index];
    }

    T getState () const {
        return sum_;
    }

private:
    T sum_ {0};
};

template <std::size_t InIndex, IsArithmetic T>
class SummarizerAvg {
public:   
    using TargetType = double;

    template <typename DataIn>
    void summarize (const DataIn& in, std::size_t index) {
        current_count_ += 1;
        current_average_ = current_average_ + (static_cast<double>(std::get<InIndex>(in)[index]) - current_average_) / current_count_;
    }

    double getState () const {
        return current_average_;
    }

private:
    double current_average_ {0};
    int current_count_{0};
};

template <std::size_t InIndex, IsArithmetic T>
class SummarizerStdDev {
public:
    using TargetType = double;

    template <typename DataIn>
    void summarize (const DataIn& in, std::size_t index) {
        current_count_ += 1;
        current_average_ = current_average_ + (static_cast<double>(std::get<InIndex>(in)[index]) - current_average_) / current_count_;
        values_.push_back(std::get<InIndex>(in)[index]);
    }

    double getState () const {
        if (values_.size() > 0) {
            double stddev {0};
            for (auto value : values_) {
                stddev += (value - current_average_)*(value - current_average_);
            }
            return std::sqrt(stddev / (values_.size()));
        }
        return std::numeric_limits<double>::quiet_NaN();
    }

private:
    double current_average_ {0};
    int current_count_ {0};
    std::vector<T> values_{};
};

template <std::size_t InIndex, IsArithmetic T>
class SummarizerMin {
public:
    using TargetType = T;

    template <typename DataIn>
    void summarize (const DataIn& in, std::size_t index) {
        if (std::get<InIndex>(in)[index] < min_value_) {
            min_value_ = std::get<InIndex>(in)[index];
        }
    }

    T getState() const {
        return min_value_;
    }

private:
    T min_value_ {std::numeric_limits<T>::max()};
};

template <std::size_t InIndex, IsArithmetic T>
class SummarizerMax {
public:
    using TargetType = T;

    template <typename DataIn>
    void summarize (const DataIn& in, std::size_t index) {
        if (std::get<InIndex>(in)[index] > max_value_) {
            max_value_ = std::get<InIndex>(in)[index];
        }
    }

    T getState() const {
        return max_value_;
    }

private:
    T max_value_ {std::numeric_limits<T>::lowest()};
};

template <std::size_t InIndex, typename T>
requires (std::is_same_v<T, bool>)
class SummarizerCountIf {
public:
    using TargetType = int;

    template <typename DataIn>
    void summarize (const DataIn& in, std::size_t index) {
        if (std::get<InIndex>(in)[index] == true) {
            count_++;
        }
    }

    int getState() const {
        return count_;
    }

private:
    int count_ {0};
};

template <std::size_t InIndex, typename T>
requires (std::is_same_v<T, bool>)
class SummarizerCountIfNot {
public:
    using TargetType = int;

    template <typename DataIn>
    void summarize (const DataIn& in, std::size_t index) {
        if (std::get<InIndex>(in)[index] == false) {
            count_++;
        }
    }

    int getState() const {
        return count_;
    }

private:
    int count_ {0};
};

// ############################################################################
// Trait: Is Valid Summarize Op
// ############################################################################
// ToDo: add here that summarize column names are not in group-by names
template <typename ...>
struct AreValidSummarizeOpsImpl : std::true_type {};

template <template <FixedString, FixedString> typename Op, FixedString ColumnName, FixedString NewColumnName, typename ...RestOps, typename ...Columns>
struct AreValidSummarizeOpsImpl<TypeList<Op<ColumnName, NewColumnName>, RestOps...>, Columns...> {
    static constexpr bool value = are_names_in_columns<NameList<ColumnName>, Columns...> && AreValidSummarizeOpsImpl<TypeList<RestOps...>, Columns...>::value;
};

template <typename Ops, typename ...Columns>
constexpr bool are_valid_summarize_ops = AreValidSummarizeOpsImpl<Ops, Columns...>::value;

// ############################################################################
// Trait: Compound Summarizer
// ############################################################################
template <typename ...Summarizers>
struct CompoundSummarizer {
    static constexpr std::size_t NumSummarizers = sizeof...(Summarizers);
    using Data = std::tuple<Summarizers...>;
    
    static auto create () {
        return std::make_tuple(Summarizers{}...);
    }

    template <typename SummarizerData, typename ColumnStoreData>
    static void summarize(SummarizerData& summarizer_data, const ColumnStoreData& column_store_data, std::size_t row_index) {
        summarizeImpl(summarizer_data, column_store_data, row_index, std::index_sequence_for<Summarizers...>{});
    }

    template <typename SummarizerData, typename ColumnStoreData, std::size_t ...SummarizerIndices>
    static void summarizeImpl(SummarizerData& summarizer_data, const ColumnStoreData& column_store_data, std::size_t row_index, std::integer_sequence<std::size_t, SummarizerIndices...>) {
        ((std::get<SummarizerIndices>(summarizer_data).summarize(column_store_data, row_index)), ...);
    }
};

// ############################################################################
// Trait: Prepend Compound Summarizer
// ############################################################################
template <typename, typename>
struct CompoundSummarizerPrepend {};

template <typename S, typename ...Rest>
struct CompoundSummarizerPrepend<S, CompoundSummarizer<Rest...>> {
    using type = CompoundSummarizer<S, Rest...>;
};

// ############################################################################
// Trait: Get Summarizer For Op
// ############################################################################
template <typename ...>
struct GetSummarizerForOpImpl {};

template <FixedString ColumnName, FixedString NewColumnName, typename ...Columns>
struct GetSummarizerForOpImpl<Min<ColumnName, NewColumnName>, Columns...> {
    using type = SummarizerMin<get_column_index_by_name<ColumnName, Columns...>, GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, FixedString NewColumnName, typename ...Columns>
struct GetSummarizerForOpImpl<Max<ColumnName, NewColumnName>, Columns...> {
    using type = SummarizerMax<get_column_index_by_name<ColumnName, Columns...>, GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, FixedString NewColumnName, typename ...Columns>
struct GetSummarizerForOpImpl<Sum<ColumnName, NewColumnName>, Columns...> {
    using type = SummarizerSum<get_column_index_by_name<ColumnName, Columns...>, GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, FixedString NewColumnName, typename ...Columns>
struct GetSummarizerForOpImpl<Avg<ColumnName, NewColumnName>, Columns...> {
    using type = SummarizerAvg<get_column_index_by_name<ColumnName, Columns...>, GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, FixedString NewColumnName, typename ...Columns>
struct GetSummarizerForOpImpl<StdDev<ColumnName, NewColumnName>, Columns...> {
    using type = SummarizerStdDev<get_column_index_by_name<ColumnName, Columns...>, GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, FixedString NewColumnName, typename ...Columns>
struct GetSummarizerForOpImpl<CountIf<ColumnName, NewColumnName>, Columns...> {
    using type = SummarizerCountIf<get_column_index_by_name<ColumnName, Columns...>, GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, FixedString NewColumnName, typename ...Columns>
struct GetSummarizerForOpImpl<CountIfNot<ColumnName, NewColumnName>, Columns...> {
    using type = SummarizerCountIfNot<get_column_index_by_name<ColumnName, Columns...>, GetColumnTypeByName<ColumnName, Columns...>>;
};

template <typename Op, typename ...Columns>
using GetSummarizerForOp = typename GetSummarizerForOpImpl<Op, Columns...>::type;

// ############################################################################
// Trait: GetColumnForOp
// ############################################################################
template <typename, typename ...>
struct GetColumnForOpImpl {};

template <template <FixedString, FixedString> typename Op, FixedString ColumnName, FixedString NewColumnName, typename ...Columns>
struct GetColumnForOpImpl<Op<ColumnName, NewColumnName>, Columns...> {
    using type = Column<NewColumnName, typename GetSummarizerForOp<Op<ColumnName, NewColumnName>, Columns...>::TargetType>;
};

template <typename Op, typename ...Columns>
using GetColumnForOp = typename GetColumnForOpImpl<Op, Columns...>::type;


// ############################################################################
// Trait: GetNewColumnsForOps
// ############################################################################
template <typename ...>
struct GetNewColumnsForOpsImpl {
    using type = TypeList<>;
};

template <typename FirstOp, typename ...RestOps, typename ...Columns>
struct GetNewColumnsForOpsImpl<TypeList<FirstOp, RestOps...>, Columns...> {
    using type = TypeListPrepend<
        GetColumnForOp<FirstOp, Columns...>,
        typename GetNewColumnsForOpsImpl<TypeList<RestOps...>, Columns...>::type
    >;
};

template <typename Ops, typename ...Columns>
using GetNewColumnsForOps = typename GetNewColumnsForOpsImpl<Ops, Columns...>::type;


// ############################################################################
// Trait: Get Compound Summarizer
// ############################################################################
template <typename, typename ...>
struct GetCompoundSummarizerImpl {
    using type = CompoundSummarizer<>;
};

template <typename FirstOp, typename ...RestOps, typename ...Columns>
struct GetCompoundSummarizerImpl<TypeList<FirstOp, RestOps...>, Columns...> {
    using type = typename CompoundSummarizerPrepend<
        GetSummarizerForOp<FirstOp, Columns...>,
        typename GetCompoundSummarizerImpl<TypeList<RestOps...>, Columns...>::type
    >::type;
};

template <typename Ops, typename ...Columns>
using GetCompoundSummarizer = typename GetCompoundSummarizerImpl<Ops, Columns...>::type;

// ############################################################################
// Class: SummarizationExecuter
// ############################################################################
template <typename NewDataFrame, typename CompoundSummarizer>
class SummarizationExecuterNoGroupBy {
public:
    SummarizationExecuterNoGroupBy () {
        summarizer_data_.push_back(
            CompoundSummarizer::create()
        );
    }

    template <typename ColumnStoreData>
    void summarize (const ColumnStoreData& column_store_data, std::size_t row_index) {
        CompoundSummarizer::summarize(summarizer_data_[0], column_store_data, row_index);
    }

    auto constructResult () {
        using SummarizerIndices = std::make_index_sequence<CompoundSummarizer::NumSummarizers>;
        return constructResultImpl(SummarizerIndices{});
    }

    template <std::size_t ...SummarizerIndices>
    auto constructResultImpl (std::integer_sequence<std::size_t, SummarizerIndices...>) {
        NewDataFrame result;
        for (auto loop_index = 0LU; loop_index < summarizer_data_.size(); ++loop_index) {
            result.insert(
                std::get<SummarizerIndices>(summarizer_data_[loop_index]).getState()...
            );
        }
        return result;
    }

private:
    std::vector<typename CompoundSummarizer::Data> summarizer_data_{};
};

template <typename NewDataFrame, typename GroupByColumnIndices, typename GroupByTypes, typename CompoundSummarizer>
class SummarizationExecuterGroupBy {
public:
    template <typename ColumnStoreData>
        void summarize(const ColumnStoreData& column_store_data, std::size_t row_index) {
            auto index = getGroupByIndex(column_store_data, row_index, GroupByDataIndices{}, GroupByColumnIndices{});
            if (index == -1) {
                createNewGroupByEntry(column_store_data, row_index, GroupByColumnIndices{});;
                index = group_by_data_.size() - 1;
            }
            CompoundSummarizer::summarize(summarizer_data_[index], column_store_data, row_index);
        }

        template <typename ColumnStoreData, std::size_t ... GroupByColumnIndicesInColumnStore>
        void createNewGroupByEntry (const ColumnStoreData& column_store_data, std::size_t row_index, std::integer_sequence<std::size_t, GroupByColumnIndicesInColumnStore...>) {
            group_by_data_.push_back(
                std::make_tuple(std::get<GroupByColumnIndicesInColumnStore>(column_store_data)[row_index]...)
            );
            summarizer_data_.push_back(CompoundSummarizer::create());
        }

        template <typename ColumnStoreData, std::size_t ...GroupByDataIndices, std::size_t ...GroupByColumnIndicesInColumnStore>
        int getGroupByIndex(const ColumnStoreData& column_store_data, std::size_t row_index, std::integer_sequence<std::size_t, GroupByDataIndices...>, std::integer_sequence<std::size_t, GroupByColumnIndicesInColumnStore...>) {
            for (std::size_t loop_index = 0; loop_index < group_by_data_.size(); ++loop_index) {
                if (((std::get<GroupByDataIndices>(group_by_data_[loop_index]) == std::get<GroupByColumnIndicesInColumnStore>(column_store_data)[row_index]) && ...)) {
                    return loop_index;
                }
            }
            return -1;
        }

    auto constructResult () {
        using SummarizerIndices = std::make_index_sequence<CompoundSummarizer::NumSummarizers>;
        return constructResultImpl(GroupByDataIndices{}, SummarizerIndices{});
    }

    template <std::size_t ...GroupByDataIndices, std::size_t ...SummarizerIndices>
    auto constructResultImpl (std::integer_sequence<std::size_t, GroupByDataIndices...>, std::integer_sequence<std::size_t, SummarizerIndices...>) {
        NewDataFrame result;
        for (auto loop_index = 0LU; loop_index < group_by_data_.size(); ++loop_index) {
            result.insert(
                std::get<GroupByDataIndices>(group_by_data_[loop_index])...,
                std::get<SummarizerIndices>(summarizer_data_[loop_index]).getState()...
            );
        }
        return result;
    }

private:
    using GroupByDataIndices = std::make_index_sequence<get_integer_sequence_size<GroupByColumnIndices>>;

    std::vector<GroupByTypes> group_by_data_{};;
    std::vector<typename CompoundSummarizer::Data> summarizer_data_{};
};

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_DATAFRAME_PRINT_HPP