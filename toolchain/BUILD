package(default_visibility = ["//visibility:public"])

cc_toolchain_suite(
    name = "clang_suite",
    toolchains = {
        "x86_64": ":x86_64_toolchain",
    },
)

filegroup(name = "empty")

cc_toolchain(
    name = "x86_64_toolchain",
    toolchain_identifier = "x86_64_toolchain",
    toolchain_config = ":x86_64_toolchain_config",
    all_files = ":empty",
    compiler_files = ":empty",
    dwp_files = ":empty",
    linker_files = ":empty",
    objcopy_files = ":empty",
    strip_files = ":empty",
    supports_param_files = 0,
)

load(":clang_x86_64_toolchain_config.bzl", "cc_toolchain_config")
cc_toolchain_config(name="x86_64_toolchain_config")