---
layout: default
title: String API
parent: API Documentation
nav_order: 2
---

# String Utilities
{:toc}

The *Data Crunching* library provides a type-aware `split` function for string to perform the split operation as well as type conversion in a single step.
The utility function comes in two variants.


## Conversion into Container

The first variant converts all (split) elements into a container of a single type.
In this case the `split` function has no template arguments.

```cpp
std::vector<T> = dacr::split(<string-to-split>, <delimiter>);
std::set<T> = dacr::split(<string-to-split>, <delimiter>);
```

The supported containers are `std::vector` and `std::set`, while the supported types are all arithmetic types and (custom) classes that can be constructed from a string.
When converting to a `std::set`, the types must additionally have the constraints imposed by the `std::set`.


## Conversion into Tuple

The second variant converts all (split) elements into a tuple of provided types.
In this case, the user-defined types are provided as template arguments.

```cpp
auto tuple = dacr::split<T1, T2, ..., TN>(<string-to-split>, <delimiter>);
```

This variant additionally supports skipping elements in the to-be-split string using `dacr::Ignore<Count>`:

```cpp
auto tuple = dacr::split<T1, dacr::Skip<1>, T2>(<stringto-split>, <delimiter>);
```

The first element gets converted to `T1`, while the third element gets converted to `T2`.
The second element is skipped.