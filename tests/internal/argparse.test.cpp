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

#include "data_crunching/argparse.hpp"
#include "data_crunching/internal/argparse.hpp"

using namespace dacr;
using namespace dacr::internal;

TEST(ArgParseInternal, GetMnemonic) {
    auto mnemonic = getMnemonic(Required{}, Mnemonic{.short_arg = "n"});
    EXPECT_THAT(mnemonic, ::testing::Optional(std::string{"n"}));

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
    EXPECT_FALSE(store);

    auto no_store = getStore(Required{}, Positional{});
    EXPECT_TRUE(no_store);
}

TEST(ArgParseInternal, IsSpecContainedInSpecs) {
    EXPECT_TRUE((is_spec_contained_in_specs<Positional, Positional>));
    EXPECT_TRUE((is_spec_contained_in_specs<Required, Positional, Required>));
    EXPECT_TRUE((is_spec_contained_in_specs<Store, Positional, Store, Required>));

    EXPECT_FALSE((is_spec_contained_in_specs<Positional, Required>));
    EXPECT_FALSE((is_spec_contained_in_specs<Required, Positional, Store>));
    EXPECT_FALSE((is_spec_contained_in_specs<Store, Positional, Help, Required>));
}

TEST(ArgParseInternal, IsOptionalContainedInSpecs) {
    EXPECT_TRUE((is_optional_contained_in_specs<Positional, Optional<int>>));
    EXPECT_FALSE((is_optional_contained_in_specs<Positional, Required>));
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
    EXPECT_THAT(arg_common_data.mnemonic, ::testing::Optional(std::string{"a"}));
    EXPECT_TRUE(arg_common_data.is_positional);
    EXPECT_TRUE(arg_common_data.is_required);
}

TEST(ArgParseInternal, ConstructArgumentData) {
    EXPECT_TRUE((std::is_same_v<
        ConstructArgumentData<ArgImpl<"arg1", int>, ArgImpl<"arg2", double>>,
        TypeList<Field<"arg1", int>, Field<"arg2", double>>
    >));
}

TEST(ArgParseInternal, IsArgumentMatched) {
    ArgCommonData common_data {.arg_name = "arg", .mnemonic = "a"};

    EXPECT_TRUE(isArgumentMatched("--arg", common_data));
    EXPECT_TRUE(isArgumentMatched("-a", common_data));

    EXPECT_FALSE(isArgumentMatched("--other", common_data));
    EXPECT_FALSE(isArgumentMatched("-o", common_data));
}

TEST(ArgParseInternal, IsPositionalArgument) {
    EXPECT_TRUE(isPositionalArgument("arg"));

    EXPECT_FALSE(isPositionalArgument("--arg"));
    EXPECT_FALSE(isPositionalArgument("-a"));
}

auto isArgumentConsumption (ArgConsumption::Status status, int consume_count = -1, std::string error_message = "") {
    return ::testing::AllOf(
        ::testing::Field(&ArgConsumption::status, ::testing::Eq(status)),
        ::testing::Field(&ArgConsumption::consume_count, ::testing::Eq(consume_count)),
        ::testing::Field(&ArgConsumption::error_message, ::testing::Eq(error_message))
    );
}

template <typename T>
struct NamedTupleMock {
    template <FixedString Name>
    T& get () {
        return value;
    }

    T value{};
};

