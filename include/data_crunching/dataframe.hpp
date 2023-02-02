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

#ifndef DATA_CRUNCHING_DATAFRAME_HPP
#define DATA_CRUNCHING_DATAFRAME_HPP

#include <ranges>
#include <vector>
#include <iostream>

#include "data_crunching/internal/dataframe_general.hpp"
#include "data_crunching/internal/dataframe_print.hpp"
#include "data_crunching/internal/dataframe_sort.hpp"
#include "data_crunching/internal/dataframe_summarize.hpp"
#include "data_crunching/internal/column.hpp"
#include "data_crunching/internal/name_list.hpp"
#include "data_crunching/internal/type_list.hpp"

namespace dacr {

template <internal::IsColumn ...Columns>
requires internal::are_names_unique<internal::GetColumnNames<Columns...>>
// ToDO: column names must not be empty
class DataFrame {
public:
    template<internal::IsColumn ...OtherColumns>
    requires internal::are_names_unique<internal::GetColumnNames<OtherColumns...>>
    friend class DataFrame;

    DataFrame() = default;

    // ############################################################################
    // API: Get Size
    // ############################################################################
    std::size_t getSize() const {
        if constexpr (sizeof...(Columns) > 0) {
            return std::get<0>(column_store_data_).size();
        }
        else {
            return 0;
        } 
    }

    // ############################################################################
    // API: Scalar Insertion
    // ############################################################################
    template <typename ...TypesToInsert>
    requires (
        sizeof...(TypesToInsert) == sizeof...(Columns) &&
        internal::is_convertible_to<TypeList<TypesToInsert...>, internal::GetColumnTypes<Columns...>>
    )
    void insert (TypesToInsert&& ...values) {
        // this step is necessary to guarantee exception-saftey to at least keep the state of the column store
        // as before the call
        assureSufficientCapacityInColumnStore(1, IndicesForColumnStore{});
        insertImpl<TypesToInsert...>(std::forward<TypesToInsert>(values)..., IndicesForColumnStore{});
    }

    // ############################################################################
    // API: Range Insertion
    // ############################################################################
    template <internal::IsRangeWithSize ...Ranges>
    requires (
        sizeof...(Ranges) == sizeof...(Columns) && 
        internal::is_convertible_to<internal::ExtractValueTypesFromRanges<Ranges...>, internal::GetColumnTypes<Columns...>>
    )
    std::size_t insertRanges (Ranges&& ...ranges) {
        const std::size_t min_size = internal::getMinSizeFromRanges(std::forward<Ranges>(ranges)...);
        assureSufficientCapacityInColumnStore(min_size, IndicesForColumnStore{});
        internal::insertRangesIntoContainers(column_store_data_, IndicesForColumnStore{}, min_size, std::forward<Ranges>(ranges)...);
        return min_size;
    }

    // ############################################################################
    // API: Append
    // ############################################################################
    void append (const DataFrame& df) {
        assureSufficientCapacityInColumnStore(df.getSize(), IndicesForColumnStore{});
        appendImpl(df, IndicesForColumnStore{});
    }

    // ############################################################################
    // API: Column (Read) Access
    // ############################################################################
    template <FixedString ColumnName>
    requires (internal::is_name_in_columns<ColumnName, Columns...>)
    const auto& getColumn () const {
        constexpr auto index = internal::get_column_index_by_name<ColumnName, Columns...>;
        return std::get<index>(column_store_data_);
    }

    // ############################################################################
    // API: Select
    // ############################################################################
    template <FixedString ...ColumnNames>
    requires (
        sizeof...(ColumnNames) > 0 && 
        internal::are_names_unique<internal::NameList<ColumnNames...>> &&
        internal::are_names_in_columns<internal::NameList<ColumnNames...>, Columns...>
    )
    auto select () {
        if constexpr (sizeof...(Columns) > 0) {
            using NewDataFrame = internal::GetDataFrameWithColumnsByName<internal::NameList<ColumnNames...>, Columns...>;
            using SelectedColumnIndices = internal::GetColumnIndicesByNames<internal::NameList<ColumnNames...>, Columns...>;
            return selectImpl<NewDataFrame>(SelectedColumnIndices{});
        }
        else {
            return DataFrame{};
        }
    }

