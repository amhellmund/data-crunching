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

using namespace dacr;
using namespace dacr::internal;

TEST(NamedTupleInternal, GetNamesFromFields) {
    EXPECT_TRUE((std::is_same_v<
        GetNamesFromFields<Field<"int", int>, Field<"dbl", double>>,
        NameList<"int", "dbl">
    >));
}

TEST(NamedTupleInternal, GetTypesFromFields) {
    EXPECT_TRUE((std::is_same_v<
        GetTypesFromFields<Field<"int", int>, Field<"dbl", double>>,
        TypeList<int, double>
    >));
}

TEST(NamedTupleInternal, GetFieldIndexByName) {
    EXPECT_EQ((get_field_index_by_name<"int", Field<"int", int>, Field<"dbl", double>>), 0);
    EXPECT_EQ((get_field_index_by_name<"dbl", Field<"int", int>, Field<"dbl", double>>), 1);
} 

TEST(NamedTupleInternal, GetTypeFromFieldsByIndex) {
    EXPECT_TRUE((std::is_same_v<
        GetTypeFromFieldsByIndex<0, Field<"int", int>, Field<"dbl", double>>,
        int
    >));

    EXPECT_TRUE((std::is_same_v<
        GetTypeFromFieldsByIndex<1, Field<"int", int>, Field<"dbl", double>>,
        double
    >));
}