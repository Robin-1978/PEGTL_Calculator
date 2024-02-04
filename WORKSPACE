workspace(name = "geo")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "rules_foreign_cc",
    sha256 = "bcd0c5f46a49b85b384906daae41d277b3dc0ff27c7c752cc51e43048a58ec83",
    strip_prefix = "rules_foreign_cc-0.7.1",
    url = "https://github.com/bazelbuild/rules_foreign_cc/archive/0.7.1.tar.gz",
)

load("@rules_foreign_cc//foreign_cc:repositories.bzl", "rules_foreign_cc_dependencies")

# This sets up some common toolchains for building targets. For more details, please see
# https://bazelbuild.github.io/rules_foreign_cc/0.7.1/flatten.html#rules_foreign_cc_dependencies
rules_foreign_cc_dependencies()

http_archive(
    name = "com_github_fmt",
    build_file = "//:build_files/fmt.BUILD",
    sha256 = "78b8c0a72b1c35e4443a7e308df52498252d1cefc2b08c9a97bc9ee6cfe61f8b",
    strip_prefix = "fmt-10.1.1",
    urls = ["https://github.com/fmtlib/fmt/archive/refs/tags/10.1.1.tar.gz"],
)

http_archive(
    name = "com_github_pegtl",
    build_file = "//:build_files/pegtl.BUILD",
    sha256 = "d6cd113d8bd14e98bcbe7b7f8fc1e1e33448dc359e8cd4cca30e034ec2f0642d",
    strip_prefix = "PEGTL-3.2.7",
    urls = ["https://github.com/taocpp/PEGTL/archive/refs/tags/3.2.7.tar.gz"],
)
