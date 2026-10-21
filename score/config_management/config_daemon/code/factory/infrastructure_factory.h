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

#ifndef SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_INFRASTRUCTURE_FACTORY_H
#define SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_INFRASTRUCTURE_FACTORY_H

#include "score/config_management/config_daemon/code/data_model/parameterset_collection_manager.h"
#include "score/config_management/config_daemon/code/fault_event_reporter/fault_event_reporter.h"
#include "score/config_management/config_daemon/code/plugins/plugin.h"

#include <memory>
#include <vector>

namespace score
{
namespace config_management
{
namespace config_daemon
{

class InfrastructureFactory
{
  public:
    InfrastructureFactory() = default;
    virtual ~InfrastructureFactory() = default;
    InfrastructureFactory(InfrastructureFactory&&) = delete;
    InfrastructureFactory(const InfrastructureFactory&) = delete;

    InfrastructureFactory& operator=(InfrastructureFactory&&) = delete;
    InfrastructureFactory& operator=(const InfrastructureFactory&) = delete;

    virtual std::shared_ptr<data_model::IParameterSetCollectionManager> CreateParameterSetCollectionManager(
        std::vector<std::shared_ptr<IPlugin>>& plugins) const = 0;
    virtual std::shared_ptr<fault_event_reporter::IFaultEventReporter> CreateFaultEventReporter() const = 0;
};

}  // namespace config_daemon
}  // namespace config_management
}  // namespace score

#endif  // SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_INFRASTRUCTURE_FACTORY_H
