# *******************************************************************************
# Copyright (c) 2025 Contributors to the Eclipse Foundation
#
# See the NOTICE file(s) distributed with this work for additional
# information regarding copyright ownership.
#
# This program and the accompanying materials are made available under the
# terms of the Apache License Version 2.0 which is available at
# https://www.apache.org/licenses/LICENSE-2.0
#
# SPDX-License-Identifier: Apache-2.0
# *******************************************************************************

load("@rules_cc//cc:defs.bzl", "cc_test")
load("@score_baselibs//:bazel/unit_tests.bzl", "cc_unit_test_suites_for_host_and_qnx")
load("@score_baselibs//score/language/safecpp:toolchain_features.bzl", "COMPILER_WARNING_FEATURES")
load(":custom.bzl", "load_custom_targets", "load_custom_test_suites")

cc_test(
    name = "main_function_test",
    testonly = True,
    srcs = [
        "main.cpp",
        "main_test.cpp",
    ],
    features = COMPILER_WARNING_FEATURES,
    tags = ["unit"],
    deps = [
        "//score/config_management/config_daemon/code/app/details:app_for_unit_test",
        "//score/config_management/config_daemon/code/factory/details:factory_impl_for_unit_test",
        "@score_lifecycle//score/launch_manager:mock_application_context_cc",
        "@score_lifecycle//score/launch_manager:mock_lifecycle_cc",
        "@score_lifecycle//score/launch_manager:runapplication_cc",
    ],
)

cc_unit_test_suites_for_host_and_qnx(
    name = "unit_tests",
    cc_unit_tests = [
        ":main_function_test",
    ],
    test_suites_from_sub_packages = [
        "//score/config_management/config_daemon/code/app:unit_tests",
        "//score/config_management/config_daemon/code/factory:unit_tests",
        "//score/config_management/config_daemon/code/fault_event_reporter:unit_tests",
        "//score/config_management/config_daemon/code/data_model:unit_tests",
        "//score/config_management/config_daemon/code/data_model/parameter_set_storage:unit_tests",
        "//score/config_management/config_daemon/code/services:unit_tests",
        "//score/config_management/config_daemon/code/plugins:unit_tests",
    ] + load_custom_test_suites(),
    visibility = ["//score/config_management/config_daemon:__subpackages__"],
)

load_custom_targets()
