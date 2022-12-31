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

#include <iomanip>
#include <sstream>
#include <string>

#include "data_crunching/internal/fixed_string.hpp"

namespace dacr {

struct PrintOptions {
    /* column widths for arithmetic data types */
    int integer_width {10};
    int long_width {10};
    int fixedpoint_width{8};
    int fixedpoint_precision{2};
    /* column width for string types */
    int string_width {10};
    /* column width for custom data types */
    int custom_width {10};
    /* maximum number of rows to display */
    int max_rows{100};
};

namespace internal {

template <typename T>
struct DataFormatter {
    static void format(std::ostream& stream, const T& value, const PrintOptions&) {
        stream << value;
    }

    static int getWidth (const PrintOptions& print_options) {
        return print_options.custom_width;
    }
};

template <>
struct DataFormatter<bool> {
    static void format(std::ostream& stream, bool value, const PrintOptions&) {
        stream << std::boolalpha << value;
    }

    static int getWidth (const PrintOptions& print_options) {
        return 5;
    }
};

template <>
struct DataFormatter<char> {
    static void format(std::ostream& stream, char value, const PrintOptions&) {
        stream << value;
    }

    static int getWidth (const PrintOptions& print_options) {
        return 6;
    }
};

template <>
struct DataFormatter<short> {
    static void format(std::ostream& stream, short value, const PrintOptions&) {
        stream << value;
    }

    static int getWidth (const PrintOptions& print_options) {
        return 6;
    }
};

template <>
struct DataFormatter<int> {
    static void format(std::ostream& stream, int value, const PrintOptions&) {
        stream << value;
    }

    static int getWidth (const PrintOptions& print_options) {
        return 10;
    }
};

template <>
struct DataFormatter<double> {
    static void format(std::ostream& stream, double value, const PrintOptions& print_options) {
        stream << std::left << std::setw(print_options.fixedpoint_width) << std::setprecision(print_options.fixedpoint_precision) << value;
    }

    static int getWidth (const PrintOptions& print_options) {
        return print_options.fixedpoint_width;
    }
};

template <>
struct DataFormatter<float> {
    static void format(std::ostream& stream, float value, const PrintOptions& print_options) {
        stream << std::left << std::setw(print_options.fixedpoint_width) << std::setprecision(print_options.fixedpoint_precision) << value;
    }

    static int getWidth (const PrintOptions& print_options) {
        return print_options.fixedpoint_width;
    }
};

template <FixedString ColumnName, typename ColumnType, std::size_t DataIndex>
class ColumnPrinter {
public:
    static void printHeader(std::ostream& stream, const PrintOptions& print_options) {
        printString(stream, ColumnName.data, getWidth(print_options));
    }

    static void printHeaderSeparator(std::ostream& stream, const PrintOptions& print_options) {
        stream << std::string(getWidth(print_options), '-');
    }

    template <typename ColumnStoreData>
    static void printData(std::ostream& stream, const PrintOptions& print_options, const ColumnStoreData& column_store_data, std::size_t row_index) {
        std::stringstream sstr;
        DataFormatter<ColumnType>::format(sstr, std::get<DataIndex>(column_store_data)[row_index], print_options);
        printString(stream, sstr.str(), getWidth(print_options));
    }

    static int getWidth(const PrintOptions& printOptions) {
        return DataFormatter<ColumnType>::getWidth(printOptions);
    }

private:
    static void printString(std::ostream& stream, std::string_view str, int width) {
        if (str.size() <= width) {
            stream << str << std::string(width - str.size(), ' '); 
        }
        else if (width > 3) {
            stream << str.substr(0, width - 3) << std::string(3, '.');
        }
        else {
            stream << str.substr(0, 3);
        }
    }
};

class RowStartPrinter {
public:
    static void printHeader(std::ostream& stream, const PrintOptions&) {
        stream << "| ";
    }

    static void printHeaderSeparator(std::ostream& stream, const PrintOptions&) {
        stream << "|-";
    }

