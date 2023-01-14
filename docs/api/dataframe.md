---
layout: default
title: DataFrame API
parent: API Documentation
nav_order: 2
---

# DataFrame
{:toc}

The class `DataFrame` is an in-memory data storage providing essential operations for data processing, filtering and analysis.
A `DataFrame` is composed of a set of columns with each column having a `name` and a `type`:

```cpp
using Column = dacr::Column<"name", type>;

using DataFrame = dacr::DataFrame<
    Column1,
    Column2,
    ...
    ColumnN
>;
```

The individual elements of a column are stored consecutively in memory using a `std::vector` as underlying storage container.
All columns are guaranteed to have the same size with all column values of some `index` forming a row in the `DataFrame`.

Given the data layout, the `DataFrame` represents an in-memory column-store database offering the following API known from database systems:

- select columns by name
- create a new column with values based on the existing column values
- filter rows by user-provided conditions
- join two dataframes by common columns
- sort rows by multiple columns
- aggregate colummns with group-by semantics
- printing the contents in a tabular representation

The subsequent sections explain the API of the `DataFrame` in detail.

> **NOTE:** Due to the early development state of the library, the `DataFrame` is not yet memory efficient in the sense that data copy operations are minimized.
Instead all operations of the `DataFrame` perform copies of the data.
Future versions will optimize the data access to reduce data copies.

## Construction
{:toc}

A default constructor is provided with any special functionality.

> **NOTE:** More advanced constructors, e.g. providing initial column values, will be provided in future releases.

```cpp
using DataFrame = dacr::DataFrame<dacr::Column<"a", int>>;

DataFrame df {};
```

## Insertion
{:toc}

Column data is inserted into the `DataFrame` by either using `insert` or `insert_ranges`.

### Row-Wise Insertion

```cpp
template <typename ...T>
void insert (T&& ...values);
```

The function `insert` adds a single new row to the `DataFrame` accepting one compatible value for each column in column-definition-order.

```cpp
using DataFrameInsert = dacr::DataFrame<
    dacr::Column<"a", int>,
    dacr::Column<"b", double>,
    dacr::Column<"c", std::string>
>;

DataFrameInsert df{};

df.insert(10, 20.0, "abc");
```

### Column-Wise Insertion

```cpp
template <RangeWithSize ...Ranges>
std::size_t insert_ranges (Ranges&& ...ranges);
```

The function `insert_ranges` adds multiple values for each column at once.
It accepts ranges for each column with one range for each column in column-definition order.
Due to the invariant of the `DataFrame` that all columns must have an equal size, this function inserts only as many elements as are provided by the smallest range.
The number of elements inserted is returned.

```cpp
std::vector<int> range_a {1, 2, 3};
std::set<double> range_b {3.0, 4.0, 5.0};
std::array<std::string, 3> range_c {"a", "b"};

df.insert_ranges(range_a, range_b, range_c); // only two elements are inserted from each range
```

## Column Filter
{:toc}

```cpp
template <FixedString ...SelectNames>
NewDataFrame select ();
```

The function `select` filters out complete columns from the `DataFrame` returning a new `DataFrame` with only the selected columns as data.
This operation currently performs a deep copy of the selected column data.

```cpp
using DataFrameSelect = dacr::DataFrame<
    dacr::Column<"a", int>,
    dacr::Column<"b", double>
>;

DataFrameSelect df{};

auto df_select = df.select<"a">();
// decltype(df_select) == DataFrame<Column<"a", int>>
```

## Row Filter
{:toc}

```cpp
template <typename SelectNames = SelectAll, typename Func>
DataFrame query (Func&& query_function);
```

The function `query` filters rows by a user-defined lambda function.
The argument passed to the lambda function is a `NamedTuple` with the field names and types corresponding to the column names and types of the `DataFrame`.
The lambda function is expected to return a `bool` value indicating if a row shall be kept (`true`) or filtered out (`false`).

```cpp
using DataFrameQuery = dacr::DataFrame<
    dacr::Column<"a", int>,
    dacr::Column<"b", double>
>;

DataFrameQuery df{};

auto df_query = df.query([](dacr_param) {
    return dacr_value("a") > dacr_value("b");
});
```

### Query Lambda Function

In plain C++, the lambda function with access to the `NamedTuple` would look like this:

```cpp
[](const auto& data) {
    return data.template get<"bool_column">();
}
```