// ############################################################################
// Test: Argument
// ############################################################################
TEST(ArgParseInternal, ArgConsume) {
    ArgImpl<"arg", int> arg{};
    EXPECT_THAT(arg.consume({"--arg", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(10));
}

TEST(ArgParseInternal, ArgConsumeTwice) {
    ArgImpl<"arg", int> arg{};
    EXPECT_THAT(arg.consume({"--arg", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(10));

    EXPECT_THAT(arg.consume({"--arg", "20"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(20));
}

TEST(ArgParseInternal, ArgConsumeConversionFailure) {
    ArgImpl<"arg", int> arg{};
    EXPECT_THAT(arg.consume({"--arg", "cde"}, 0), isArgumentConsumption(ArgConsumption::Status::ERROR, -1, "argument conversion failed"));
}

TEST(ArgParseInternal, ArgConsumeMnemonic) {
    ArgImpl<"arg", int> arg{mnemonic("a")};
    EXPECT_THAT(arg.consume({"-a", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(10));

    EXPECT_THAT(arg.consume({"-b", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::NO_MATCH));
}

TEST(ArgParseInternal, ArgConsumeMissingArgument) {
    ArgImpl<"arg", int> arg{mnemonic("a")};
    EXPECT_THAT(arg.consume({"--arg"}, 0), isArgumentConsumption(ArgConsumption::Status::ERROR, -1, "missing argument"));
    EXPECT_THAT(arg.consume({"-a"}, 0), isArgumentConsumption(ArgConsumption::Status::ERROR, -1, "missing argument"));
}

TEST(ArgParseInternal, ArgConsumePositional) {
    ArgImpl<"arg", int> arg{positional()};
    EXPECT_THAT(arg.consume({"10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 1));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(10));
}

TEST(ArgParseInternal, ArgConsumePositionalConversionFailure) {
    ArgImpl<"arg", int> arg{positional()};
    EXPECT_THAT(arg.consume({"cde"}, 0), isArgumentConsumption(ArgConsumption::Status::ERROR, -1, "argument conversion failed"));
}

TEST(ArgParseInternal, ArgConsumePositionalTwice) {
    ArgImpl<"arg", int> arg{positional()};
    EXPECT_THAT(arg.consume({"10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 1));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(10));

    EXPECT_THAT(arg.consume({"20"}, 0), isArgumentConsumption(ArgConsumption::Status::NO_MATCH));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(10));
}

TEST(ArgParseInternal, ArgStoreValueWithOptional) {
    NamedTupleMock<int> nt;
    ArgImpl<"arg", int> arg{optional(10)};
    auto store_result = arg.storeValue(nt);

    EXPECT_TRUE(store_result.success);
    EXPECT_EQ(nt.value, 10);
}

TEST(ArgParseInternal, ArgStoreValueRequired) {
    NamedTupleMock<int> nt;
    ArgImpl<"arg", int> arg{};
    auto store_result = arg.storeValue(nt);

    EXPECT_FALSE(store_result.success);
    EXPECT_THAT(store_result.error_message, ::testing::StartsWith("argument is required but not found"));
}

// ############################################################################
// Test: Otional Argument
// ############################################################################
TEST(ArgParseInternal, OptionalArgNoValue) {
    ArgImpl<"arg", std::optional<int>> arg{};
    EXPECT_FALSE(arg.getValue().has_value());
}

TEST(ArgParseInternal, OptionalArgConsume) {
    ArgImpl<"arg", std::optional<int>> arg{};
    EXPECT_THAT(arg.consume({"--arg", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(10));
}

TEST(ArgParseInternal, OptionalArgConsumeMnemonic) {
    ArgImpl<"arg", std::optional<int>> arg{mnemonic("a")};
    EXPECT_THAT(arg.consume({"-a", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(10));

    EXPECT_THAT(arg.consume({"-b", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::NO_MATCH));
}


TEST(ArgParseInternal, OptionalArgConsumeConversionFailure) {
    ArgImpl<"arg", std::optional<int>> arg{};
    EXPECT_THAT(arg.consume({"--arg", "cde"}, 0), isArgumentConsumption(ArgConsumption::Status::ERROR, -1, "argument conversion failed"));
}

TEST(ArgParseInternal, OptionalArgConsumePositionalTwice) {
    ArgImpl<"arg", std::optional<int>> arg{};
    EXPECT_THAT(arg.consume({"--arg", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(10));

    EXPECT_THAT(arg.consume({"--arg", "20"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::Optional(20));
}

TEST(ArgParseInternal, OptionalArgStoreValue) {
    NamedTupleMock<std::optional<int>> nt;
    ArgImpl<"arg", std::optional<int>> arg{};
    auto store_result = arg.storeValue(nt);

    EXPECT_TRUE(store_result.success);
    EXPECT_FALSE(nt.value.has_value());
}

// ############################################################################
// Test: Switch Argument
// ############################################################################
TEST(ArgParseInternal, SwitchArgNoValue) {
    ArgImpl<"arg", bool> arg{};
    EXPECT_FALSE(arg.getValue());
}

TEST(ArgParseInternal, SwitchArgConsume) {
    ArgImpl<"arg", bool> arg{};
    EXPECT_THAT(arg.consume({"--arg"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 1));
    EXPECT_TRUE(arg.getValue());
}

TEST(ArgParseInternal, SwitchArgNoValueStoreFalse) {
    ArgImpl<"arg", bool> arg{store(false)};
    EXPECT_TRUE(arg.getValue());
}

TEST(ArgParseInternal, SwitchArgConsumeStoreFalse) {
    ArgImpl<"arg", bool> arg{store(false)};
    EXPECT_THAT(arg.consume({"--arg"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 1));
    EXPECT_FALSE(arg.getValue());
}

TEST(ArgParseInternal, SwitchArgConsumeMnemonic) {
    ArgImpl<"arg", bool> arg{mnemonic("a")};
    EXPECT_THAT(arg.consume({"-a"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 1));
    EXPECT_TRUE(arg.getValue());
}

TEST(ArgParseInternal, SwitchArgStoreValue) {
    NamedTupleMock<bool> nt;
    ArgImpl<"arg", bool> arg{};
    auto store_result = arg.storeValue(nt);

    EXPECT_TRUE(store_result.success);
    EXPECT_FALSE(nt.value);
}

// ############################################################################
// Test: N-Ary Argument
// ############################################################################
TEST(ArgParseInternal, NAryArgNoValue) {
    ArgImpl<"arg", std::vector<int>> arg{};
    EXPECT_THAT(arg.getValue(), ::testing::ElementsAre());
}

TEST(ArgParseInternal, NAryArgConsume) {
    ArgImpl<"arg", std::vector<int>> arg{};
    EXPECT_THAT(arg.consume({"--arg", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::ElementsAre(10));
}

TEST(ArgParseInternal, NAryArgConsumeMultiple) {
    ArgImpl<"arg", std::vector<int>> arg{};
    EXPECT_THAT(arg.consume({"--arg", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.consume({"--arg", "20"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::ElementsAre(10, 20));
}

TEST(ArgParseInternal, NAryArgConsumeMultipleMnemonic) {
    ArgImpl<"arg", std::vector<int>> arg{mnemonic("a")};
    EXPECT_THAT(arg.consume({"--arg", "10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.consume({"-a", "15"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.consume({"--arg", "20"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 2));
    EXPECT_THAT(arg.getValue(), ::testing::ElementsAre(10, 15, 20));
}

TEST(ArgParseInternal, NAryArgConsumeConversionFailure) {
    ArgImpl<"arg", std::vector<int>> arg{};
    EXPECT_THAT(arg.consume({"--arg", "cde"}, 0), isArgumentConsumption(ArgConsumption::Status::ERROR, -1, "argument conversion failed"));
}

TEST(ArgParseInternal, NAryArgConsumePositional) {
    ArgImpl<"arg", std::vector<int>> arg{positional()};
    EXPECT_THAT(arg.consume({"10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 1));
    EXPECT_THAT(arg.getValue(), ::testing::ElementsAre(10));
}

TEST(ArgParseInternal, NAryArgConsumePositionalMultiple) {
    ArgImpl<"arg", std::vector<int>> arg{positional()};
    EXPECT_THAT(arg.consume({"10"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 1));
    EXPECT_THAT(arg.consume({"15"}, 0), isArgumentConsumption(ArgConsumption::Status::MATCH, 1));
    EXPECT_THAT(arg.getValue(), ::testing::ElementsAre(10, 15));
}

TEST(ArgParseInternal, CollectArgCommonData) {
    auto common_data = collectArgCommonData(ArgImpl<"abc", int>{});
    EXPECT_EQ(common_data.size(), 1);

    auto common_data_two_elements = collectArgCommonData(ArgImpl<"abc", int>{}, ArgImpl<"cde", bool>{});
    EXPECT_EQ(common_data_two_elements.size(), 2);
}

TEST(ArgParseInternal, ValidateArgsSuccess) {
    auto result = validateArgs(
        ArgImpl<"arg", int>{mnemonic("a"), help("Some help text")},
        ArgImpl<"other", std::string>{mnemonic("o"), optional("abc")},
        ArgImpl<"switch", bool>{mnemonic("s"), store(false)},
        ArgImpl<"list", std::vector<int>>{mnemonic("l")}
    );
    EXPECT_TRUE(result.success);
}

TEST(ArgParseInternal, ValidateArgsUniqueNamesFailure) {
    auto result = validateArgs(
        ArgImpl<"arg", int>{mnemonic("a"), help("Some help text")},
        ArgImpl<"arg", std::string>{mnemonic("o"), optional("abc")},
        ArgImpl<"switch", bool>{mnemonic("s"), store(false)},
        ArgImpl<"list", std::vector<int>>{mnemonic("l")}
    );
    EXPECT_FALSE(result.success);
    EXPECT_THAT(result.error_message, ::testing::StartsWith("argument name is not unique"));
}

TEST(ArgParseInternal, ValidateArgsUniqueMnemonicsFailure) {
    auto result = validateArgs(
        ArgImpl<"arg", int>{mnemonic("a"), help("Some help text")},
        ArgImpl<"other", std::string>{mnemonic("a"), optional("abc")},
        ArgImpl<"switch", bool>{mnemonic("s"), store(false)},
        ArgImpl<"list", std::vector<int>>{mnemonic("l")}
    );
    EXPECT_FALSE(result.success);
    EXPECT_THAT(result.error_message, ::testing::StartsWith("argument mnemonic is not unique"));
}

TEST(ArgParseInternal, ValidateArgsMultipleNAryFailure) {
    auto result = validateArgs(
        ArgImpl<"arg", int>{mnemonic("a"), help("Some help text")},
        ArgImpl<"other", std::string>{mnemonic("o"), optional("abc")},
        ArgImpl<"switch", std::vector<std::string>>{mnemonic("s"), positional()},
        ArgImpl<"list", std::vector<int>>{mnemonic("l"), positional()}
    );
    EXPECT_FALSE(result.success);
    EXPECT_THAT(result.error_message, ::testing::StartsWith("argument is never reached"));
}

TEST(ArgParseInternal, NAryArgStoreValue) {
    NamedTupleMock<std::vector<std::string>> nt;
    ArgImpl<"arg", std::vector<std::string>> arg{};
    auto store_result = arg.storeValue(nt);

    EXPECT_TRUE(store_result.success);
    EXPECT_THAT(nt.value, ::testing::ElementsAre());
}

TEST(ArgParseInternal, NAryArgStoreValueRequired) {
    NamedTupleMock<std::vector<std::string>> nt;
    ArgImpl<"arg", std::vector<std::string>> arg{required()};
    auto store_result = arg.storeValue(nt);

    EXPECT_FALSE(store_result.success);
    EXPECT_THAT(store_result.error_message, ::testing::StartsWith("argument is required but not found"));
}

TEST(ArgParseInternal, NAryArgStoreValuePositionalRequired) {
    NamedTupleMock<std::vector<std::string>> nt;
    ArgImpl<"arg", std::vector<std::string>> arg{positional()};
    auto store_result = arg.storeValue(nt);

    EXPECT_FALSE(store_result.success);
    EXPECT_THAT(store_result.error_message, ::testing::StartsWith("argument is required but not found"));
}