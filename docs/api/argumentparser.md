---
layout: default
title: ArgumentParser API
parent: API Documentation
nav_order: 2
---

# Argument Parser
{:toc}

The class `ArgumentParser` is an utility class to parse commandline arguments.
It follows two design principles:

* **One Definition**: the variable storage, commandline argument names as well as argument attributes are defined together once in a single place.
* **Compile-Time Check**: some inconsistencies are already checked at compile-time.

The general structure of the argument parser is show below.

```cpp
auto parser = dacr::ArgumentParser(
    <required-program-description-as-string>,
    dacr::Arg<ArgName1, ArgType1>(ArgAttributes1...),
    dacr::Arg<ArgName2, ArgType2>(ArgAttributes2...),
    ...
    dacr::Arg<ArgNameN, ArtTypeN>(ArgAttributesN...)
);
```

Argument names must not contain any whitespace characters.
After the parser is defined, the commandline arguments passed to the program are parsed with:

```cpp
auto args = parser.parse(argc, argv);
```

The arguments `argc` and `argv` refer to the variables passed to the `main` function.
The result of the `parse` function is a `NamedTuple` with the following structure:

```cpp
using ParseResult = dacr::NamedTuple<
    dacr::Field<ArgName1, ArgType1>,
    dacr::Field<ArgName2, ArgType2>,
    ...
    dacr::Field<ArgName3, ArgType3>
>;
```

## Argument Attributes

The `ArgumentParser` supports the following argument attributes:

| Attribute | Description | Usage |
|:---------:|:------------|:------|
| `help` | The description for the argument. | `help("<help_text>")` |
| `mnemonic` | Short variable name with a single dash (`-<mnemonic>`). The short variable name must not contain whitespace characters. | `mnemonic("<short-variable>")` |
| `optional` | A default value for the argument. | `optional(<default_value>)` |
| `positional` | An argument with a leading dash. | `positional()` |
| `store` | The value to set for switch arguments if provided on the commandline. The default value is the negation then. | `store(true|false)` |
| `required` | The attribute is required. | `required()` |

All non-positional arguments may be referred to with either `--<arg-name>` or `-<mnemonic>`.
The `mnemonic`s must not be single characters, such that `-os` is a valid mnemonic for a longer argument name.


## Argument Types

The `ArgumentParser` supports the following arguments:

* Ordinary Arguments
* Optional Arguments
* Switch Arguments
* N-Ary Arguments


### Ordinary Arguments

Ordinary arguments are defined by:

```cpp
Arg<Name, Type>(attributes...)
```

The supported types are arithmetic types (`char`, `int`, `long int`, `float`, `double`) and class-types having a string-based construction.
The supported attributes are `help`, `mnemonic`, `optional` and `positional`.
The `required` attribute is not supported, but ordinary arguments with an `optional` attribute are implicitly required `required`.

#### Example

```cpp
Arg<"threshold", int>(required(), optional(10)))
```


### Optional Arguments

Optional arguments are defined by:

```cpp
Arg<Name, std::optional<Type>>(attributes...)
```

The supported types are the same as for ordinary attributes.
The supported attributes are `help` and `mnemonic`.

### Example

```cpp
Arg<"optional-value", std::optional<double>>(
    mnemonic("ov"),
    help("description")
)
```


### Switch Arguments

Switch arguments are defined by:

```cpp
Arg<Name, bool>(attributes...)
```

The supported attributes are `help`, `mnemonic` and `store`.

#### Example

```cpp
Arg<"optional-value", std::optional<double>>(
    mnemonic("ov"),
    help("description")
)
```


### N-Ary Arguments

N-ary arguments are defined by:

```cpp
Arg<Name, std::vector<Type>>(attributes...)
```

The supported types are the same as for ordinary arguments.
The supported attributes are `help`, `mnemonic`, `positional` and `required`.
N-ary arguments marked as `positional` are implicitly assumed `required`.

#### Example

```cpp
Arg<"list-argument", std::vector<int>>(positional())
```


## Argument Summary

| Argument Type | Supported Types | Supported Attributes |
|:-------------:|:----------------|:---------------------|
| Ordinary | `char`, `int`, `long int`, `float`, `double`, `<Class>{string}` | `help`, `mnemonic`, `optional`, `positional` |
| Optional | `std::optional` of `char`, `int`, `long int`, `float`, `double`, `<Class>{string}` | `help`, `mnemonic` |
| Switch | `bool` | `help`, `mnemonic`, `store` |
| N-Ary | `std::vector` of `char`, `int`, `long int`, `float`, `double`, `<Class>{string}` | `help`, `mnemonic`, `positional`, `required` |


## Parsing

The arguments passed to the program are finally parsed with the `parse` function:

```cpp
NamedTuple parse (int argc, char *argv[])
```

This function returns the parsed arguments as a `NamedTuple`.


## Help

The `ArgumentParser` always prints a commandline help in case `--help` or `-h` is specified.
The help output looks like this:

```console

```

