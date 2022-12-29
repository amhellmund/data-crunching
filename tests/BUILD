cc_test(
    name = "test_dataframe",
    srcs = [
        "dataframe.test.cpp",
    ],
    deps = [
        "//:data_crunching",
        "@googletest//:gtest_main",
    ],
)

cc_test(
    name = "test_internal",
    srcs = [
        "internal/column.test.cpp",
        "internal/dataframe.test.cpp",
        "internal/fixed_string.test.cpp",
        "internal/name_list.test.cpp",
        "internal/utils.test.cpp",
    ],
    deps = [
        "//:data_crunching",
        "@googletest//:gtest_main",
    ],
)