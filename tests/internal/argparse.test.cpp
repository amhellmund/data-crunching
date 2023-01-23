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

// TEST(ArgParseInternal, ConstructArgumentData) {
//     EXPECT_TRUE((std::is_same_v<
//         ConstructArgumentData<Arg<"arg1", int>, Arg<"arg2", double>>,
//         TypeList<Field<"arg1", int>, Field<"arg2", double>>
//     >));
// }