The `DataFrame` API however provides two macros as syntactic sugar

```cpp
[](dacr_param) {
    return dacr_value("bool_column");
}
```

which correspond to the plain C++ version above.

### Column Selection for Query

If the `query` function should be executed on a reduced list of columns only, it is possible for slight performance improvements to use an additonal `dacr::Select`.
In this case, the `NamedTuple` passed to the lambda function only contains the fields
from the `dacr::Select` list.

```cpp
auto df_query = df.query<dacr::Select<"a">>([](dacr_param) {
    return dacr_value("a") > 10; // access to dacr_value("b") is invalid
});

```

## Column Extension
{:toc}

```cpp
template <FixedString NewColumnName, typename SelectNames = SelectAll, typename Func>
NewDataFrame apply (Func&& apply_function);
```

The function `apply` creates a new column with name `NewColumnName` by invoking the passed lambda function for each row.
The type for the new column is deduced from the return value of the passed function.
The function returns a new `DataFrame` with an added column `dacr::Column<NewColumnname, DeducedColumnType>`.
The data from the already existing columns is copied to the new instance.

> **NOTE:** Future versions of this API will provide move-semantics for performance improvement.

```cpp
using DataFrameApply = dacr::DataFrame<
    dacr::Column<"a", int>,
    dacr::Column<"b", double>
>;

DataFrameApply df{};

// new column type is: double
auto df_apply = df.apply<"c">([](dacr_param) {
    return dacr_value("a") * dacr_value("b");
});
```
### Column Selection for Apply

If the `apply` function should be executed on a reduced list of columns only, it is possible for slight performance improvements to use an additonal `dacr::Select`.
In this case, the `NamedTuple` passed to the lambda function only contains the fields
from the `dacr::Select` list.

```cpp
auto df_apply = df.apply<dacr::Select<"a">>([](dacr_param) {
    return dacr_value("a") > 10; // access to dacr_value("b") is invalid
});
```

## Join Operation
{:toc}

```cpp
template <Join JoinType, FixedString ...JoinNames, typename OtherDataFrame>
NewDataFrame join (const OtherDataFrame& otherDataFrame);
```

The function `join` merges two `DataFrames` together by a set of common columns using the to be specified `JoinType`.
The common columns must be identical in `name` and `type`.
The result is a new `DataFrame` consisting of the common columns and the union of the remaining columns of both `DataFrame`s.
The union of the remaining columns must be unique.

This API currently supports the following `JoinType`s:

| JoinType | Description |
|:--------:|:------------|
| `Inner` | An inner join taking only rows where the common columns match by equality. Other columns get dropped. |

```cpp
using DataFrameJoin1 = dacr::DataFrame<
    dacr::Column<"id1", int>,
    dacr::Column<"id2", char>,
    dacr::Column<"value_left", double>
>;

using DataFrameJoin2 = dacr::DataFrame<
    dacr::Column<"id1", int>,
    dacr::Column<"id2", char>,
    dacr::Column<"value_right", std::string>
>;

DataFrameJoin1 df1{};
DataFrameJoin2 df2{};

auto df_joined = df1.join<dacr::Join::Inner, "id1", "id2">(df2);
// decltype(df_joined) == dacr::DataFrame<
//     dacr::Column<"id1", int>,
//     dacr::Column<"id2", char>,
//     dacr::Column<"value_left", double>,
//     dacr::Column<"value_right", std::string>
// >
```

## Aggregation
{:toc}

```cpp
template <typename GroupBySpec, typename ...Operations>
NewDataFrame summarize ();
```

The function `summarize` performs an aggregation of columns by applying a set pre-defined operations with an optional group-by semantic.

The `GroupBySpec` is either `dacr::GroupByNone` or `dacr::GroupBy<ColumnNames>`.
If `dacr::GroupByNone` is specified, the aggregation is performed for all values of a column.
If `dacr::GroupBy` is used, the aggregation is performed for each distinct set of column values as identifier by `ColumnNames`.

The `Operation` has the general syntax:

```cpp
using Operation = dacr::OpName<ColumnName, AggregationColumName>;
```

The aggregated value for column `ColumnName` is stored in column `AggregationColumnName`.

The provided operations (`OpName`s) are:

