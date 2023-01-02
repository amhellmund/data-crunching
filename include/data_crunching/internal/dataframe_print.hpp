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

#ifndef DATA_CRUNCHING_INTERNAL_DATAFRAME_PRINT_HPP
#define DATA_CRUNCHING_INTERNAL_DATAFRAME_PRINT_HPP

#include <concepts>
#include <iomanip>
#include <limits>
#include <sstream>
#include <string>

#include "data_crunching/internal/fixed_string.hpp"

namespace dacr {

struct PrintOptions {
    /* column widths for arithmetic data types */
    std::size_t fixedpoint_width{8};
    std::size_t fixedpoint_precision{2};
    /* column width for string types */
    std::size_t string_width {10};
    /* column width for custom data types */
    std::size_t custom_width {10};
    /* maximum number of rows to display. Default: all */
    std::size_t max_rows{std::numeric_limits<std::size_t>::max()};
};

namespace internal {

// ############################################################################
// Concepts: Arithmetic Data Types
// ############################################################################
template <typename T>
concept IsIntegral = std::is_integral_v<T>;

template <typename T>
concept IsFloatingPoint = std::is_floating_point_v<T>;

// ############################################################################
// Trait: Data Formatter
// ############################################################################
template <typename T>
struct DataFormatter {
    static void format(std::ostream& stream, const T& value, const PrintOptions&) {
        stream << value;
    }

    static int getWidth (const PrintOptions& print_options) {
        return print_options.custom_width;
    }
};

template <IsIntegral T>
struct DataFormatter<T> {
    static void format(std::ostream& stream, const T& value, const PrintOptions&) {
        stream << value;
    }

    static int getWidth (const PrintOptions&) {
        return std::numeric_limits<T>::digits10 + 1 /* next higher digit */ + 1 /* sign */;
    }
};

template <IsFloatingPoint T>
struct DataFormatter<T> {
    static void format(std::ostream& stream, const T& value, const PrintOptions& print_options) {
        stream << std::left << std::setw(print_options.fixedpoint_width) << std::setprecision(print_options.fixedpoint_precision) << value;
    }

    static int getWidth (const PrintOptions& print_options) {
        return print_options.fixedpoint_width + 1 /* sign */;
    }
};

template <>
struct DataFormatter<bool> {
    static constexpr int MAX_WIDTH_BOOL = 5;

    static void format(std::ostream& stream, bool value, const PrintOptions&) {
        stream << std::boolalpha << value;
    }

    static int getWidth (const PrintOptions&) {
        return MAX_WIDTH_BOOL;
    }
};

template <>
struct DataFormatter<std::string> {
    static void format(std::ostream& stream, const std::string& value, const PrintOptions&) {
        stream << value;
    }

    static int getWidth (const PrintOptions& print_options) {
        return print_options.string_width;
    }
};


// ############################################################################
// Trait: Printer
// ############################################################################
template <FixedString ColumnName, typename ColumnType, std::size_t DataIndex>
class ColumnPrinter {
public:
    static void printHeader(std::ostream& stream, const PrintOptions& print_options) {
        printString(stream, ColumnName.data, getWidth(print_options));
    }

    static void printLineSeparator(std::ostream& stream, const PrintOptions& print_options) {
        stream << std::string(getWidth(print_options), '-');
    }

    template <typename ColumnStoreData>
    static void printData(std::ostream& stream, const PrintOptions& print_options, const ColumnStoreData& column_store_data, std::size_t row_index) {
        std::stringstream sstr;
        DataFormatter<ColumnType>::format(sstr, std::get<DataIndex>(column_store_data)[row_index], print_options);
        printString(stream, sstr.str(), getWidth(print_options));
    }

    static int getWidth(const PrintOptions& printOptions) {
        return std::max<int>(DataFormatter<ColumnType>::getWidth(printOptions), MIN_COLUMN_WIDTH);
    }

private:
    static constexpr std::size_t MIN_COLUMN_WIDTH = 3;

    static void printString(std::ostream& stream, std::string_view str, int width) {
        if (str.size() <= width) {
            stream << str << std::string(width - str.size(), ' '); 
        }
        else if (width > MIN_COLUMN_WIDTH) {
            stream << str.substr(0, width - 2) << std::string(2, '.');
        }
        else {
            stream << str.substr(0, MIN_COLUMN_WIDTH);
        }
    }
};

class RowStartPrinter {
public:
    static void printHeader(std::ostream& stream, const PrintOptions&) {
        stream << "| ";
    }

    static void printLineSeparator(std::ostream& stream, const PrintOptions&) {
        stream << "|-";
    }

