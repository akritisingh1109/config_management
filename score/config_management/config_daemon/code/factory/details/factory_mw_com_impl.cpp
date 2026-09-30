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
#include "score/mw/service/backend/mw_com/provided_service_builder.h"
#include "score/mw/service/backend/mw_com/provided_service_decorator.h"

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

mw::service::ProvidedServiceContainer ServiceFactoryImpl::CreateInternalConfigProviderService(
    const std::shared_ptr<data_model::IParameterSetCollection> read_only_parameter_data_interface) const
{
    mw::service::ProvidedServiceBuilder builder{};
    auto service_reactor =
        std::make_unique<InternalConfigProviderServiceReactorImpl>(read_only_parameter_data_interface);

    score::Result<InternalConfigProviderService> icp_creation_result =
        InternalConfigProviderService::Create(std::move(service_reactor), kICPServiceInstanceSpecifierName);

    if (icp_creation_result.has_value())
    {
        score::cpp::ignore = builder.With<InternalConfigProviderService>(std::move(icp_creation_result).value());
    }
    else
    {
        mw::log::LogError() << "ServiceFactoryImpl::" << __func__
                            << ": Failed to create InternalConfigProviderService:" << icp_creation_result.error();
    }
    return builder.GetServices();
}

LastUpdatedParameterSetSender ServiceFactoryImpl::CreateLastUpdatedParameterSetSender(
    mw::service::ProvidedServiceContainer& services)
{
    auto* const provided_service_container =
        services.GetServices<mw::service::backend::mw_com::ProvidedServiceBuilder::DecoratorType>();
    if ((provided_service_container != nullptr) && provided_service_container->Has<IInternalConfigProviderService>())
    {
        return [internal_config_provider_service{provided_service_container->Get<IInternalConfigProviderService>()}](
                   const std::string_view parameter_set_name) noexcept -> bool {
            return internal_config_provider_service->SendLastUpdatedParameterSet(parameter_set_name);
        };
    }
    // Not fatal: the underlying service backend may not (yet) provide a real implementation
    // (e.g. stub-only mw::service). Return a no-op callback instead of an empty one so callers
    // relying on a valid callback keep working.
    mw::log::LogWarn() << "ServiceFactoryImpl::" << __func__
                       << ": InternalConfigProviderService unavailable, using no-op callback";
    return [](const std::string_view) noexcept -> bool {
        return false;
    };
}

InitialQualifierStateSender ServiceFactoryImpl::CreateInitialQualifierStateSender(
    mw::service::ProvidedServiceContainer& services)
{
    auto* const provided_service_container =
        services.GetServices<mw::service::backend::mw_com::ProvidedServiceBuilder::DecoratorType>();
    if ((provided_service_container != nullptr) && provided_service_container->Has<IInternalConfigProviderService>())
    {
        return [internal_config_provider_service{provided_service_container->Get<IInternalConfigProviderService>()}](
                   const config_daemon::InitialQualifierState initial_qualifier_state) noexcept -> void {
            internal_config_provider_service->SetInitialQualifierState(initial_qualifier_state);
        };
    }
    // Not fatal: see comment above.
    mw::log::LogWarn() << "ServiceFactoryImpl::" << __func__
                       << ": InternalConfigProviderService unavailable, using no-op callback";
    return [](const config_daemon::InitialQualifierState) noexcept -> void {};
}

}  // namespace config_daemon
}  // namespace config_management
}  // namespace score
