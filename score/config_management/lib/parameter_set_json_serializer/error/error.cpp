// *******************************************************************************
// Copyright (c) 2026 Contributors to the Eclipse Foundation
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

#include "score/config_management/lib/parameter_set_json_serializer/error/error.h"

#include "score/result/error_domain.h"

#include <score/utility.hpp>

namespace score
{
namespace config_management
{

namespace
{

class ParameterSetJsonSerializerErrorDomain final : public score::result::ErrorDomain
{
  public:
    std::string_view MessageFor(const score::result::ErrorCode& code) const noexcept override
    {
        if ((code < score::cpp::to_underlying(ParameterSetJsonSerializerError::kNonFiniteNumber)) ||
            (code > score::cpp::to_underlying(ParameterSetJsonSerializerError::kQualifierOutOfRange)))
        {
            return std::string_view{"Unknown Error!"};
        }

        std::string_view message;
        switch (static_cast<ParameterSetJsonSerializerError>(code))
        {
            case ParameterSetJsonSerializerError::kNonFiniteNumber:
                message = std::string_view{"Floating point value is not finite and cannot be represented in JSON."};
                break;
            case ParameterSetJsonSerializerError::kUnsupportedNumberType:
                message = std::string_view{"Number does not fit any supported arithmetic type."};
                break;
            case ParameterSetJsonSerializerError::kUnsupportedValueType:
                message = std::string_view{"Parameter value is neither a number, a boolean nor a list."};
                break;
            case ParameterSetJsonSerializerError::kQualifierOutOfRange:
                message = std::string_view{"Qualifier is not a single decimal digit."};
                break;
            // LCOV_EXCL_START (Reaching this default case is not possible as range is checked above.)
            default:
                message = std::string_view{"Unknown Error!"};
                break;
                // LCOV_EXCL_STOP
        }
        return message;
    }
};

constexpr ParameterSetJsonSerializerErrorDomain kParameterSetJsonSerializerErrorDomain{};

}  // namespace

score::result::Error MakeError(const ParameterSetJsonSerializerError code, const std::string_view user_message) noexcept
{
    return {static_cast<score::result::ErrorCode>(code), kParameterSetJsonSerializerErrorDomain, user_message};
}

}  // namespace config_management
}  // namespace score