    template <typename ColumnStoreData>
    static void printData(std::ostream& stream, const PrintOptions&, const ColumnStoreData&, std::size_t) {
        stream << "| ";
    }
};

class RowEndPrinter {
public:
    static void printHeader(std::ostream& stream, const PrintOptions&) {
        stream << " |\n";
    }

    static void printLineSeparator(std::ostream& stream, const PrintOptions&) {
        stream << "-|\n";
    }

    template <typename ColumnStoreData>
    static void printData(std::ostream& stream, const PrintOptions&, const ColumnStoreData&, std::size_t) {
        stream << " |\n";
    }
};

class ColumnSeparatorPrinter {
public:
    static void printHeader(std::ostream& stream, const PrintOptions&) {
        stream << " | ";
    }

    static void printLineSeparator(std::ostream& stream, const PrintOptions&) {
        stream << "---";
    }

    template <typename ColumnStoreData>
    static void printData(std::ostream& stream, const PrintOptions&, const ColumnStoreData&, std::size_t) {
        stream << " | ";
    }
};

// ############################################################################
// Trait: PrintExecuter
// ############################################################################
template <typename ...Printer>
class PrintExecuter {
public:
    PrintExecuter(std::ostream& stream, const PrintOptions& print_options) : stream_{stream}, print_options_{print_options} {
    }

    template <typename ColumnStoreData>
    void print(const ColumnStoreData& column_store_data) {
        printHeader();
        auto size = std::get<0>(column_store_data).size();
        printData(column_store_data, size);
    }

private:
    void printHeader() {
        ((Printer::printLineSeparator(stream_, print_options_)), ...);
        ((Printer::printHeader(stream_, print_options_)), ...);
        ((Printer::printLineSeparator(stream_, print_options_)), ...);
    }

    template <typename ColumnStoreData>
    void printData(const ColumnStoreData& column_store_data, std::size_t size) {
        std::size_t rows_to_display = std::min(size, print_options_.max_rows);
        for (auto row_index = 0LU; row_index < rows_to_display; ++row_index) {
            ((Printer::printData(stream_, print_options_, column_store_data, row_index)), ...);
        }
        ((Printer::printLineSeparator(stream_, print_options_)), ...);
        stream_ << "Rows in DataFrame: " << size << "\n";
    }

    std::ostream& stream_;
    const PrintOptions& print_options_;
};

// ############################################################################
// Trait: Merge Print Executer
// ############################################################################
template <typename, typename>
struct PrintExecuterMergeImpl {};

template <typename ...Printers1, typename ...Printers2>
struct PrintExecuterMergeImpl<PrintExecuter<Printers1...>, PrintExecuter<Printers2...>> {
    using type = PrintExecuter<Printers1..., Printers2...>;
};

template <typename Printer1, typename Printer2>
using PrintExecuterMerge = typename PrintExecuterMergeImpl<Printer1, Printer2>::type;

// ############################################################################
// Trait: Construct Print Executer
// ############################################################################
template <typename, typename, typename>
struct ConstructPrintExecuterImpl {};

template <FixedString ColName, typename ColType, std::size_t ColIndex>
struct ConstructPrintExecuterImpl<NameList<ColName>, TypeList<ColType>, std::integer_sequence<std::size_t, ColIndex>> {
    using type = PrintExecuter<ColumnPrinter<ColName, ColType, ColIndex>, RowEndPrinter>;
};

template <FixedString FirstColName, FixedString ...RestColNames, typename FirstColType, typename ...RestColTypes, std::size_t FirstColIndex, std::size_t ...RestColIndices>
struct ConstructPrintExecuterImpl<NameList<FirstColName, RestColNames...>, TypeList<FirstColType, RestColTypes...>, std::integer_sequence<std::size_t, FirstColIndex, RestColIndices...>> {
    using type = PrintExecuterMerge<
        PrintExecuter<ColumnPrinter<FirstColName, FirstColType, FirstColIndex>, ColumnSeparatorPrinter>,
        typename ConstructPrintExecuterImpl<NameList<RestColNames...>, TypeList<RestColTypes...>, std::integer_sequence<std::size_t, RestColIndices...>>::type
    >;
};

template <typename ColumnNames, typename ColumnTypes, typename ColumnIndices>
using ConstructPrintExecuter = PrintExecuterMerge<
    PrintExecuter<RowStartPrinter>,
    typename ConstructPrintExecuterImpl<ColumnNames, ColumnTypes, ColumnIndices>::type
>;

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_DATAFRAME_PRINT_HPP