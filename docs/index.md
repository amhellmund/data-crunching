# Data Crunching for C++

The analysis (or _crunching_) of multi-dimensional, semantically grouped data is typically performed by database systems (relational, column-store, schema-free).
Dynamically-typed languages like Python introduced feature-rich frameworks (e.g. pandas) with an easy-to-use syntax to perform complex data analysis operations with a small amount of code.
While dynamically-typed languages have some advantages about statically-compiled languages in terms of _flexibility_, they often lack premature (type) correctness checks such that operations fail at runtime.

The goal of this project to a C++ data analysis framework that combines the two worlds of syntatically easy-to-use API with operation correctness checks at compile-time.
The project offer s the following main APIs:

| API | Description | Link |
|:---:|:------------|:-----|
| `DataFrame` | In-Memory Column-Store for Data Analysis | [Documentation](api/dataframe.md) |
| `NamedTuple` | Tuple Dataclass with String-Based Member Access | [Documentation](api/namedtuple.md) |