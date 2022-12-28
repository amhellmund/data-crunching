load("@bazel_tools//tools/build_defs/cc:action_names.bzl", "ACTION_NAMES")
load("@bazel_tools//tools/cpp:cc_toolchain_config_lib.bzl", "feature", "flag_group", "flag_set", "tool_path")

def _impl(ctx):
    tool_paths = [
        tool_path(
            name = "gcc",
            path = "/usr/bin/clang++",
        ),
        tool_path(
            name = "ld",
            path = "/usr/bin/clang++",
        ),
        tool_path(
            name = "ar",
            path = "/usr/bin/ar",
        ),
        tool_path(
            name = "cpp",
            path = "/bin/false",
        ),
        tool_path(
            name = "gcov",
            path = "/bin/false",
        ),
        tool_path(
            name = "nm",
            path = "/bin/false",
        ),
        tool_path(
            name = "objdump",
            path = "/bin/false",
        ),
        tool_path(
            name = "strip",
            path = "/bin/false",
        ),
    ]
    features = [
        feature(
            name = "default_linker_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = [
                        ACTION_NAMES.cpp_link_executable,
                        ACTION_NAMES.cpp_link_dynamic_library,
                        ACTION_NAMES.cpp_link_nodeps_dynamic_library,
                    ],
                    flag_groups = ([
                        flag_group(
                            flags = [
                                "-g",
                                "-stdlib=libc++",
                                "-fexperimental-library",
                            ],
                        ),
                    ]),
                ),
            ],
        ),
        feature(
            name = "default_compiler_flags",
            enabled = True,
            flag_sets = [
                flag_set(
                    actions = [
                        ACTION_NAMES.cpp_compile
                    ],
                    flag_groups = ([
                        flag_group(
                            flags = [
                                "-g",
                                "-std=c++20",
                                "-stdlib=libc++",
                                "-fexperimental-library",
                            ]
                        )
                    ])
                )
            ]
        )
    ]
    return cc_common.create_cc_toolchain_config_info(
        ctx = ctx,
        cxx_builtin_include_directories = [
            "/usr/include",
            "/usr/lib",
        ],
        features = features,
        toolchain_identifier = "local",
        host_system_name = "local",
        target_system_name = "local",
        target_cpu = "x86_64",
        target_libc = "unknown",
        compiler = "clang",
        abi_version = "unknown",
        abi_libc_version = "unknown",
        tool_paths = tool_paths,
    )

cc_toolchain_config = rule(
    implementation = _impl,
    attrs = {},
    provides = [CcToolchainConfigInfo],
)
