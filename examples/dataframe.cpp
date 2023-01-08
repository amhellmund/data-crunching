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

#include <string>
#include <data_crunching/dataframe.hpp>

auto getPersonData() {
    using DataFrame = dacr::DataFrame<
        dacr::Column<"name", std::string>,
        dacr::Column<"city", std::string>,
        dacr::Column<"age", int>,
        dacr::Column<"size_in_m", double>,
        dacr::Column<"weight_in_kg", int>
    >;

    DataFrame df;
    df.insert("NameA", "Berlin", 30, 1.75, 80);
    df.insert("NameB", "London", 62, 1.60, 100);
    df.insert("NameC", "Seoul", 20, 1.90, 70);
    df.insert("NameD", "Tokio", 59, 1.72, 60);
    df.insert("NameE", "San Francisco", 40, 1.79, 95);
    df.insert("NameF", "Toronto", 51, 1.99, 156);
    return df;
}

auto getCityData() {
    using DataFrame = dacr::DataFrame<
        dacr::Column<"city", std::string>,
        dacr::Column<"country", std::string>,
        dacr::Column<"continent", std::string>
    >;

    DataFrame df;
    df.insert("Berlin", "Germany", "Europe");
    df.insert("London", "England", "Europe");
    df.insert("Seoul", "South Korea", "Asia");
    df.insert("Tokio", "Japan", "Asia");
    df.insert("San Francisco", "USA", "North America");
    df.insert("Toronto", "Canada", "North America");
    return df;
}

int main (int argc, char *argv[]) {
    auto df_person = getPersonData(); 
    auto df_bmi = df_person.apply<"bmi">([](dacr_param) { 
        return (
            static_cast<double>(dacr_value("weight_in_kg")) / 
            (dacr_value("size_in_m") * dacr_value("size_in_m"))
        );
    });
    auto df_bmi_below_60 = df_bmi.query([](dacr_param) {
        return dacr_value("age") < 60;
    });
    
    auto df_city = getCityData();  
    auto df_join_with_city = df_bmi_below_60.join<dacr::Join::Inner, "city">(df_city);
    auto df_summarize = df_join_with_city.summarize<
        dacr::GroupBy<"country">,
        dacr::Avg<"bmi", "bmi_avg">
    >();

    auto df_summarize_sorted = df_summarize.sortBy<dacr::SortOrder::Ascending, "country">();

    df_summarize_sorted.print({
        .string_width = 20,
    });
}