    template <typename ColumnStoreData>
    static void printData(std::ostream& stream, const PrintOptions&, const ColumnStoreData&, std::size_t) {
        stream << "| ";
    }

    static int getWidth(const PrintOptions&) {
        return 2;
    }
};

class RowEndPrinter {
public:
    static void printHeader(std::ostream& stream, const PrintOptions&) {
        stream << " |\n";
    }

    static void printHeaderSeparator(std::ostream& stream, const PrintOptions&) {
        stream << "-|\n";
    }

    template <typename ColumnStoreData>
    static void printData(std::ostream& stream, const PrintOptions&, const ColumnStoreData&, std::size_t) {
        stream << " |\n";
    }

    static int getWidth(const PrintOptions&) {
        return 2;
    }
};

class ColumnSeparatorPrinter {
public:
    static void printHeader(std::ostream& stream, const PrintOptions&) {
        stream << " | ";
    }

    static void printHeaderSeparator(std::ostream& stream, const PrintOptions&) {
        stream << "---";
    }

    template <typename ColumnStoreData>
    static void printData(std::ostream& stream, const PrintOptions&, const ColumnStoreData&, std::size_t) {
        stream << " | ";
    }

    static int getWidth(const PrintOptions&) {
        return 3;
    }
};

template <typename ...Printer>
struct PrintExecuter {
    PrintExecuter(std::ostream& stream, const PrintOptions& print_options) : stream_{stream}, print_options_{print_options} {
    }

    void printHeader() {
        stream_ << __PRETTY_FUNCTION__ << "\n";
        ((Printer::printHeader(stream_, print_options_)), ...);
        ((Printer::printHeaderSeparator(stream_, print_options_)), ...);
    }

    template <typename ColumnStoreData>
    void printData(const ColumnStoreData& column_store_data, std::size_t row_index) {
        ((Printer::printData(stream_, print_options_, column_store_data, row_index)), ...);
    }

    std::ostream& stream_;
    const PrintOptions& print_options_;
};

template <typename, typename>
struct PrintExecuterMergeImpl {};

template <typename ...Printers1, typename ...Printers2>
struct PrintExecuterMergeImpl<PrintExecuter<Printers1...>, PrintExecuter<Printers2...>> {
    using type = PrintExecuter<Printers1..., Printers2...>;
};

template <typename Printer1, typename Printer2>
using PrintExecuterMerge = typename PrintExecuterMergeImpl<Printer1, Printer2>::type;

template <typename, typename, typename>
struct ConstructColumnPrinterImpl {};

template <FixedString ColName, typename ColType, std::size_t ColIndex>
struct ConstructColumnPrinterImpl<NameList<ColName>, TypeList<ColType>, std::integer_sequence<std::size_t, ColIndex>> {
    using type = PrintExecuter<ColumnPrinter<ColName, ColType, ColIndex>, RowEndPrinter>;
};

template <FixedString FirstColName, FixedString ...RestColNames, typename FirstColType, typename ...RestColTypes, std::size_t FirstColIndex, std::size_t ...RestColIndices>
struct ConstructColumnPrinterImpl<NameList<FirstColName, RestColNames...>, TypeList<FirstColType, RestColTypes...>, std::integer_sequence<std::size_t, FirstColIndex, RestColIndices...>> {
    using type = PrintExecuterMerge<
        PrintExecuter<ColumnPrinter<FirstColName, FirstColType, FirstColIndex>, ColumnSeparatorPrinter>,
        typename ConstructColumnPrinterImpl<NameList<RestColNames...>, TypeList<RestColTypes...>, std::integer_sequence<std::size_t, RestColIndices...>>::type
    >;
};

template <typename ColumnNames, typename ColumnTypes, typename ColumnIndices>
using ConstructColumnPrinter = PrintExecuterMerge<
    PrintExecuter<RowStartPrinter>,
    typename ConstructColumnPrinterImpl<ColumnNames, ColumnTypes, ColumnIndices>::type
>;

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_DATAFRAME_PRINT_HPP