| OpName | Applicable To | Result Type | Description |
|:------:|:-------------:|:-----------:|:------------|
| `Sum` | Types with `operator+` | Column Type | Compute the summation of all column values. |
| `Min` | Types with `operator<` | Column Type | Determine the minimal column value. |
| `Max` | Types with `operator>` | Column Type | Determine the maximal column value. |
| `Avg` | Arithmetic Types | `double` | Compute the average of all column values. |
| `StdDev` | Arithmetic Types | `double` | Compute the standard deviation of all column values. This operation currently stores all column values (higher memory usage) to compute firstly the average and then, secondly, the standard deviation. |
| `CountIf` | Boolean Types | `int` | Count the `true` elements of a boolean column. |
| `CountIfNot` | Boolean Types | `int` | Count the `false` elements of a boolean column. | 

```cpp
using DataFrameSummarize = dacr::DataFrame<
    dacr::Column<"country", std::string>,
    dacr::Column<"continent", std::string>,
    dacr::Column<"age", int>,
    dacr::Column<"weight", int>,
    dacr::Column<"female", bool>
>;

DataFrameSummarize df{};
auto df_summarize = df.summarize<
    dacr::GroupBy<"country", "continent">,
    dacr::Avg<"age", "age_avg">,
    dacr::Max<"weight", "weight_max">,
    dacr::CountIf<"female", "number_women">
>;
// decltype(df_summarize) == dacr::DataFrame<
//     dacr::Column<"country", std::string>,
//     dacr::Column<"continent", std::string>,
//     dacr::Column<"age_avg", double>,
//     dacr::Column<"weight_max", int>,
//     dacr::Column<"number_women", int> 
// >;
```

## Sorting
{:toc}

```cpp
template <SortOrder Order, FixedString ...SortByNames>
DataFrame sortBy ();
```

The function `sortBy` sorts the `DataFrame` row-wise by multiple columns.
The comparison between rows is performed by `operator<`.
The precendence of comparisons between the columns is defined by the sequence of `SortByNames`, meaning:

* check if rows are sorted by `SortByName1`
* check if rows are sorted by `SortByName2`
* ...
* check if rows are sorted by `SortBynameN`

The `SortOrder` is either `Ascending` or `Descending`.

```cpp
using DataFrameSort = dacr::DataFrame<
    dacr::Column<"a", int>,
    dacr::Column<"b", double>,
    dacr::Column<"c", std::string>
>;

DataFrameSort df{};
auto df_sorted = df.sortBy<dacr::SortOrder::Ascending, "a", "b">();
```

## Appending
{:toc}

```cpp
DataFrame append (const DataFrame& other);
```

The function `append` adds the rows of a `DataFrame` of the same type.

```cpp
using DataFrameAppend = dacr::DataFrame<
    dacr::Column<"a", int>,
    dacr::Column<"b", double>
>;

DataFrameAppend df1{}, df2{};
auto df_append = df1.append(df2);
```

## Printing
{:toc}

```cpp
void print (const dacr::PrintOptions& print_options, std::ostream& stream = std::cout);
```

The function `print` dumps the content of the `DataFrame` to a stream.
It requires the `operator<<` to be implemented for custom types.
It accepts two parameters as input: a set of `PrintOptions` to customize the output and the `stream` to dump to.
The `PrintOptions` contain the following configuration options:

| Option | Default | Description | 
|:------:|:-------:|:------------|
| `fixedpoint_precision` | 2 | The precision of floating point types. |
| `fixedpoint_width` | 10 | The width of floating point types. |
| `custom_width` | 10 | The maximum width of custom types. |
| `string_width` | 10 | The maximum width of string-based types. |
| `max_rows` | all | The maximum number of rows to display. |

```cpp
using DataFramePrint = dacr::DataFrame<
    dacr::Column<"a", int>,
    dacr::Column<"b", std::string>
>;
data
DataFramePrint df{};
df.print({
    .string_width = 20,
});
```

**Important**: the print options must be specified in the order of the table if multiple parameters are set at once.
This is due to the language rules for aggregate initialization of structs.

## Raw Data Access
{:toc}

### Size of DataFrame

```cpp
std::size_t getSize () const;
```

The function `getSize` returns the number of rows in the `DataFrame`.

### Column Access

```cpp
template <FixedString ColumnName>
std::vector<Type>& getColumn ();
```

The function `getColumn` returns an instance-qualified `std::vector` reference to the column data.

> **NOTE:** This will later be changed into a `DataSeries` type in later versions once the `DataSeries` type is introduced.