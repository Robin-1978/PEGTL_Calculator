COPTS = [
    "-std=c++17",
]
LINK_OPTS = []

# json
cc_library(
    name = "pegtl",
    hdrs = glob([
        "include/**/*.hpp",
    ]),
    includes = [
        "include",
    ],
    copts = COPTS,
    linkopts = LINK_OPTS,
    deps = [
    ],
    visibility = ["//visibility:public"],
)