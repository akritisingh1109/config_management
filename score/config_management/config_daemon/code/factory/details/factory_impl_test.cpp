// *******************************************************************************
// Copyright (c) 2025 Contributors to the Eclipse Foundation
//
// See the NOTICE file(s) distributed with this work for additional
// information regarding copyright ownership.
//
// This program and the accompanying materials are made available under the
// terms of the Apache License Version 2.0 which is available at
// https://www.apache.org/licenses/LICENSE-2.0
//
// SPDX-License-Identifier: Apache-2.0
// *******************************************************************************

#include "score/config_management/config_daemon/code/factory/details/factory_impl.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <score/utility.hpp>
#include <memory>

namespace score
{
namespace config_management
{
namespace config_daemon
{
namespace test
{
namespace
{

class TestFactoryImpl : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        unit_ = std::make_unique<FactoryImpl>();
    }

    std::unique_ptr<FactoryImpl> unit_;
};

TEST_F(TestFactoryImpl, CreatePluginCollector)
{
    RecordProperty("Priority", "3");
    RecordProperty("DerivationTechnique", "Analysis of equivalence classes and boundary values");
    RecordProperty("TestType", "Interface test");
    RecordProperty("Verifies", "::score::config_management::config_daemon::FactoryImpl::CreatePluginCollector()");
    RecordProperty("Description",
                   "This test ensures that CreatePluginCollector() returns a valid PluginCollector object. Unlike "
                   "every other FactoryImpl method, this one is not delegated to ServiceFactory/InfrastructureFactory "
                   "and is therefore the only behavior genuinely owned by FactoryImpl itself.");

    // When calling CreatePluginCollector()
    const auto plugin_collector = unit_->CreatePluginCollector();

    // Then the returned plugin_collector instance must be valid
    ASSERT_NE(nullptr, plugin_collector) << "FactoryImpl did not return a valid PluginCollector object";
}

}  // namespace
}  // namespace test
}  // namespace config_daemon
}  // namespace config_management
}  // namespace score
