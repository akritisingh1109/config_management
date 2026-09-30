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

#include "score/config_management/config_daemon/code/data_model/details/parameterset_collection_impl.h"
#include "score/config_management/config_daemon/code/data_model/details/parameterset_collection_manager_impl.h"
#include "score/config_management/config_daemon/code/data_model/parameter_set_storage/details/parameter_set_storage_impl.h"
#include "score/config_management/config_daemon/code/factory/details/infrastructure_factory_impl.h"
#include "score/config_management/config_daemon/code/fault_event_reporter/details/fault_event_reporter_score_impl.h"
#include "score/config_management/config_daemon/code/json_helper/details/json_helper_impl.h"
#include "score/config_management/config_daemon/code/proxies/fault_event/details/fault_event_proxy_impl.h"
#include "score/mw/service/backend/ara/adaptive_immediate_instantiation_strategy.h"
#include "score/mw/service/proxy_needs_factory.h"
#include "platform/aas/mw/storage/key_value_storage/key_value_storage.h"
#include "score/filesystem/details/standard_filesystem.h"
#include "score/filesystem/factory/filesystem_factory.h"
#include "score/hash/code/core/factory/impl/safe_hash_calculator_factory.h"

#include <score/utility.hpp>
#include <memory>
#include <vector>

namespace score
{
namespace config_management
{
namespace config_daemon
{

namespace
{

class FaultEventProxyPort
{
  public:
    constexpr FaultEventProxyPort() = delete;
    constexpr static auto Get() noexcept
    {
        return "ConfigDaemon/ConfigDaemon_RootSwc/FaultEventInterfaceRPort";
    }
};

}  // namespace

InfrastructureFactoryImpl::InfrastructureFactoryImpl()
    : InfrastructureFactory{}, json_helper_{std::make_shared<common::JsonHelper>()}
{
}

// Suppress AUTOSAR C++14 M8-4-2 rule finding.
// Justification: Function follows the declaration in the header with identical parameter names.
// coverity[autosar_cpp14_m8_4_2_violation]
std::shared_ptr<data_model::IParameterSetCollectionManager>
InfrastructureFactoryImpl::CreateParameterSetCollectionManager(std::vector<std::shared_ptr<IPlugin>>& plugins) const
{
    using KVS = data_model::ParameterSetStorageImpl::KeyValueStorage;
    auto storage = std::make_unique<data_model::ParameterSetStorageImpl>(
        json_helper_->GetJsonSyncedWriter(),
        score::filesystem::FilesystemFactory{}.CreateInstance(),
        std::make_shared<hash::SafeHashCalculatorFactory>(),
        std::make_unique<KVS>(score::cpp::string_view{"ConfigDaemon/ConfigDaemon_RootSwc/PersistencyNvmblockPPort"}),
        json_helper_->GetJsonParser());

    auto primary_collection = std::make_shared<data_model::ParameterSetCollection>();
    return std::make_shared<data_model::ParameterSetCollectionManager>(
        std::move(primary_collection), plugins, std::move(storage));
}

std::shared_ptr<fault_event_reporter::IFaultEventReporter> InfrastructureFactoryImpl::CreateFaultEventReporter() const
{
    using FaultEventProxyType = mw::service::ProxyNeeds<mw::service::Optional<IFaultEventProxy>>;
    using FaultEventStrategy = mw::service::AdaptiveImmediateInstantiationStrategy<IFaultEventProxy,
                                                                                   FaultEventProxy,
                                                                                   FaultEventProxy::AdaptiveProxy,
                                                                                   FaultEventProxyPort>;
    auto fault_event_proxy_type =
        ::score::mw::service::ProxyNeedsFactory<FaultEventProxyType>::Create<FaultEventStrategy>();
    const score::cpp::stop_token empty_token;
    auto fault_event_proxy_container = fault_event_proxy_type.InitiateServiceDiscovery(empty_token).GetProxyContainer();
    auto fault_event_proxy_data = fault_event_proxy_container.Extract<mw::service::Optional<IFaultEventProxy>>();

    return std::make_shared<fault_event_reporter::FaultEventReporter>(std::move(fault_event_proxy_data));
}

}  // namespace config_daemon
}  // namespace config_management
}  // namespace score
