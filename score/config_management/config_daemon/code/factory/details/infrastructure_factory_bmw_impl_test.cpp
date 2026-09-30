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
#include "score/config_management/config_daemon/code/proxies/fault_event/details/fault_event_proxy_impl.h"

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

using ::testing::_;
using ::testing::Exactly;
using ::testing::Invoke;

class TestInfrastructureFactoryBmwImpl : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        unit_ = std::make_unique<InfrastructureFactoryImpl>();
    }
    template <typename P>
    void SetupTestEnvironmentForProxy(mw::core::InstanceSpecifier instance_specifier);
    template <typename P>
    void TearOffTestEnvironmentForProxy();

    std::unique_ptr<InfrastructureFactoryImpl> unit_;
};

template <typename P>
void TestInfrastructureFactoryBmwImpl::SetupTestEnvironmentForProxy(mw::core::InstanceSpecifier instance_specifier)
{
    P::CreateFindProxy();
    EXPECT_CALL(*(P::GetFindProxyInstance()), StartFindService(_, instance_specifier))
        .Times(Exactly(1))
        .WillOnce(Invoke([&](auto&& callback, auto&& /*instance_specifier*/) {
            typename P::HandleType proxy_handle{mw::com::InstanceIdentifier{{}}};
            callback({{proxy_handle}});
            return mw::com::FindServiceHandle{};
        }));
}

template <typename P>
void TestInfrastructureFactoryBmwImpl::TearOffTestEnvironmentForProxy()
{
    P::DestroyFindProxy();
}

TEST_F(TestInfrastructureFactoryBmwImpl, Destruction)
{
    RecordProperty("Priority", "3");
    RecordProperty("DerivationTechnique", "Analysis of equivalence classes and boundary values");
    RecordProperty("TestType", "Interface test");
    RecordProperty("Verifies", "::score::config_management::config_daemon::InfrastructureFactoryImpl::~InfrastructureFactoryImpl");
    RecordProperty("Description",
                   "Verifies that InfrastructureFactoryImpl can be destroyed without throwing and cleans up without "
                   "leaks.");

    // Given an instance via base-class pointer
    std::unique_ptr<InfrastructureFactory> factory_base = std::make_unique<InfrastructureFactoryImpl>();
    // When being reset, Then no exception is thrown
    EXPECT_NO_THROW(factory_base.reset());

    // Given an instance via direct pointer
    auto factory_impl = std::make_unique<InfrastructureFactoryImpl>();
    // When being reset, Then no exception is thrown
    EXPECT_NO_THROW(factory_impl.reset());

    // Given an instance via object on stack; When scope exits, Then no exception is thrown
    EXPECT_NO_THROW([]() {
        InfrastructureFactoryImpl factory;
        score::cpp::ignore = factory;
    }());
}

TEST_F(TestInfrastructureFactoryBmwImpl, CreateParameterSetCollectionManager)
{
    RecordProperty("Priority", "3");
    RecordProperty("DerivationTechnique", "Analysis of equivalence classes and boundary values");
    RecordProperty("TestType", "Interface test");
    RecordProperty("Verifies",
                   "::score::config_management::config_daemon::InfrastructureFactoryImpl::CreateParameterSetCollectionManager()");
    RecordProperty("Description",
                   "This test ensures that CreateParameterSetCollectionManager() returns a valid "
                   "ParameterSetCollectionManager object");

    // When calling CreateParameterSetCollectionManager()
    std::vector<std::shared_ptr<IPlugin>> plugins{};
    const auto parameter_data = unit_->CreateParameterSetCollectionManager(plugins);

    // Then the returned parameter_data instance must be valid
    auto* proxy_impl = dynamic_cast<data_model::IParameterSetCollectionManager*>(parameter_data.get());
    ASSERT_NE(nullptr, proxy_impl) << "InfrastructureFactoryImpl did not return a valid parameter_data object";
}

TEST_F(TestInfrastructureFactoryBmwImpl, CreateFaultEventReporter)
{
    RecordProperty("Priority", "3");
    RecordProperty("DerivationTechnique", "Analysis of equivalence classes and boundary values");
    RecordProperty("TestType", "Interface test");
    RecordProperty("Verifies", "::score::config_management::config_daemon::InfrastructureFactoryImpl::CreateFaultEventReporter()");
    RecordProperty("Description",
                   "This test ensures that CreateFaultEventReporter() returns a valid FaultEventReporter object");

    using AraFaultEventProxy = FaultEventProxy::AdaptiveProxy;
    SetupTestEnvironmentForProxy<AraFaultEventProxy>(
        mw::core::InstanceSpecifier{"ConfigDaemon/ConfigDaemon_RootSwc/FaultEventInterfaceRPort"});

    // When calling CreateFaultEventReporter()
    const auto reporter = unit_->CreateFaultEventReporter();

    // Then the returned fault_event_reporter instance must be valid
    auto* proxy_impl = dynamic_cast<fault_event_reporter::FaultEventReporter*>(reporter.get());
    ASSERT_NE(nullptr, proxy_impl) << "InfrastructureFactoryImpl did not return a valid FaultEventReporter object";

    TearOffTestEnvironmentForProxy<AraFaultEventProxy>();
}

}  // namespace
}  // namespace test
}  // namespace config_daemon
}  // namespace config_management
}  // namespace score
