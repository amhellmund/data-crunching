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

#ifndef DATA_CRUNCHING_INTERNAL_DATAFRAME_SUMMARIZE_HPP
#define DATA_CRUNCHING_INTERNAL_DATAFRAME_SUMMARIZE_HPP

#include "data_crunching/internal/fixed_string.hpp"
#include "data_crunching/internal/utils.hpp"
#include "data_crunching/internal/name_list.hpp"
#include "data_crunching/internal/column.hpp"

namespace dacr {

struct GroupByNone {};

template <FixedString ...GroupByNames>
struct GroupBy {};

template <FixedString ...ColumnNames>
struct Sum {};

template <FixedString ...ColumnNames>
struct Min {};

template <FixedString ...ColumnNames>
struct Max {};

template <FixedString ...ColumnNames>
struct Avg {};

template <FixedString ...ColumnNames>
struct StdDev {};

template <FixedString ...ColumnNames>
struct CountIf {};

template <FixedString ...ColumnNames>
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

template <FixedString ...ColumnNames>
struct IsSummarizeOpImpl<Sum<ColumnNames...>> : std::true_type {};

template <FixedString ...ColumnNames>
struct IsSummarizeOpImpl<Min<ColumnNames...>> : std::true_type {};

template <FixedString ...ColumnNames>
struct IsSummarizeOpImpl<Max<ColumnNames...>> : std::true_type {};

template <FixedString ...ColumnNames>
struct IsSummarizeOpImpl<Avg<ColumnNames...>> : std::true_type {};

template <FixedString ...ColumnNames>
struct IsSummarizeOpImpl<StdDev<ColumnNames...>> : std::true_type {};

template <FixedString ...ColumnNames>
struct IsSummarizeOpImpl<CountIf<ColumnNames...>> : std::true_type {};

template <FixedString ...ColumnNames>
struct IsSummarizeOpImpl<CountIfNot<ColumnNames...>> : std::true_type {};

template <typename T>
concept IsSummarizeOp = IsSummarizeOpImpl<T>::value;

// ############################################################################
// Concept: Trait Summarizer
// ############################################################################
template <std::size_t InIndex, IsArithmetic T>
class SummarizerSum {
public:
    static constexpr auto getSuffix () {
        return FixedString("_sum");
    }

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
template <typename ...>
struct AreValidSummarizeOpsImpl : std::true_type {};

template <template <FixedString ...> typename Op, FixedString ...ColumnNames, typename ...RestOps, typename ...Columns>
struct AreValidSummarizeOpsImpl<TypeList<Op<ColumnNames...>, RestOps...>, Columns...> {
    static constexpr bool value = are_names_in_columns<NameList<ColumnNames...>, Columns...> && AreValidSummarizeOpsImpl<TypeList<RestOps...>, Columns...>::value;
};

template <typename Ops, typename ...Columns>
constexpr bool are_valid_summarize_ops = AreValidSummarizeOpsImpl<Ops, Columns...>::value;


// ############################################################################
// Trait: GetColumnForOp
// ############################################################################
template <typename, typename ...>
struct GetColumnForOpImpl {};

template <FixedString ColumnName, typename ...Columns>
struct GetColumnForOpImpl<Sum<ColumnName>, Columns...> {
    using type = Column<ColumnName.append("_sum"), GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, typename ...Columns>
struct GetColumnForOpImpl<Min<ColumnName>, Columns...> {
    using type = Column<ColumnName.append("_min"), GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, typename ...Columns>
struct GetColumnForOpImpl<Max<ColumnName>, Columns...> {
    using type = Column<ColumnName.append("_max"), GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, typename ...Columns>
struct GetColumnForOpImpl<Avg<ColumnName>, Columns...> {
    using type = Column<ColumnName.append("_avg"), GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, typename ...Columns>
struct GetColumnForOpImpl<StdDev<ColumnName>, Columns...> {
    using type = Column<ColumnName.append("_stddev"), GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, typename ...Columns>
struct GetColumnForOpImpl<CountIf<ColumnName>, Columns...> {
    using type = Column<ColumnName.append("_cntif"), GetColumnTypeByName<ColumnName, Columns...>>;
};

template <FixedString ColumnName, typename ...Columns>
struct GetColumnForOpImpl<CountIfNot<ColumnName>, Columns...> {
    using type = Column<ColumnName.append("_cntifnot"), GetColumnTypeByName<ColumnName, Columns...>>;
};

template <typename Op, typename ...Columns>
using GetColumnForOp = typename GetColumnForOpImpl<Op, Columns...>::type;

// template <utils::FixedString ColumnName>
// struct GetColumnForOp<Min<ColumnName>> {
//     using type = Column<ColumnName.append("_min"), GetColumnTypeByName<ColumnName>>;
// };

// template <utils::FixedString ColumnName>
// struct GetColumnForOp<Max<ColumnName>> {
//     using type = Column<ColumnName.append("_max"), GetColumnTypeByName<ColumnName>>;
// };

// template <utils::FixedString ColumnName>
// struct GetColumnForOp<Avg<ColumnName>> {
//     using type = Column<ColumnName.append("_avg"), double>;
// };

// template <utils::FixedString ColumnName>
// struct GetColumnForOp<StdDev<ColumnName>> {
//     using type = Column<ColumnName.append("_stddev"), double>;
// };


} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_DATAFRAME_PRINT_HPP