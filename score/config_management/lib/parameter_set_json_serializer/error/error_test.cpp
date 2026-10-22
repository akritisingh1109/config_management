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

#include <gtest/gtest.h>

namespace score
{
namespace config_management
{
namespace
{

void TestMessage(const ParameterSetJsonSerializerError error, const char* const message)
{
    EXPECT_EQ(MakeError(error).Message(), message);
}

TEST(ParameterSetJsonSerializerError, CanConvertToString)
{
    RecordProperty("ASIL", "B");
    RecordProperty("TestType", "Interface test");
    RecordProperty("Description", "Verifies that MakeError method will create error with correct code and message");
    RecordProperty("DerivationTechnique", "Error guessing based on knowledge or experience");

    TestMessage(ParameterSetJsonSerializerError::kNonFiniteNumber,
                "Floating point value is not finite and cannot be represented in JSON.");
    TestMessage(ParameterSetJsonSerializerError::kUnsupportedNumberType,
                "Number does not fit any supported arithmetic type.");
    TestMessage(ParameterSetJsonSerializerError::kUnsupportedValueType,
                "Parameter value is neither a number, a boolean nor a list.");
    TestMessage(ParameterSetJsonSerializerError::kQualifierOutOfRange, "Qualifier is not a single decimal digit.");
    TestMessage(static_cast<ParameterSetJsonSerializerError>(0xff), "Unknown Error!");
    TestMessage(static_cast<ParameterSetJsonSerializerError>(-1), "Unknown Error!");
}

}  // namespace
}  // namespace config_management
}  // namespace score
