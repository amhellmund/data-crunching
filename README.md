[![Build Status](https://github.com/amhellmund/data_crunching/workflows/ci/badge.svg)](https://github.com/amhellmund/data_crunching/actions?query=workflow%3Aci)

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
- Bazel 6.0.0
- clang++ 15.0.4
- libc++ 15.0.4


# Documentation

The full documentation for the _data crunching_ API is available [here](https://amhellmund.github.io/data-crunching/).
Below is a short description of the core APIs with minimal examples get a sneak preview of what is provided.


# Core APIs

The _data crunching_ currently provides these core APIs:

- `DataFrame`: a column-store in-memory database for data analysis.
- `NamedTuple`: a data class combining structural definition with reflection.


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

```cpp
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
```

The output of running this example is:

```
|------------------------------------|
| country              |     bmi_avg |
|------------------------------------|
| Canada               |     39.3929 |
| Germany              |     26.1224 |
| Japan                |     20.2812 |
| South Korea          |     19.3906 |
| USA                  |     29.6495 |
|------------------------------------|
```


## NamedTuple

The `NamedTuple` is a data-class (like `std::tuple`) with string-based read/write accesss to data fields.
Unlike `std::tuple`, each data field has a unique name thereby combining structural definition of data with reflection of its members.
As of `C++20` (and probably `C++23`), C++ does not support reflection of class members.
This data-class offers reflection of data-members eventually.

### Example

```cpp
#include <data_crunching/namedtuple.hpp>

int main (int argc, char*argv[]) {
    // variant 1: NamedTuple definition
    using NT = dacr::NamedTuple<dacr::Field<"a", int>, dacr::Field<"d", double>>;
    NT namedtuple1{10, 20.0};

    // variant 2: direct field initialization
    auto namedtuple2 = dacr::NamedTuple(dacr_field("a") = 10, dacr_field("d") = 20.0);

    // variant 3: direct-field initialization via custom literals
    auto namedtuple3 = dacr::NamedTuple("a"_field = 10, "d"_field = 20.0);

    // access to members
    namedtuple3.get<"a">() = 2.0 * namedtuple3.get<"d">();

    // structured bindings
    auto [a, d] = namedtuple3;
}
```


# Supplemental APIs

In addition to the core APIs, a few supplemental APIs are provided that ease data crunching:

- `ArgumentParser`: a parser for command-line arguments into `NamedTuple`.
- `split`: a utility to split strings into user-defined types


## Argument Parser

The `ArgumentParser` is an utility class to read commandline arguments into a `NamedTuple`.
This allows to define argument parsing without any repitition (like defining an underlying struct to store the values and the command-line arguments, etc).
Like the `DataFrame` class, the `ArgumentParser` tries to check as many failures at compile-time.

> **NOTE:** 
    It is important to mention that the provided `ArgumentParser` is a simple one without advanced features like subcommands.
    It should however be sufficient for major use cases in data crunching applications.

### Example

```cpp
#include <data_crunching/argparse.hpp>

using namespace dacr;

struct Namespace {
    std::string name;
};

std::ostream& operator<< (std::ostream& os, const Namespace& ns) {
    os << "Namespace: " << ns.name;
    return os;
}

int main (int argc, char*argv[]) {
    auto argparser = ArgumentParser(
        "Program to showcase C++ argparse",
        Arg<"namespace", Namespace>(mnemonic("n"), help("The namespace"), optional("cde")),
        Arg<"switch", bool>(mnemonic("s"), help("Help text"), store(true)),
        Arg<"input", std::vector<int>>(positional(), help("The input numbers"))
    );
    auto args = argparser.parse(argc, argv);
    std::cout << args.get<"namespace">() << "\n";
    std::cout << std::boolalpha << args.get<"switch">() << "\n";
    std::cout << args.get<"input">().size() << "\n";
    for (auto v : args.get<"input">()) {
        std::cout << "  " << v << "\n";
    }
}
```


## String Split

The `split` function is a utility to perform the split and type conversion in a single step.
It comes in two flavors: conversion into a `std::vector` or `std::set` of a single type or conversion into `std::tuple` of different types.

### Example

```cpp
#include <data_crunching/string.hpp>

using namespace dacr;

struct Namespace {
    std::string name;
};

int main (int argc, char*argv[]) {
    std::vector<Namespace> ns = dacr::split("abc,cde", ",");
    std::set<int> unique_ids = dacr::split("10,20,30", ",");

    auto tuple = dacr::split<int,double>("10,20.12", ",");
    auto [id, name, size] = dacr::split<int,dacr::Skip<2>,std::string,double>("101,Toronto,James,Last,1.76", ",");
}
```

# Limitations

As stated by the disclaimer on the top of the page, this project is in an early development phase.
The provided APIs have been tested with plain data types only.
Reference types might work depending on the use cases.
In addition, the code has only been tested so far on a single Linux-based platform with a fairly modern
clang-based development environment.
Other development environments might or might not work currently.

# Contributions

If you are interested in the *data crunching* library, help is always appreciated by giving feedback, testing code in other development environments or implementing new features. 

    
