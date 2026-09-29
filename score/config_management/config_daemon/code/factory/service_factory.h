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

#ifndef SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_SERVICE_FACTORY_H
#define SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_SERVICE_FACTORY_H

#include "score/config_management/config_daemon/code/data_model/parameterset_collection.h"
#include "score/config_management/config_daemon/code/services/internal_config_provider_service.h"
#include "score/mw/service/provided_services_container.h"

#include <memory>

namespace score
{
namespace config_management
{
namespace config_daemon
{

class ServiceFactory
{
  public:
    ServiceFactory() = default;
    virtual ~ServiceFactory() = default;
    ServiceFactory(ServiceFactory&&) = delete;
    ServiceFactory(const ServiceFactory&) = delete;

    ServiceFactory& operator=(ServiceFactory&&) = delete;
    ServiceFactory& operator=(const ServiceFactory&) = delete;

    virtual mw::service::ProvidedServicesContainer CreateInternalConfigProviderService(
        const std::shared_ptr<data_model::IParameterSetCollection> read_only_parameter_data_interface) const = 0;
    virtual LastUpdatedParameterSetSender CreateLastUpdatedParameterSetSender(
        mw::service::ProvidedServicesContainer& services) = 0;
    virtual InitialQualifierStateSender CreateInitialQualifierStateSender(
        mw::service::ProvidedServicesContainer& services) = 0;
};

}  // namespace config_daemon
}  // namespace config_management
}  // namespace score

#endif  // SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_SERVICE_FACTORY_H
