# Data Crunching for C++

Data processing and analysis (also known as data crunching) are core aspects of software engineering.
Programming languages like Python (Pandas) provide feature-rich libraries that allow to express data crunching in a compact, yet powerful way.
The goal of this project is to provide a type-aware and type-safe data crunching library for C++20 (and future standards) offering an intuitive and easy-to-use syntax.

> **DISCLAIMER:**
The project is currently in a proof-of-concept phase to elaborate on the usefullness of type-aware data crunching APIs and the chosen syntax.
Therefore, some of the features are partly implemented or with limited checks on the input arguments (see Limitations for details).

# Compatibility

The _data crunching_ library is known to compile in the following development environment:

- GNU/Linux (Ubuntu)
- Bazel 5.3.2
- clang++ 15.0.4
- libc++ 15.0.4

# Core APIs

The _data crunching_ currently provides these core APIs:

- `DataFrame`: a column-store in-memory database for data analysis
- `NamedTuple`: a data class combining structural definition with reflection

## DataFrame

The `DataFrame` is an in-memory column-store database with column-wise contiguous memory layout.
It uses some of the latest C++ standard features to assure (type-)correctness of operations at compile-time
following the principle: *"The code is correct without semantic failures at runtime"*.
For example, the code checks for every operations that selected columns do exist or it disallows operations for incorrect data types (e.g. standard deviation for string-based data).

Feature-wise, the `DataFrame` provides:

- query rows by user-defined functions
- summarize columns by statistical functions including group-by functionality across multiple columns
- join `DataFrame`s across unique column names
- sort rows across multiple columns

### Example

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


## NamedTuple

The `NamedTuple` is a data-class (like `std::tuple`) with string-based read/write accesss to data fields.
Unlike `std::tuple`, each data field has a unique name thereby combining structural definition of data with reflection of its members.
As of `C++20` (and probably `C++23`), C++ does not support reflection of class members.
This data-class offers reflection of data-members eventually.

### Example

    #include <data_crunching/namedtuple.hpp>

    int main (int argc, char*argv[]) {
        // variant A: NamedTuple definition
        using NT = dacr::NamedTuple<dacr::Field<"a", int>, dacr::Field<"d", double>>;
        NT namedtuple1{10, 20.0};

        // variant B: makeNamedTuple
        auto namedtuple2 = dacr::makeNamedTuple<"a", "d">(10, 20.0);

        // variant C: direct field initialization
        auto namedtuple = dacr::NamedTuple(dacr_field("a") = 10, dacr_field("d") = 20.0);

        namedtuple.get<"a">() = 2.0 * namedtuple.get<"d">();
    }

# Contributions

If you are interested in the *data crunching* library, help is always appreciated by giving feedback, testing code in other development environments or implementing new features. 

    