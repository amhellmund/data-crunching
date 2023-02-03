cc_binary(
    name = "example_argparse",
    srcs = ["argparse.cpp"],
    deps = [
        "//:data_crunching",
    ]
)

exports_files(
    [
        "data/city.csv",
        "data/person.csv",
    ]
)

cc_binary(
    name = "example_dataframe",
    srcs = ["dataframe.cpp"],
    deps = [
        "//:data_crunching",
    ],
    data = [
        "//examples:data/city.csv",
        "//examples:data/person.csv",
    ]
)

cc_binary(
    name = "example_namedtuple",
    srcs = ["namedtuple.cpp"],
    deps = [
        "//:data_crunching",
    ]
)

cc_binary(
    name = "example_string",
    srcs = ["string.cpp"],
    deps = [
        "//:data_crunching",
    ]
)