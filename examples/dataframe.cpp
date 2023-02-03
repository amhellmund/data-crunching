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
#include <data_crunching/io/csv.hpp>

auto getPersonData() {
    using DataFrame = dacr::DataFrame<
        dacr::Column<"name", std::string>,
        dacr::Column<"city", std::string>,
        dacr::Column<"age", int>,
        dacr::Column<"size_in_m", double>,
        dacr::Column<"weight_in_kg", int>
    >;
    return dacr::load_from_csv<DataFrame>("examples/data/person.csv");
}

auto getCityData() {
    using DataFrame = dacr::DataFrame<
        dacr::Column<"city", std::string>,
        dacr::Column<"country", std::string>,
        dacr::Column<"continent", std::string>
    >;
    return dacr::load_from_csv<DataFrame>("examples/data/city.csv");
}

int main (int argc, char *argv[]) {
    // load the data
    auto df_person = getPersonData(); 

    // create a new column with a computed value
    auto df_bmi = df_person.apply<"bmi">([](dacr_param) { 
        return (
            static_cast<double>(dacr_value("weight_in_kg")) / 
            (dacr_value("size_in_m") * dacr_value("size_in_m"))
        );
    });

    // filter rows by custom lambda function
    auto df_bmi_for_below_60_years = df_bmi.query([](dacr_param) {
        return dacr_value("age") < 60;
    });
    
    // load second dataset
    auto df_city = getCityData();

    // join the two dataframes to a single data frame
    auto df_join_with_city = df_bmi_for_below_60_years.join<dacr::Join::Inner, "city">(df_city);

    // compute summarizations with group-by
    auto df_summarize = df_join_with_city.summarize<
        dacr::GroupBy<"country">,
        dacr::Avg<"bmi", "bmi_avg">
    >();

    // sort the dataframe
    auto df_summarize_sorted = df_summarize.sort<dacr::SortOrder::Ascending, "country">();

    // print the dataframe
    df_summarize_sorted.print({
        .fixedpoint_precision = 4,
        .string_width = 20,
    });
}