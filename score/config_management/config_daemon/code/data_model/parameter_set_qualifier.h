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
    kQualifying  // Must remain the enumerator with the highest value, see the static_assert below.
};

// BUD-02 of the Parameter Set JSON Format Contract budgets the qualifier with 1 octet, i.e. its numeric underlying
// value is always a single decimal digit (0..9). kQualifying is the enumerator with the highest value, so asserting
// on it covers all enumerators. Adding an enumerator beyond 9 requires adapting the contract (BUD-02),
// QUALIFIER_WORST_CASE_LENGTH of the buffer_size_calculator and ParameterSetJsonSerializerImpl::SerializeParameterSet.
static_assert(static_cast<std::underlying_type_t<ParameterSetQualifier>>(ParameterSetQualifier::kQualifying) <= 9U,
              "ParameterSetQualifier values must be single decimal digits (see BUD-02 of the "
              "Parameter Set JSON Format Contract)");

}  // namespace config_daemon
}  // namespace config_management
}  // namespace score

#endif  // SCORE_CONFIG_MANAGEMENT_CONFIG_DAEMON_CODE_DATA_MODEL_PARAMETER_SET_QUALIFIER_H