    // ############################################################################
    // API: Apply
    // ############################################################################
    template <FixedString NewColumnName, typename SelectNames = SelectAll, typename Func>
    requires (
        NewColumnName.getLength() > 0 &&
        internal::is_valid_select<SelectNames, Columns...>
    )
    auto apply (Func&& function) {
        if constexpr (sizeof...(Columns) > 0) {
            using SelectedNamesForApply = internal::GetSelectNameList<SelectNames, Columns...>;
            using SelectedColumnIndices = internal::GetColumnIndicesByNames<SelectedNamesForApply, Columns...>;
            using SelectedTypesForFunc = internal::GetColumnTypesByNames<SelectedNamesForApply, Columns...>;
            using NamedTupleForFuncArgs = internal::ConstructNamedTuple<SelectedNamesForApply, SelectedTypesForFunc>;
            using FuncReturnType = std::invoke_result_t<Func, NamedTupleForFuncArgs>;
            using NewDataFrame = internal::ConstructDataFrameForApply<SelectedNamesForApply, NewColumnName, FuncReturnType, Columns...>;
            
            return applyImpl<NewDataFrame, NamedTupleForFuncArgs, SelectedNamesForApply::getSize()>(
                std::forward<Func>(function),
                SelectedColumnIndices{}
            );
        } 
        else {
            return DataFrame{};
        }
    }

    // ############################################################################
    // API: Query
    // ############################################################################
    template <typename SelectNames = SelectAll, typename Func>
    requires (internal::is_valid_select<SelectNames, Columns...>)
    auto query (Func&& function) {
        if constexpr (sizeof...(Columns) > 0) {
            using SelectedNamesForApply = internal::GetSelectNameList<SelectNames, Columns...>;
            using SelectedColumnIndices = internal::GetColumnIndicesByNames<SelectedNamesForApply, Columns...>;
            using SelectedTypesForFunc = internal::GetColumnTypesByNames<SelectedNamesForApply, Columns...>;
            using NamedTupleForFuncArgs = internal::ConstructNamedTuple<SelectedNamesForApply, SelectedTypesForFunc>;
            using FuncReturnType = std::invoke_result_t<Func, NamedTupleForFuncArgs>;
            static_assert(std::is_same_v<FuncReturnType, bool>, "Callback return type for query() must be bool");

            return queryImpl<NamedTupleForFuncArgs>(std::forward<Func>(function), SelectedColumnIndices{}, IndicesForColumnStore{});
        }
        else {
            return DataFrame{};
        }
    }

    // ############################################################################
    // API: Join
    // ############################################################################
    template <Join JoinType, FixedString ...JoinNames, typename ...OtherColumns>
    requires (sizeof...(JoinNames) > 0 &&
        internal::are_names_unique<internal::NameList<JoinNames...>> &&
        internal::are_names_in_columns<internal::NameList<JoinNames...>, Columns...> &&
        internal::are_names_in_columns<internal::NameList<JoinNames...>, Columns...> &&
        internal::are_names_unique<
            internal::NameListMerge<
                internal::NameListDifference<internal::GetColumnNames<Columns...>, internal::NameList<JoinNames...>>,
                internal::NameListDifference<internal::GetColumnNames<OtherColumns...>, internal::NameList<JoinNames...>>
            >
        > &&
        internal::are_columns_the_same_in_two_lists_by_names<internal::NameList<JoinNames...>, TypeList<Columns...>, TypeList<OtherColumns...>>
    )
    auto join (const DataFrame<OtherColumns...>& df) {
        if constexpr (sizeof...(Columns) > 0) {
            // compute all the indices
            using JoinIndicesSelf = internal::GetColumnIndicesByNames<internal::NameList<JoinNames...>, Columns...>;
            using JoinIndicesOther = internal::GetColumnIndicesByNames<internal::NameList<JoinNames...>, OtherColumns...>;
            using ColumnNamesToCopyOther = internal::NameListDifference<internal::GetColumnNames<OtherColumns...>, internal::NameList<JoinNames...>>;
            using DataIndicesToCopyOther = internal::GetColumnIndicesByNames<ColumnNamesToCopyOther, OtherColumns...>;

            using DataIndicesInResultOther = internal::IntegerSequenceByRange<sizeof...(Columns), sizeof...(Columns) + sizeof...(OtherColumns) - sizeof...(JoinNames)>;
            using JoinedDataFrame = internal::DataFrameMerge<DataFrame, internal::GetDataFrameWithColumnsByName<ColumnNamesToCopyOther, OtherColumns...>>;
            
            return joinImpl<JoinType, JoinedDataFrame, JoinIndicesSelf, JoinIndicesOther, DataIndicesInResultOther, DataIndicesToCopyOther>(df);
        }
        else {
            return DataFrame{};
        }
    }

