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

#include "data_crunching/internal/argparse.hpp"

using namespace dacr;
using namespace dacr::internal;

TEST(ArgParseInternal, GetMnemonic) {
    auto mnemonic = getMnemonic(Required{}, Mnemonic{.short_arg = "n"});
    EXPECT_THAT(mnemonic, ::testing::Optional(std::string{"-n"}));

    auto no_mnemonic = getMnemonic(Required{}, Positional{});
    EXPECT_THAT(no_mnemonic, ::testing::Eq(std::nullopt));
}

TEST(ArgParseInternal, GetHelp) {
    auto help = getHelp(Required{}, Help{.text = "help"}, Mnemonic{.short_arg = "n"});
    EXPECT_THAT(help, ::testing::Optional(std::string{"help"}));

    auto no_help = getHelp(Required{}, Positional{});
    EXPECT_THAT(no_help, ::testing::Eq(std::nullopt));
}

TEST(ArgParseInternal, GetOptional) {
    std::optional<int> optional = getOptional(Required{}, Optional<int>{.value = 10}, Mnemonic{.short_arg = "n"});
    EXPECT_THAT(optional, ::testing::Optional(10));

    std::optional<int> no_optional = getOptional(Required{}, Positional{});
    EXPECT_THAT(no_optional, ::testing::Eq(std::nullopt));
}

TEST(ArgParseInternal, GetStore) {
    auto store = getStore(Store{.value = false}, Optional<int>{.value = 10}, Mnemonic{.short_arg = "n"});
    EXPECT_THAT(store, ::testing::Optional(false));

    auto no_store = getStore(Required{}, Positional{});
    EXPECT_THAT(no_store, ::testing::Eq(std::nullopt));
}

TEST(ArgParseInternal, IsSpecContainedInSpecs) {
    EXPECT_TRUE((is_spec_contained_in_specs<Positional, Positional>));
    EXPECT_TRUE((is_spec_contained_in_specs<Required, Positional, Required>));
    EXPECT_TRUE((is_spec_contained_in_specs<Store, Positional, Store, Required>));

    EXPECT_FALSE((is_spec_contained_in_specs<Positional, Required>));
    EXPECT_FALSE((is_spec_contained_in_specs<Required, Positional, Store>));
    EXPECT_FALSE((is_spec_contained_in_specs<Store, Positional, Help, Required>));
}

TEST(ArgParseInternal, SpecForArgImpl) {
    EXPECT_TRUE((is_valid_spec_for_arg<Help>));
    EXPECT_TRUE((is_valid_spec_for_arg<Mnemonic>));
    EXPECT_TRUE((is_valid_spec_for_arg<Optional<int>>));
    EXPECT_TRUE((is_valid_spec_for_arg<Positional>));

    EXPECT_FALSE((is_valid_spec_for_arg<Required>));
    EXPECT_FALSE((is_valid_spec_for_arg<Store>));    
}

TEST(ArgParseInternal, SpecForOptionalArg) {
    EXPECT_TRUE((is_valid_spec_for_optional_arg<Help>));
    EXPECT_TRUE((is_valid_spec_for_optional_arg<Mnemonic>));

    EXPECT_FALSE((is_valid_spec_for_optional_arg<Optional<int>>));
    EXPECT_FALSE((is_valid_spec_for_optional_arg<Positional>));
    EXPECT_FALSE((is_valid_spec_for_optional_arg<Required>));
    EXPECT_FALSE((is_valid_spec_for_optional_arg<Store>));    
}

TEST(ArgParseInternal, SpecForSwitchArg) {
    EXPECT_TRUE((is_valid_spec_for_switch_arg<Help>));
    EXPECT_TRUE((is_valid_spec_for_switch_arg<Mnemonic>));
    EXPECT_TRUE((is_valid_spec_for_switch_arg<Store>));  

    EXPECT_FALSE((is_valid_spec_for_switch_arg<Optional<int>>));
    EXPECT_FALSE((is_valid_spec_for_switch_arg<Positional>));
    EXPECT_FALSE((is_valid_spec_for_switch_arg<Required>));
}

TEST(ArgParseInternal, SpecForNAryArg) {
    EXPECT_TRUE((is_valid_spec_for_n_ary_arg<Help>));
    EXPECT_TRUE((is_valid_spec_for_n_ary_arg<Mnemonic>));
    EXPECT_TRUE((is_valid_spec_for_n_ary_arg<Positional>));
    EXPECT_TRUE((is_valid_spec_for_n_ary_arg<Required>));
      
    EXPECT_FALSE((is_valid_spec_for_n_ary_arg<Optional<int>>));
    EXPECT_FALSE((is_valid_spec_for_n_ary_arg<Store>));
}

TEST(ArgParseInternal, Optional) {
    EXPECT_TRUE((is_optional<std::optional<int>>));
      
    EXPECT_FALSE((is_optional<int>));
}

TEST(ArgParseInternal, GetArgCommonData) {
    auto arg_common_data = getArgCommonData("arg", 
        Help{.text = "help"},
        Mnemonic{.short_arg = "a"},
        Positional{},
        Required{}
    );
    EXPECT_THAT(arg_common_data.help, ::testing::Optional(std::string{"help"}));
    EXPECT_THAT(arg_common_data.mnemonic, ::testing::Optional(std::string{"-a"}));
    EXPECT_TRUE(arg_common_data.is_positional);
    EXPECT_TRUE(arg_common_data.is_required);
}

TEST(ArgParseInternal, ConstructArgumentData) {
    EXPECT_TRUE((std::is_same_v<
        ConstructArgumentData<Arg<"arg1", int>, Arg<"arg2", double>>,
        TypeList<Field<"arg1", int>, Field<"arg2", double>>
    >));
}