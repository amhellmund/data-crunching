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
using dacr::makeNamedTuple;

TEST(NamedTuple, NamedTupleEnd2End) {
    auto namedtuple = makeNamedTuple<"int", "dbl">(10, 20.0);
    EXPECT_TRUE((std::is_same_v<
        decltype(namedtuple),
        NamedTuple<Field<"int", int>, Field<"dbl", double>>
    >));

    EXPECT_EQ(namedtuple.get<"int">(), 10);
    EXPECT_EQ(namedtuple.get<"dbl">(), 20.0);

    EXPECT_NO_THROW(namedtuple.set<"int">(100));
    EXPECT_NO_THROW(namedtuple.set<"dbl">(200.0));

    EXPECT_EQ(namedtuple.get<"int">(), 100);
    EXPECT_EQ(namedtuple.get<"dbl">(), 200.0);
}