    // ############################################################################
    // API: Summarize
    // ############################################################################
    template <internal::IsGroupBySpec GroupBy, internal::IsSummarizeOp ...Ops>
    requires (internal::are_valid_summarize_ops<TypeList<Ops...>, Columns...>)
    // ToDo group-by names are not part of summarize-ops
    auto summarize () {
        if constexpr (sizeof...(Columns) > 0) {
            if constexpr (GroupBy::NumColumns == 0) {
                using CompoundSummarizer = internal::GetCompoundSummarizer<TypeList<Ops...>, Columns...>;
                using NewDataFrame = typename internal::GetNewColumnsForOps<TypeList<Ops...>, Columns...>::template To<DataFrame>;
                
                using Executer = internal::SummarizationExecuterNoGroupBy<NewDataFrame, CompoundSummarizer>;
                Executer executer{};
                for (std::size_t loop_index = 0; loop_index < getSize(); ++loop_index) {
                    executer.summarize(column_store_data_, loop_index);
                }
                return executer.constructResult();
            }
            else {
                using GroupByIndices = internal::GetColumnIndicesByNames<typename GroupBy::Names, Columns...>;
                using GroupByTypes = typename internal::GetColumnTypesByNames<typename GroupBy::Names, Columns...>::template To<std::tuple>;

                using CompoundSummarizer = internal::GetCompoundSummarizer<TypeList<Ops...>, Columns...>;
                using DataFrameGroupBy = internal::GetDataFrameWithColumnsByName<typename GroupBy::Names, Columns...>;
                using DataFrameOps = typename internal::GetNewColumnsForOps<TypeList<Ops...>, Columns...>::template To<DataFrame>;
                using NewDataFrame = internal::DataFrameMerge<DataFrameGroupBy, DataFrameOps>;
                
                using Executer = internal::SummarizationExecuterGroupBy<NewDataFrame, GroupByIndices, GroupByTypes, CompoundSummarizer>;
                Executer executer{};
                for (std::size_t loop_index = 0; loop_index < getSize(); ++loop_index) {
                    executer.summarize(column_store_data_, loop_index);
                }
                return executer.constructResult();
            }
        }
        else {
            return DataFrame{};
        }
    }

    // ############################################################################
    // API: Sort
    // ############################################################################
    template <SortOrder Order, FixedString ...SortByNames>
    requires (
        sizeof...(SortByNames) > 0 &&
        internal::are_names_unique<internal::NameList<SortByNames...>> &&
        internal::are_names_in_columns<internal::NameList<SortByNames...>, Columns...>
    )
    auto sort () {
        if constexpr (sizeof...(Columns) > 0) {
            using ColumnIndices = internal::GetColumnIndicesByNames<internal::NameList<SortByNames...>, Columns...>;
            using ElementComparison = internal::ConstructElementComparison<Order, ColumnIndices>;
            using ColumnStoreElementComparison = internal::ColumnStoreRowComparisonProxy<ColumnStoreDataType, ElementComparison>;

            std::vector<ColumnStoreElementComparison> row_comparison_proxy {};
            row_comparison_proxy.reserve(getSize());
            for (std::size_t i = 0; i < getSize(); ++i) {
                row_comparison_proxy.push_back({column_store_data_, i});
            }
            std::sort(row_comparison_proxy.begin(), row_comparison_proxy.end());

            DataFrame result{};
            result.assureSufficientCapacityInColumnStore(getSize(), IndicesForColumnStore{});
            for (const auto& order : row_comparison_proxy) {
                sortByImpl(result, order.getIndex(), IndicesForColumnStore{});
            }
            return result;
        }
        else {
            return DataFrame{};
        }
    }

