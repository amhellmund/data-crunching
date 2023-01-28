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

#include <array>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "data_crunching/argparse.hpp"

using namespace dacr;

struct CustomClass {
    std::string value;
};

TEST(ArgParse, End2End) {
    auto argparse = ArgumentParser(
        "Sample Test Program",
        Arg<"char", char>(mnemonic("c"), help("Char argument")),
        Arg<"int", int>(mnemonic("i"), help("Int argument"), optional(10)),
        Arg<"float", float>(mnemonic("f"), help("Float argument"), positional()),
        Arg<"double", double>(mnemonic("d"), help("Double argument"), optional(12.24)),
        Arg<"custom", CustomClass>(mnemonic("cu"), help("Custom class argument"), optional("some_value")),
        Arg<"optdouble", std::optional<double>>(mnemonic("od"), help("Optional double argument")),
        Arg<"switch", bool>(mnemonic("s"), help("Switch argument")),
        Arg<"other_switch", bool>(mnemonic("os"), help("Other switch argument"), store(false)),
        Arg<"list_int", std::vector<int>>(mnemonic("li"), help("List of int argument")),
        Arg<"list_double", std::vector<double>>(mnemonic("ld"), help("List of double argument")),
        Arg<"list_float", std::vector<float>>(mnemonic("lf"), help("List of float argument"), required()),
        Arg<"list_char", std::vector<char>>(mnemonic("lc"), help("List of char argument"), positional())
    );
    const char *argv[] = {
        "test_program",
        "--char", "A",
        "--int", "123",
        "42.0",
        "--custom", "my_custom_string",
        "--switch",
        "-os",
        "--list_int", "10",
        "-li", "20",
        "--list_double", "234.223",
        "--list_float", "1.23",
        "-lf", "2.34",
        "--list_char", "a"
    };
    auto args = argparse.parse(sizeof(argv) / sizeof(argv[0]), argv);

    EXPECT_EQ(args.get<"char">(), 'A');
    EXPECT_EQ(args.get<"int">(), 123);
    EXPECT_THAT(args.get<"float">(), ::testing::FloatEq(42.0));
    EXPECT_THAT(args.get<"double">(), ::testing::DoubleEq(12.24));
    EXPECT_THAT(args.get<"custom">().value, ::testing::StrEq("my_custom_string"));
    EXPECT_TRUE(args.get<"switch">());
    EXPECT_FALSE(args.get<"other_switch">());
    EXPECT_THAT(args.get<"list_int">(), ::testing::ElementsAre(10, 20));
    EXPECT_THAT(args.get<"list_char">(), ::testing::ElementsAre('a'));
    EXPECT_THAT(args.get<"list_float">(), ::testing::ElementsAre(::testing::FloatEq(1.23), ::testing::FloatEq(2.34)));
    EXPECT_THAT(args.get<"list_double">(), ::testing::ElementsAre(::testing::DoubleEq(234.223)));
}

#include "data_crunching/argparse.hpp"