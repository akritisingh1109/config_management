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

#ifndef SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_DETAILS_SERVICE_FACTORY_IMPL_H
#define SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_DETAILS_SERVICE_FACTORY_IMPL_H

#include "score/config_management/config_daemon/code/factory/service_factory.h"

namespace score
{
namespace config_management
{
namespace config_daemon
{

class ServiceFactoryImpl final : public ServiceFactory
{
  public:
    ServiceFactoryImpl() = default;
    ~ServiceFactoryImpl() override = default;
    ServiceFactoryImpl(ServiceFactoryImpl&&) = delete;
    ServiceFactoryImpl(const ServiceFactoryImpl&) = delete;

    ServiceFactoryImpl& operator=(ServiceFactoryImpl&&) = delete;
    ServiceFactoryImpl& operator=(const ServiceFactoryImpl&) = delete;

    mw::service::ProvidedServicesContainer CreateInternalConfigProviderService(
        const std::shared_ptr<data_model::IParameterSetCollection> read_only_parameter_data_interface) const override;
    LastUpdatedParameterSetSender CreateLastUpdatedParameterSetSender(
        mw::service::ProvidedServicesContainer& services) override;
    InitialQualifierStateSender CreateInitialQualifierStateSender(
        mw::service::ProvidedServicesContainer& services) override;
};

}  // namespace config_daemon
}  // namespace config_management
}  // namespace score

#endif  // SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_FACTORY_DETAILS_SERVICE_FACTORY_IMPL_H