    // ############################################################################
    // API: Print
    // ############################################################################
    template <typename SelectNames = SelectAll>
    requires (internal::is_valid_select<SelectNames, Columns...>)
    void print (const PrintOptions& print_options = PrintOptions{}, std::ostream& stream = std::cout) {
        if constexpr (sizeof...(Columns) > 0) { 
            using SelectedNamesForApply = internal::GetSelectNameList<SelectNames, Columns...>;
            using SelectedColumnTypes = internal::GetColumnTypesByNames<SelectedNamesForApply, Columns...>;
            using SelectedColumnIndices = internal::GetColumnIndicesByNames<SelectedNamesForApply, Columns...>;

            using PrintExecuter = internal::ConstructPrintExecuter<SelectedNamesForApply, SelectedColumnTypes, SelectedColumnIndices>;
            
            PrintExecuter printer{stream, print_options};
            printer.print(column_store_data_);
        }
        else {
            stream << "No columns in DataFrame available\n";
        }
    }

private:
    using ColumnStoreDataType = internal::ConstructColumnStoreDataType<std::vector, Columns...>;
    using IndicesForColumnStore = std::index_sequence_for<Columns...>;

    template <std::size_t ...Indices>
    void assureSufficientCapacityInColumnStore (std::size_t amount_to_insert, std::integer_sequence<std::size_t, Indices...>) {
        if constexpr (sizeof...(Columns) > 0) {
            const auto& first_column_data = std::get<0>(column_store_data_);
            if (first_column_data.size() + amount_to_insert >= first_column_data.capacity()) {
                const std::size_t new_capacity = first_column_data.capacity() * 1.50;
                ((std::get<Indices>(column_store_data_).reserve(new_capacity)), ...);
            }
        }
    }

    template <typename ...TypesToInsert, std::size_t ...Indices>
    void insertImpl (TypesToInsert&& ...values, std::integer_sequence<std::size_t, Indices...>) {
        (std::get<Indices>(column_store_data_).push_back(std::forward<TypesToInsert>(values)), ...);
    }

    template <typename NewDataFrame, std::size_t ...Indices>
    auto selectImpl(std::integer_sequence<std::size_t, Indices...>) {
        NewDataFrame result;
        result.insertRanges(std::get<Indices>(column_store_data_)...);
        return result;
    }

    template <std::size_t ...Indices>
    void appendImpl (const DataFrame& df, std::integer_sequence<std::size_t, Indices...>) {
        internal::insertRangesIntoContainers(column_store_data_, IndicesForColumnStore{}, df.getSize(), std::get<Indices>(df.column_store_data_)...);
    }

    template <typename NewDataFrame, typename NamedTupleForFunc, std::size_t NumSelectedNames, typename Func, std::size_t ...Indices>
    auto applyImpl (Func&& function, std::integer_sequence<std::size_t, Indices...>) {
        NewDataFrame result;
        result.template assureSufficientCapacityInColumnStore(getSize(), typename NewDataFrame::IndicesForColumnStore{});
        internal::insertRangesIntoContainers(result.column_store_data_, std::make_index_sequence<NumSelectedNames>{}, getSize(), std::get<Indices>(column_store_data_)...);
        
        auto& result_column = std::get<NumSelectedNames>(result.column_store_data_);
        for (int i = 0LU; i < getSize(); ++i) {
            result_column.push_back(
                std::forward<Func>(function)(NamedTupleForFunc{std::get<Indices>(column_store_data_)[i]...})
            );
        }
        return result;
    }

