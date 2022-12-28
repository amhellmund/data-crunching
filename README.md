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

## Further Features

Complementary features to enrich the `DataFrame` API, e.g. to read CSV files into `DataFrames` are also made public. This includes among others:

- Type-aware `split` function for `string` and `string_view` to convert split parts into types directly.
- Range adaptors for numeric `sequence`s and text-based file streams to allow using the C++20 ranges library for such activities.

# Example

The below code exemplifies how to use the `DataFrame` class to perform data crunching.

    #include <cmath>
    #include <string>
    
    #include <data_crunching/dataframe.hpp>
    #include <data_crunching/dataframe/load.hpp>

    using DataFrame = dacr::DataFrame<
        dacr::Column<"name", std::string>,
        dacr::Column<"city", std::string>,
        dacr::Column<"age", int>,
        dacr::Column<"size_in_m", double>,
        dacr::Column<"weight_in_kg", int>
    >;

    int main (int argc, char*argv) {
        auto df = dacr::load_csv<DataFrame, dacr::CSVInOrder>("input.csv");

        // compute body-mass-index into a new column applying the lambda for each row
        // the type for column "bmi" is deduced automatically by return value of lambda function
        df.apply<"bmi">([](nc_param) { 
            return (
                static_cast<double>(nc_data("weight_in_kg")) / 
                std::pow(nc_data("size_in_cm"), 2)
            );
        });

        df.print();
    }