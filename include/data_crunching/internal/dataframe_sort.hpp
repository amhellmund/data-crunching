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

#ifndef DATA_CRUNCHING_INTERNAL_DATAFRAME_SORT_HPP
#define DATA_CRUNCHING_INTERNAL_DATAFRAME_SORT_HPP

#include <type_traits>

namespace dacr {

enum class SortOrder {
    Ascending = 0,
    Descending,
};

namespace internal {

// ############################################################################
// Trait: Element Comparison
// ############################################################################
template <SortOrder Order, std::size_t CompareIndex, typename RecursiveSortCompare>
struct CompareElementsForSort {
    template <typename DataIn>
    static bool compare(const DataIn& data_in, std::size_t index_lhs, std::size_t index_rhs) {
        if constexpr (Order == SortOrder::Ascending) {
            if constexpr (std::is_same_v<RecursiveSortCompare, void>) {
                return std::get<CompareIndex>(data_in)[index_lhs] < std::get<CompareIndex>(data_in)[index_rhs];
            }
            else {
                return (std::get<CompareIndex>(data_in)[index_lhs] < std::get<CompareIndex>(data_in)[index_rhs] || (
                    std::get<CompareIndex>(data_in)[index_lhs] == std::get<CompareIndex>(data_in)[index_rhs] && RecursiveSortCompare::compare(data_in, index_lhs, index_rhs)
                ));
            }
        }
        else {
            if constexpr (std::is_same_v<RecursiveSortCompare, void>) {
                return std::get<CompareIndex>(data_in)[index_lhs] > std::get<CompareIndex>(data_in)[index_rhs];
            }
            else {
                return (std::get<CompareIndex>(data_in)[index_lhs] > std::get<CompareIndex>(data_in)[index_rhs] || (
                    std::get<CompareIndex>(data_in)[index_lhs] == std::get<CompareIndex>(data_in)[index_rhs] && RecursiveSortCompare::compare(data_in, index_lhs, index_rhs)
                ));
            }
        }
    }
};

// ############################################################################
// Trait: Construct Element Comparison
// ############################################################################
template <SortOrder, typename>
struct ConstructElementComparisonImpl {};

template <SortOrder Order, std::size_t LastIndex>
struct ConstructElementComparisonImpl<Order, std::integer_sequence<std::size_t, LastIndex>> {
    using type = CompareElementsForSort<Order, LastIndex, void>;
};

template <SortOrder Order, std::size_t FirstIndex, std::size_t ...RestIndices>
struct ConstructElementComparisonImpl<Order, std::integer_sequence<std::size_t, FirstIndex, RestIndices...>> {
    using type = CompareElementsForSort<Order, FirstIndex, typename ConstructElementComparisonImpl<Order, std::integer_sequence<std::size_t, RestIndices...>>::type>;
};

template <SortOrder Order, typename Indices>
using ConstructElementComparison = typename ConstructElementComparisonImpl<Order, Indices>::type;


// ############################################################################
// Trait: Element Proxy
// ############################################################################
template <typename ColumnStoreDataType, typename ElementComparison>
class ColumnStoreRowComparisonProxy {
public:
    ColumnStoreRowComparisonProxy(const ColumnStoreDataType& data, std::size_t index) : data_{data}, index_{index} {}
        
    ColumnStoreRowComparisonProxy& operator=(const ColumnStoreRowComparisonProxy& other) {
        data_ = other.data_;
        index_ = other.index_;
        return *this;
    }

    std::size_t getIndex() const {
        return index_;
    }

    friend bool operator< (const ColumnStoreRowComparisonProxy& lhs, const ColumnStoreRowComparisonProxy& rhs) {
        return ElementComparison::template compare(lhs.data_.get(), lhs.index_, rhs.index_);
    }

private:
    std::reference_wrapper<const ColumnStoreDataType> data_;
    std::size_t index_;
};


} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_DATAFRAME_SORT_HPP