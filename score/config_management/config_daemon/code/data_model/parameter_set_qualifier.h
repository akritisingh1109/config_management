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

#ifndef SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_DATA_MODEL_PARAMETER_SET_QUALIFIER_H
#define SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_DATA_MODEL_PARAMETER_SET_QUALIFIER_H

#include <cstdint>
#include <type_traits>

namespace score
{
namespace config_management
{
namespace config_daemon
{
enum class ParameterSetQualifier : std::uint8_t
{
    kUnqualified,
    kQualified,
    kDefault,
    kModified,
    kQualifying
};

// BUD-02 of the Parameter Set JSON Format Contract budgets the qualifier with 3 octets, i.e. for std::uint8_t.
// Changing the underlying type requires adapting the contract and the buffer_size_calculator.
static_assert(std::is_same_v<std::underlying_type_t<ParameterSetQualifier>, std::uint8_t>,
              "ParameterSetQualifier's underlying type must be std::uint8_t (see BUD-02 of the "
              "Parameter Set JSON Format Contract)");

}  // namespace config_daemon
}  // namespace config_management
}  // namespace score

#endif  // SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_DATA_MODEL_PARAMETER_SET_QUALIFIER_H
