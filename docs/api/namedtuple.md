---
layout: default
title: NamedTuple API
parent: API Documentation
nav_order: 2
---

# NamedTuple
{:toc}

The class `NamedTuple` is a data class (like a struct or `std::tuple`) with string-based read/write access to members.
A `NamedTuple` is built from a list of `Field`s each having a `name` and a `type`:

```cpp
using Field = dacr::Field<"name", type>;
using NamedTuple = dacr::NamedTuple<Field1, Field2, ..., FieldN>;
```

The `name`s of the individual fields must be unique within a `NamedTuple`.
Each `name` must not be empty.


## Construction

The `NamedTuple` type may be constructed with explicit types or via Class-Template-Argument-Deduction (CTAD) of the constructor using initial values.

### Explicit Construction

The `NamedTuple` layout (name/type pairs) is defined expliclty by a list of `Field`s

```cpp
using Point3D = dacr::NamedTuple<
    dacr::Field<"x", double>,
    dacr::Field<"y", double>,
    dacr::Field<"z", double>
>;
```

and finally created at runtime via the constructor by specifying the individual 

```cpp
Point3d point{1.0, -1.0, 0.5};
```

### Implicit Construction via Constructor

Alternatively, the `NamedTuple` layout maybe created directly via the construtor using either the `_field` user-defined literal (if not disabled by `DACR_DISABLE_FIELD_LITERAL`, see [Configuration](config.md)) or the `dacr_field` helper macro. 

```cpp
auto point3d_by_field_literals = dacr::NamedTuple("x"_field = 1.0, "y"_field = -1.0, "z"_field = 0.5);

auto point3d_by_field_macro = dacr::NamedTuple(dacr_field("x") = 1.0, dacr_field("y") = -1.0, dacr_field("z") = 0.5);
```

## Member Access

The members of the `NamedTuple` may be accessed for read and write by the `get` template function:

```cpp
auto x = point.get<"x">();

point.get<"x">() = 2.0;
```

## Structured Bindings

Structured bindings may be used for `NamedTuple` at the moment using the member function `toStdTuple`.
This function returns an instance-qualified (const or non-const) reference of the underlying `std::tuple`.

```cpp
auto& [x, y, z] = point.toStdTuple();

const auto& [x, y, z] = point.toStdTuple();
```