    template <typename NamedTupleForFunc, typename Func, std::size_t ...IndicesForFunc, std::size_t ...IndicesForCopy>
    auto queryImpl (Func&& function, std::integer_sequence<std::size_t, IndicesForFunc...>, std::integer_sequence<std::size_t, IndicesForCopy...>) {
        DataFrame result;
        for (int i = 0LU; i < getSize(); ++i) {
            bool keep_row = std::forward<Func>(function)(NamedTupleForFunc{std::get<IndicesForFunc>(column_store_data_)[i]...});
            if (keep_row) {
                ((std::get<IndicesForCopy>(result.column_store_data_).push_back(
                    std::get<IndicesForCopy>(column_store_data_)[i])),...);
            }
        }
        return result;
    }

    template <Join JoinType, typename NewDataFrame, typename JoinIndicesSelf, typename JoinIndicesOther, typename DataIndicesInResultOther, typename DataIndicesToCopyOther, typename DataFrameOther>
    auto joinImpl(const DataFrameOther& df) {
        NewDataFrame result{};
        if constexpr (JoinType == Join::Inner) {
            for (auto i = 0LU; i < getSize(); ++i) {
                for (auto j = 0LU; j < df.getSize(); ++j) {
                    if (compareJoinIndices(df, i, j, JoinIndicesSelf{}, JoinIndicesOther{})) {
                        addJoinedColumnData(result.column_store_data_, df, i, j, IndicesForColumnStore{}, DataIndicesInResultOther{}, DataIndicesToCopyOther{});
                    }
                }
            }
        }
        return result;
    }

    template <typename DataFrameOther, std::size_t ...JoinIndicesSelf, std::size_t ...JoinIndicesOther>
    requires (sizeof...(JoinIndicesSelf) == sizeof...(JoinIndicesOther))
    bool compareJoinIndices (const DataFrameOther& df, std::size_t row_index_self, std::size_t row_index_other, std::integer_sequence<std::size_t, JoinIndicesSelf...>, std::integer_sequence<std::size_t, JoinIndicesOther...>) {
        return ((std::get<JoinIndicesSelf>(column_store_data_)[row_index_self] == std::get<JoinIndicesOther>(df.column_store_data_)[row_index_other]) && ...);
    }

    template <typename JoinedColumnStoreData, typename DataFrameOther, std::size_t ...ColumnIndicesSelf, std::size_t ...DataIndicesInResultOther, std::size_t ...DataIndicesToCopyOther>
    void addJoinedColumnData (JoinedColumnStoreData& joined_data, const DataFrameOther& df, std::size_t row_index_self, std::size_t row_index_other, std::integer_sequence<std::size_t, ColumnIndicesSelf...>, std::integer_sequence<std::size_t, DataIndicesInResultOther...>, std::integer_sequence<std::size_t, DataIndicesToCopyOther...>) {
        ((std::get<ColumnIndicesSelf>(joined_data).push_back(std::get<ColumnIndicesSelf>(column_store_data_)[row_index_self])), ...);
        ((std::get<DataIndicesInResultOther>(joined_data).push_back(std::get<DataIndicesToCopyOther>(df.column_store_data_)[row_index_other])), ...);
    }

    template <std::size_t ...Indices>
    void sortByImpl (DataFrame& result, std::size_t index, std::integer_sequence<std::size_t, Indices...>) {
        ((std::get<Indices>(result.column_store_data_).push_back(
            std::get<Indices>(column_store_data_)[index]
        )), ...);
    }

    ColumnStoreDataType column_store_data_{};
};

#define dacr_param auto data
#define dacr_value(field_name) data.template get<field_name>()

// ############################################################################
// Concept: Is DataFrame
// ############################################################################
template <typename ...>
struct IsDataFrameImpl : std::false_type {

};

template <typename ...Columns>
struct IsDataFrameImpl<DataFrame<Columns...>> : std::true_type {};

template <typename Type>
concept IsDataFrame = IsDataFrameImpl<Type>::value;

} // namespace dacr

#endif // DATA_CRUNCHING_DATAFRAME_HPP