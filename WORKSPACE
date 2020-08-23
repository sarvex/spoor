workspace(name = "spoor")

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "com_google_protobuf",
    commit = "fde7cf7358ec7cd69e8db9be4f1fa6a5c431386a",  # v3.13.0
    remote = "https://github.com/protocolbuffers/protobuf.git",
    shallow_since = "1597443653 -0700",
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

git_repository(
    name = "com_gflags_gflags",
    commit = "f7388c6655e699f777a5a74a3c9880b9cfaabe59",  # 2020-06-16 (>v2.2.2)
    remote = "https://github.com/gflags/gflags.git",
    shallow_since = "1592341863 +0100",
)

git_repository(
    name = "com_google_googletest",
    commit = "703bd9caab50b139428cea1aaff9974ebee5742e",  # v1.10.0
    remote = "https://github.com/google/googletest.git",
    shallow_since = "1570114335 -0400",
)

git_repository(
    name = "com_bazelbuild_bazel",
    commit = "7cfe416c5b702967b63cb2d9e2a2a2aefb8d2cac",  # v3.4.1
    remote = "https://github.com/bazelbuild/bazel.git",
    shallow_since = "1594707848 +0200",
)

git_repository(
    # Dependencies require deviating from the naming convention.
    name = "io_bazel_rules_go",
    commit = "13c17d4aa8c8407049f5644e13c9ace1275d200e",  # v0.23.8
    remote = "https://github.com/bazelbuild/rules_go.git",
    shallow_since = "1597238923 -0400",
)

git_repository(
    # Dependencies require deviating from the naming convention.
    name = "bazel_gazelle",
    commit = "c00612418c4dbc9f3cd35fe71fe1147748048b69",  # v0.21.1
    remote = "https://github.com/bazelbuild/bazel-gazelle",
    shallow_since = "1590680880 -0400",
)

load(
    "@io_bazel_rules_go//go:deps.bzl",
    "go_register_toolchains",
    "go_rules_dependencies",
)

go_rules_dependencies()

go_register_toolchains()

load("@bazel_gazelle//:deps.bzl", "gazelle_dependencies")

gazelle_dependencies()
