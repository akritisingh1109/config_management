load("@rules_cc//cc:defs.bzl", "cc_library")
load("@score_baselibs//:bazel/unit_tests.bzl", "cc_unit_test_suites_for_host_and_qnx")

alias(
    name = "config_provider_interface",
    actual = "//score/config_management/config_provider/code/config_provider",
    visibility = ["//visibility:public"],
)

alias(
    name = "config_provider_mock",
    actual = "//score/config_management/config_provider/code/config_provider:config_provider_mock",
    visibility = ["//visibility:public"],
)

cc_library(
    name = "config_provider_factory_impl",
    tags = ["FUSA"],
    visibility = ["//visibility:public"],
    deps = select({
        "//platform/aas/config_management/flags:is_mw_com": ["//score/config_management/config_provider/code/config_provider/factory:factory_mw_com"],
        "//conditions:default": ["//score/config_management/config_provider/code/config_provider/factory:factory_socal_r20_11"],
    }),
)

cc_library(
    name = "config_provider_socal_r20_11",
    tags = ["FUSA"],
    visibility = [
        "//platform/aas/config_management/parameter_provider/code/parameter_provider/config_provider_builder:__pkg__",
        "//score/config_management/config_provider:__subpackages__",
    ],
    deps = ["//score/config_management/config_provider/code/config_provider/factory:factory_socal_r20_11"],
)

alias(
    name = "config_provider_socal_r20_11_for_unit_test",
    actual = "//score/config_management/config_provider/code/config_provider/factory:factory_socal_r20_11_for_unit_tests",
    visibility = [
        "//platform/aas/config_management/parameter_provider/code/parameter_provider/config_provider_builder:__pkg__",
        "//score/config_management/config_provider:__subpackages__",
    ],
)

alias(
    name = "config_provider_factory_for_unit_test",
    actual = select({
        "//platform/aas/config_management/flags:is_mw_com": "//score/config_management/config_provider/code/config_provider/factory:factory_mw_com",
        "//conditions:default": "//score/config_management/config_provider/code/config_provider/factory:factory_socal_r20_11_for_unit_tests",
    }),
    visibility = ["//visibility:public"],
)

cc_unit_test_suites_for_host_and_qnx(
    name = "unit_tests",
    test_suites_from_sub_packages = [
        "//score/config_management/config_provider/code/config_provider:unit_tests",
        "//score/config_management/config_provider/code/parameter_set:unit_tests",
        "//score/config_management/config_provider/code/persistency:unit_tests",
        "//score/config_management/config_provider/code/proxies:unit_tests",
    ],
    visibility = ["//score/config_management:__pkg__"],
)

test_suite(
    name = "component_tests",
    tests = [],
    visibility = ["//score/config_management:__pkg__"],
)

alias(
    name = "mock_ecu_blackbox",
    actual = "//score/config_management/config_provider/test/ipnext_com_mock/adaptive_model/machine/mpad_pp",
    visibility = ["//score/config_management:__subpackages__"],
)
