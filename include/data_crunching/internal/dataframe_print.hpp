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

namespace dacr {

struct PrintOptions {
    /* column widths for arithmetic data types */
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

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_DATAFRAME_PRINT_HPP