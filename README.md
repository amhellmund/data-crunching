# Data Crunching for C++

Data processing and analysis (also known as data crunching) are core aspects of software engineering.
Programming languages like Python (Pandas) provide feature-rich libraries that allow to express data crunching in a compact, yet powerful way.
The goal of this project is to provide a type-aware and type-safe data crunching library for C++20 (and future standards) offering an intuitive and easy-to-use syntax.

> **DISCLAIMER:**
The project is currently in a proof-of-concept phase to elaborate on the usefullness of type-aware data crunching APIs and the chosen syntax.
Therefore, some of the features are partly implemented or with limited checks on the input arguments (see Limitations for details). 

# DataFrame

The core API of the *data crunching (dacr)* library for C++ is the `DataFrame` which is an in-memory column-store database with efficient memory layout.
The `DataFrame` allows to:

- query rows by user-defined functions
- summarize columns by statistical functions including group-by functionality across multiple columns
- join `DataFrame`s across unique column names
- sort rows across multiple columns

The `DataFrame` API uses some of the latest C++ standard features to assure type-correctness of operations at compile-time, for example to disallow computing the standard deviation for string-based columns.

## Further APIs

Complementary features to enrich the `DataFrame` API, e.g. to read CSV files into `DataFrames` are also made public. This includes among others:

- `NamedTuple` to define and access tuples by name instead of by type or id (like for `std::tuple`).
- Type-aware `split` function for `string` and `string_view` to convert split parts into types directly.
- Range adaptors for numeric `sequence`s and text-based file streams to allow using the C++20 ranges library for such activities.

# Example

The below code exemplifies how to use the `DataFrame` class to perform data crunching.

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