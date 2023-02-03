---
layout: default
title: I/O API
parent: API Documentation
nav_order: 2
---

# I/O Utilities
{:toc}

The *Data Crunching* library provides basic I/O utilities to read data from files.


## Load DataFrame from CSV File

The `load_from_csv` function allows loading a CSV file into a `DataFrame`.
The API is:

```cpp
template <typename DataFrame>
DataFrame load_from_csv(const std::filesystem_path& path, const std::string& delimeter = ",", bool has_header = true);
```

The function parameter `has_header` thereby specifies if the first line of the CSV file shall be ignored.