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

#include <vector>

#include "data_crunching/internal/dataframe.hpp"
#include "data_crunching/internal/column.hpp"
#include "data_crunching/internal/name_list.hpp"
#include "data_crunching/internal/type_list.hpp"

namespace dacr {

template <internal::IsColumn ...Columns>
requires internal::are_names_unique_v<internal::GetColumnNames<Columns...>>
class DataFrame {
public:
    template<internal::IsColumn ...OtherColumns>
    requires internal::are_names_unique_v<internal::GetColumnNames<OtherColumns...>>
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
        internal::is_convertible_to_v<internal::TypeList<TypesToInsert...>, internal::GetColumnTypes<Columns...>>
    )
    void insert (TypesToInsert&& ...values) {
        assure_sufficient_capacity_in_column_store(1, IndicesForColumnStore{});
        insert_impl<TypesToInsert...>(std::forward<TypesToInsert>(values)..., IndicesForColumnStore{});
    }

private:
    using ColumnStoreDataType = internal::ConstructColumnStoreDataType<std::vector, Columns...>;
    using IndicesForColumnStore = std::index_sequence_for<Columns...>;

    template <std::size_t ...Indices>
    void assure_sufficient_capacity_in_column_store (std::size_t amount_to_insert, std::integer_sequence<std::size_t, Indices...>) {
        if constexpr (sizeof...(Columns) > 0) {
            const auto& first_column_data = std::get<0>(column_store_data_);
            if (first_column_data.size() + amount_to_insert >= first_column_data.capacity()) {
                const std::size_t new_capacity = first_column_data.capacity() * 1.50;
                ((std::get<Indices>(column_store_data_).reserve(new_capacity)), ...);
            }
        }
    }

    template <typename ...TypesToInsert, std::size_t ...Indices>
    void insert_impl (TypesToInsert&& ...values, std::integer_sequence<std::size_t, Indices...>) {
        (std::get<Indices>(column_store_data_).push_back(std::forward<TypesToInsert>(values)), ...);
    }

    ColumnStoreDataType column_store_data_{};
};

} // namespace dacr

#endif // DATA_CRUNCHING_DATAFRAME_HPP