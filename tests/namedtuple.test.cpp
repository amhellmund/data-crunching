// Copyright 2022 Andi Hellmund
//
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//  See the License for the specific language governing permissions and
//  limitations under the License.

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <type_traits>

#include "data_crunching/namedtuple.hpp"

using dacr::Field;
using dacr::NamedTuple;
using dacr::internal::GetNamesFromFields;

TEST(NamedTuple, NamedTupleConstructionUsingFieldLiterals) {
    auto namedtuple = NamedTuple("int"_field = 10, "dbl"_field = 20.0);
    EXPECT_TRUE((std::is_same_v<
        decltype(namedtuple),
        NamedTuple<Field<"int", int>, Field<"dbl", double>>
    >));

    EXPECT_EQ(namedtuple.get<"int">(), 10);
    EXPECT_EQ(namedtuple.get<"dbl">(), 20.0);

    EXPECT_NO_THROW((namedtuple.get<"int">() = 100));
    EXPECT_NO_THROW((namedtuple.get<"dbl">() = 200.0));

    EXPECT_EQ(namedtuple.get<"int">(), 100);
    EXPECT_EQ(namedtuple.get<"dbl">(), 200.0);
}

TEST(NamedTuple, NamedTupleConstructionDacrField) {
    auto namedtuple = NamedTuple(dacr_field("int") = 10, dacr_field("dbl") = 20.0);
    EXPECT_EQ(namedtuple.get<"int">(), 10);
    EXPECT_EQ(namedtuple.get<"dbl">(), 20.0);
}

TEST(NamedTuple, NamedTupleConstructionExplicit) {
    NamedTuple<dacr::Field<"int", int>, dacr::Field<"dbl", double>> namedtuple{10, 20.0};
    EXPECT_EQ(namedtuple.get<"int">(), 10);
    EXPECT_EQ(namedtuple.get<"dbl">(), 20.0);
}

struct CustomData {
    int i;
    double d;
};

TEST(NamedTuple, StructuredBindings) {
    auto tuple = dacr::NamedTuple("i"_field = 10, "custom"_field = CustomData{100, 200.0});
    auto& [i, custom] = tuple;

    i = 1;
    custom.i = 2;
    custom.d = 3.0;

    EXPECT_EQ(tuple.get<"i">(), 1);
    EXPECT_EQ(tuple.get<"custom">().i, 2);
    EXPECT_DOUBLE_EQ(tuple.get<"custom">().d, 3.0);
}

TEST(NamedTuple, StructuredBindingsConst) {
    auto tuple = dacr::NamedTuple("i"_field = 10, "custom"_field = CustomData{100, 200.0});
    const auto& [i, custom] = tuple;

    EXPECT_EQ(i, 10);
    EXPECT_EQ(custom.i, 100);
    EXPECT_DOUBLE_EQ(custom.d, 200.0);
}

TEST(NamedTuple, StructuredBindingsMove) {
    auto tuple = dacr::NamedTuple("i"_field = 10, "custom"_field = CustomData{100, 200.0});
    auto&& [i, custom] = std::move(tuple);

    auto moved_to_i = std::move(i);
    auto moved_to_custom = std::move(custom);

    EXPECT_EQ(moved_to_i, 10);
    EXPECT_EQ(moved_to_custom.i, 100);
    EXPECT_DOUBLE_EQ(moved_to_custom.d, 200.0);
}
