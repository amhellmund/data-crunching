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

#include <filesystem>
#include <fstream>
#include <exception>

#include "data_crunching/dataframe.hpp"
#include "data_crunching/string.hpp"
#include "data_crunching/internal/type_conversion.hpp"
#include "data_crunching/internal/type_list.hpp"

#ifndef DATA_CRUNCHING_IO_CSV_HPP
#define DATA_CRUNCHING_IO_CSV_HPP

namespace dacr {

class IoException : std::runtime_error {
    using std::runtime_error::runtime_error;
};

namespace internal {

template <typename ...Types>
auto splitIntoTuple (const std::string line, const std::string& delimeter, TypeList<Types...>) {
    return split<Types...>(line, delimeter);
}

} // namespace internal

template <IsDataFrame DataFrame>
requires (
    internal::are_types_convertible_from_string<typename DataFrame::ColumnTypes>
)
auto load_from_csv (const std::filesystem::path& path_to_csv, const std::string& delimeter = ",", bool has_header = true) {
    if (not std::filesystem::exists(path_to_csv)) {
        throw IoException(std::format("Failed to read file: {}", path_to_csv.native()));
    }
    
    std::ifstream file_stream {path_to_csv, std::ifstream::in};
    std::string line;
    if (has_header) {
        (void)std::getline(file_stream, line);
    }
    DataFrame result{};
    while (std::getline(file_stream, line)) {
        auto tuple = internal::splitIntoTuple(line, delimeter, typename DataFrame::ColumnTypes{});
        std::apply([&result](auto ...data) {
            result.insert(data...);
        }, tuple);
    }
    return result;
}

} // namespace dacr

#endif // DATA_CRUNCHING_IO_CSV_HPP