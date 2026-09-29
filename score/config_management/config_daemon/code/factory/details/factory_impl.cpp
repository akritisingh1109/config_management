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
#include "score/config_management/config_daemon/code/factory/details/infrastructure_factory_impl.h"
#include "score/config_management/config_daemon/code/factory/details/service_factory_impl.h"
#include "score/config_management/config_daemon/code/plugins/plugin_collector/details/plugin_collector_impl.h"

#include <memory>

namespace score
{
namespace config_management
{
namespace config_daemon
{

FactoryImpl::FactoryImpl()
    : Factory{},
      service_factory_{std::make_unique<ServiceFactoryImpl>()},
      infrastructure_factory_{std::make_unique<InfrastructureFactoryImpl>()}
{
}

mw::service::ProvidedServicesContainer FactoryImpl::CreateInternalConfigProviderService(
    const std::shared_ptr<data_model::IParameterSetCollection> read_only_parameter_data_interface) const
{
    return service_factory_->CreateInternalConfigProviderService(read_only_parameter_data_interface);
}

LastUpdatedParameterSetSender FactoryImpl::CreateLastUpdatedParameterSetSender(
    mw::service::ProvidedServicesContainer& services)
{
    return service_factory_->CreateLastUpdatedParameterSetSender(services);
}

InitialQualifierStateSender FactoryImpl::CreateInitialQualifierStateSender(
    mw::service::ProvidedServicesContainer& services)
{
    return service_factory_->CreateInitialQualifierStateSender(services);
}

std::shared_ptr<data_model::IParameterSetCollectionManager> FactoryImpl::CreateParameterSetCollectionManager(
    std::vector<std::shared_ptr<IPlugin>>& plugins) const
{
    return infrastructure_factory_->CreateParameterSetCollectionManager(plugins);
}

std::shared_ptr<fault_event_reporter::IFaultEventReporter> FactoryImpl::CreateFaultEventReporter() const
{
    return infrastructure_factory_->CreateFaultEventReporter();
}

std::unique_ptr<IPluginCollector> FactoryImpl::CreatePluginCollector() const
{
    return std::make_unique<PluginCollector>();
}

}  // namespace config_daemon
}  // namespace config_management
}  // namespace score
