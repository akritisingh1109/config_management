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

#include "score/config_management/config_daemon/code/factory/details/infrastructure_factory_impl.h"
#include "score/config_management/config_daemon/code/fault_event_reporter/details/fault_event_reporter_score_impl.h"

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

class TestInfrastructureFactoryScoreImpl : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        unit_ = std::make_unique<InfrastructureFactoryImpl>();
    }

    std::unique_ptr<InfrastructureFactoryImpl> unit_;
};

TEST_F(TestInfrastructureFactoryScoreImpl, Destruction)
{
    RecordProperty("Priority", "3");
    RecordProperty("DerivationTechnique", "Analysis of equivalence classes and boundary values");
    RecordProperty("TestType", "Interface test");
    RecordProperty("Verifies", "::score::config_management::config_daemon::InfrastructureFactoryImpl::~InfrastructureFactoryImpl");
    RecordProperty("Description",
                   "Verify that InfrastructureFactoryImpl instances can be destroyed safely without throwing.");

    std::unique_ptr<InfrastructureFactory> factory_base = std::make_unique<InfrastructureFactoryImpl>();
    EXPECT_NO_THROW(factory_base.reset());

    auto factory_impl = std::make_unique<InfrastructureFactoryImpl>();
    EXPECT_NO_THROW(factory_impl.reset());

    EXPECT_NO_THROW([]() {
        InfrastructureFactoryImpl factory;
        score::cpp::ignore = factory;
    }());
}

TEST_F(TestInfrastructureFactoryScoreImpl, CreateParameterSetCollectionManager)
{
    RecordProperty("Priority", "3");
    RecordProperty("DerivationTechnique", "Analysis of equivalence classes and boundary values");
    RecordProperty("TestType", "Interface test");
    RecordProperty("Verifies",
                   "::score::config_management::config_daemon::InfrastructureFactoryImpl::CreateParameterSetCollectionManager()");
    RecordProperty("Description", "Ensure valid ParameterSetCollectionManager is created");

    std::vector<std::shared_ptr<IPlugin>> plugins{};
    const auto parameter_data = unit_->CreateParameterSetCollectionManager(plugins);
    auto* impl = dynamic_cast<data_model::IParameterSetCollectionManager*>(parameter_data.get());
    ASSERT_NE(nullptr, impl);
}

TEST_F(TestInfrastructureFactoryScoreImpl, CreateFaultEventReporter)
{
    RecordProperty("Priority", "3");
    RecordProperty("DerivationTechnique", "Analysis of equivalence classes and boundary values");
    RecordProperty("TestType", "Interface test");
    RecordProperty("Verifies", "::score::config_management::config_daemon::InfrastructureFactoryImpl::CreateFaultEventReporter()");
    RecordProperty("Description",
                   "This test ensures that CreateFaultEventReporter() returns a valid FaultEventReporter object");

    // When calling CreateFaultEventReporter()
    const auto reporter = unit_->CreateFaultEventReporter();

    // Then the returned reporter instance must be valid
    auto* proxy_impl = dynamic_cast<fault_event_reporter::FaultEventReporter*>(reporter.get());
    ASSERT_NE(nullptr, proxy_impl) << "InfrastructureFactoryImpl did not return a valid FaultEventReporter object";
}

}  // namespace
}  // namespace test
}  // namespace config_daemon
}  // namespace config_management
}  // namespace score
