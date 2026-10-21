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

#ifndef SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_DETAILS_INFRASTRUCTURE_FACTORY_IMPL_H
#define SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_DETAILS_INFRASTRUCTURE_FACTORY_IMPL_H

#include "score/config_management/config_daemon/code/factory/infrastructure_factory.h"
#include "score/config_management/config_daemon/code/json_helper/json_helper.h"

namespace score
{
namespace config_management
{
namespace config_daemon
{

class InfrastructureFactoryImpl final : public InfrastructureFactory
{
  public:
    InfrastructureFactoryImpl();
    ~InfrastructureFactoryImpl() override = default;
    InfrastructureFactoryImpl(InfrastructureFactoryImpl&&) = delete;
    InfrastructureFactoryImpl(const InfrastructureFactoryImpl&) = delete;

    InfrastructureFactoryImpl& operator=(InfrastructureFactoryImpl&&) = delete;
    InfrastructureFactoryImpl& operator=(const InfrastructureFactoryImpl&) = delete;

    std::shared_ptr<data_model::IParameterSetCollectionManager> CreateParameterSetCollectionManager(
        std::vector<std::shared_ptr<IPlugin>>& plugins) const override;
    std::shared_ptr<fault_event_reporter::IFaultEventReporter> CreateFaultEventReporter() const override;

  private:
    std::shared_ptr<common::IJsonHelper> json_helper_;
};

}  // namespace config_daemon
}  // namespace config_management
}  // namespace score

#endif  // SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_DETAILS_INFRASTRUCTURE_FACTORY_IMPL_H
