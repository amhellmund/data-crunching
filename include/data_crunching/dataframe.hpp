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

#include "data_crunching/internal/dataframe.hpp"
#include "data_crunching/internal/column.hpp"
#include "data_crunching/internal/name_list.hpp"
#include "data_crunching/internal/type_list.hpp"

namespace dacr {

template <internal::IsColumn ...Columns>
requires internal::are_names_unique<internal::GetColumnNames<Columns...>>
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
    void insertRanges (Ranges&& ...ranges) {
        const std::size_t min_size = internal::getMinSizeFromRanges(std::forward<Ranges>(ranges)...);
        assureSufficientCapacityInColumnStore(min_size, IndicesForColumnStore{});
        internal::insertRangesIntoContainers(column_store_data_, IndicesForColumnStore{}, min_size, std::forward<Ranges>(ranges)...);
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
        using NewDataFrame = internal::GetDataFrameWithColumnsByName<internal::NameList<ColumnNames...>, Columns...>;
        using SelectedColumnIndices = internal::GetColumnIndicesByNames<internal::NameList<ColumnNames...>, Columns...>;
        return selectImpl<NewDataFrame>(SelectedColumnIndices{});
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

    // ############################################################################
    // API: Query
    // ############################################################################
    template <typename SelectNames = SelectAll, typename Func>
    requires (internal::is_valid_select<SelectNames, Columns...>)
    auto query (Func&& function) {
        return DataFrame{};
    }

    // ############################################################################
    // API: Join
    // ############################################################################

    // ############################################################################
    // API: Summarize
    // ############################################################################

    // ############################################################################
    // API: Print
    // ############################################################################

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

    ColumnStoreDataType column_store_data_{};
};

#define dacr_param auto data
#define dacr_value(field_name) data.template get<field_name>()

} // namespace dacr

#endif // DATA_CRUNCHING_DATAFRAME_HPP