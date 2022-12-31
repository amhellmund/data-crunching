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
    int char_width {1};
    int byte_width {4};
    int short_width {6};
    int integer_width {10};
    int long_width {10};
    int fixedpoint_width{8};
    int fixedpoint_precision{2};
    /* column width for string types */
    int string_width {10};
    /* column width for custom data types */
    int custom_width {10};
    /* one-to-rule-all: column width */
    int column_max_width{10};
    /* maximum number of rows to display */
    int max_rows{100};
};

namespace internal {

template <typename T>
struct DataTypeFormatter {

};

template <>
struct DataTypeFormatter<bool> {
    static void format(std::ostream& stream, bool value, const PrintOptions&) {
        stream << std::boolalpha << value;
    }
};

template <>
struct DataTypeFormatter<char> {
    static void format(std::ostream& stream, char value, const PrintOptions&) {
        stream << value;
    }
};

template <>
struct DataTypeFormatter<short> {
    static void format(std::ostream& stream, short value, const PrintOptions&) {
        stream << value;
    } 
};

template <>
struct DataTypeFormatter<int> {
    static void format(std::ostream& stream, int value, const PrintOptions&) {
        stream << value;
    }
};

template <>
struct DataTypeFormatter<double> {
    static void format(std::ostream& stream, double value, const PrintOptions& print_options) {
        stream << std::setw(print_options.fixedpoint_width) << std::setprecision(print_options.fixedpoint_precision) << value;
    }
};

template <>
struct DataTypeFormatter<float> {
    static void format(std::ostream& stream, float value, const PrintOptions& print_options) {
        stream << std::setw(print_options.fixedpoint_width) << std::setprecision(print_options.fixedpoint_precision) << value;
    }
};

template <>
struct DataTypeFormatter<std::string> {
    static void format(std::ostream& stream, const std::string& value, const PrintOptions& print_options) {
        stream << value;
    }
};

template <FixedString ColumnName, typename ColumnType, std::size_t DataIndex>
class ColumnPrinter {
    ColumnPrinter(int width, std::ostream& stream) : width_{width}, stream_{stream} {}

    void printHeader() const {
        printString(ColumnName.data);
    }

    template <typename ColumnStoreData>
    void printData(const ColumnStoreData& column_store_data, std::size_t row_index, const PrintOptions& print_options) {
        std::stringstream sstr;
        DataTypeFormatter<ColumnType>::format(sstr, std::get<DataIndex>(column_store_data)[row_index], print_options);
        printString(sstr.string());
    }

private:
    void printString(std::string_view str) const {
        if (str.size() <= width_) {
            stream << str << std::string(width - str.size(), ' '); 
        }
        else if (width > 3) {
            stream << str.substr(0, width-3) << std::string(3, '.');
        }
        else {
            stream << str.substr(0, 3);
        }
    }

    int width_;
    std::ostream& stream_;
}

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_DATAFRAME_PRINT_HPP