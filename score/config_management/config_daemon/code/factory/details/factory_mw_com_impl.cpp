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

#include "score/config_management/config_daemon/code/factory/details/service_factory_impl.h"
#include "score/config_management/config_daemon/code/services/details/internal_config_provider_service_reactor_impl.h"
#include "score/config_management/config_daemon/code/services/details/mw_com/internal_config_provider_service_impl.h"
#include "score/mw/log/logging.h"

#include <score/utility.hpp>
#include <memory>

namespace score
{
namespace config_management
{
namespace config_daemon
{

namespace
{

const auto kICPServiceInstanceSpecifierName =
    score::mw::com::InstanceSpecifier::Create(
        std::string("ConfigDaemon/ConfigDaemon_RootSwc/InternalConfigProviderAppPPort"))
        .value();
}  // namespace

mw::service::ProvidedServicesContainer ServiceFactoryImpl::CreateInternalConfigProviderService(
    const std::shared_ptr<data_model::IParameterSetCollection> read_only_parameter_data_interface) const
{
    auto service_reactor =
        std::make_unique<InternalConfigProviderServiceReactorImpl>(read_only_parameter_data_interface);

    mw::service::ProvidedServicesContainer services{};
    auto skeleton_result = InternalConfigProviderSkeleton::Create(kICPServiceInstanceSpecifierName);
    if (skeleton_result.has_value())
    {
        const auto emplace_result = services.Emplace<InternalConfigProviderService>(
            "InternalConfigProviderService", std::move(service_reactor), std::move(skeleton_result).value());
        if (!emplace_result.has_value())
        {
            mw::log::LogError() << "ServiceFactoryImpl::" << __func__
                                << ": Failed to start InternalConfigProviderService:" << emplace_result.error();
        }
    }
    else
    {
        mw::log::LogError() << "ServiceFactoryImpl::" << __func__
                            << ": Failed to create InternalConfigProviderSkeleton:" << skeleton_result.error();
    }
    return services;
}

LastUpdatedParameterSetSender ServiceFactoryImpl::CreateLastUpdatedParameterSetSender(
    mw::service::ProvidedServicesContainer& services)
{
    if (auto* const service = services.Get<IInternalConfigProviderService>(); service != nullptr)
    {
        return [service](const std::string_view parameter_set_name) noexcept -> bool {
            return service->SendLastUpdatedParameterSet(parameter_set_name);
        };
    }
    return [](const std::string_view) noexcept -> bool {
        return false;
    };
}

InitialQualifierStateSender ServiceFactoryImpl::CreateInitialQualifierStateSender(
    mw::service::ProvidedServicesContainer& services)
{
    if (auto* const service = services.Get<IInternalConfigProviderService>(); service != nullptr)
    {
        return [service](const config_daemon::InitialQualifierState initial_qualifier_state) noexcept -> void {
            service->SetInitialQualifierState(initial_qualifier_state);
        };
    }
    return [](const config_daemon::InitialQualifierState) noexcept -> void {};
}

}  // namespace config_daemon
}  // namespace config_management
}  // namespace score
