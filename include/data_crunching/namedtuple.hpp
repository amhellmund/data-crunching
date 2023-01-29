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

#ifndef DATA_CRUNCHING_NAMEDTUPLE_HPP
#define DATA_CRUNCHING_NAMEDTUPLE_HPP

#include "data_crunching/internal/fixed_string.hpp"
#include "data_crunching/internal/type_list.hpp"
#include "data_crunching/internal/utils.hpp"
#include "data_crunching/internal/name_list.hpp"

namespace dacr {

template <FixedString FieldName, typename FieldType>
struct Field {
    Field (FieldType&& initial_value) : value{std::forward<FieldType>(initial_value)} {}

    FieldType value;
};

namespace internal {

// ############################################################################
// Trait: Get Types From Fields
// ############################################################################
template <typename ...>
struct GetTypesFromFieldsImpl {
    using type = TypeList<>;
};

template <FixedString FirstFieldName, typename FirstFieldType, typename ...RestFields>
struct GetTypesFromFieldsImpl<Field<FirstFieldName, FirstFieldType>, RestFields...> {
    using type = internal::TypeListPrepend<
        FirstFieldType,
        typename GetTypesFromFieldsImpl<RestFields...>::type
    >;
};

template <typename ...Fields>
using GetTypesFromFields = typename GetTypesFromFieldsImpl<Fields...>::type;


// ############################################################################
// Trait: Get Type By Index
// ############################################################################
template <std::size_t, std::size_t, typename ...>
struct GetTypeFromFieldsByIndexImpl {
    using type = void;
};

template <std::size_t LoopIndex, std::size_t SearchIndex, FixedString FirstFieldName, typename FirstFieldType, typename ...RestFields>
struct GetTypeFromFieldsByIndexImpl<LoopIndex, SearchIndex, Field<FirstFieldName, FirstFieldType>, RestFields...> {
    using type = std::conditional_t<
        LoopIndex == SearchIndex,
        FirstFieldType,
        typename GetTypeFromFieldsByIndexImpl<LoopIndex + 1, SearchIndex, RestFields...>::type
    >;
};

template <std::size_t SearchIndex, typename ...Fields>
using GetTypeFromFieldsByIndex = typename GetTypeFromFieldsByIndexImpl<0, SearchIndex, Fields...>::type;


// ############################################################################
// Trait: Get Names From Fields
// ############################################################################
template <typename ...>
struct GetNamesFromFieldsImpl {
    using type = NameList<>;
};

template <FixedString FirstFieldName, typename FirstFieldType, typename ...RestFields>
struct GetNamesFromFieldsImpl<Field<FirstFieldName, FirstFieldType>, RestFields...> {
    using type = internal::NameListPrepend<
        FirstFieldName,
        typename GetNamesFromFieldsImpl<RestFields...>::type
    >;
};

template <typename ...Fields>
using GetNamesFromFields = typename GetNamesFromFieldsImpl<Fields...>::type;

// ############################################################################
// Concept: Get Index By Name
// ############################################################################
template <std::size_t LoopIndex, FixedString, typename...>
struct GetFieldIndexByNameImpl {
    static constexpr std::size_t index = LoopIndex;
};

template <std::size_t LoopIndex, FixedString NameToSearch, FixedString FirstFieldName, typename FirstFieldType, typename ...RestFields>
struct GetFieldIndexByNameImpl<LoopIndex, NameToSearch, Field<FirstFieldName, FirstFieldType>, RestFields...>
{
    static constexpr std::size_t index = (
        areFixedStringsEqual(NameToSearch, FirstFieldName) ?
            LoopIndex :
            GetFieldIndexByNameImpl<LoopIndex + 1, NameToSearch, RestFields...>::index
    ); 
};

template <FixedString NameToSearch, typename ...Fields>
constexpr std::size_t get_field_index_by_name = GetFieldIndexByNameImpl<0, NameToSearch, Fields...>::index;

// ############################################################################
// Concept: Is Field
// ############################################################################
template <typename>
struct IsFieldImpl : std::false_type {};

template <FixedString FieldName, typename FieldType>
requires (FieldName.getLength() > 0)
struct IsFieldImpl<Field<FieldName, FieldType>> : std::true_type {};

template <typename FieldType>
concept IsField = IsFieldImpl<FieldType>::value;

// ############################################################################
// Concept: Field Forwarder
// ############################################################################
template <FixedString FieldName>
struct FieldForward {
    template <typename T>
    auto operator= (T&& value) {
        return Field<FieldName, T>{std::forward<T>(value)};
    }
};

} // namespace internal

// ############################################################################
// Class: NamedTuple
// ############################################################################
template <internal::IsField ...Fields>
requires (
    sizeof...(Fields) > 0 &&
    internal::are_names_unique<internal::GetNamesFromFields<Fields...>>
)
class NamedTuple {
public:
    NamedTuple() = default;

    template <typename ...Types>
    requires (
        sizeof...(Types) == sizeof...(Fields) &&
        internal::is_convertible_to<TypeList<Types...>, internal::GetTypesFromFields<Fields...>>
    )
    NamedTuple(Types&& ...values) : data_{std::make_tuple(std::forward<Types>(values)...)} {}

    template <internal::IsField ...CtorFields>
    NamedTuple(CtorFields&& ...fields) : data_{std::make_tuple(fields.value...)} {}

    template <FixedString FieldName>
    auto& get() {
        return std::get<internal::get_field_index_by_name<FieldName, Fields...>>(data_);
    }

    template <FixedString FieldName>
    const auto& get() const {
        return std::get<internal::get_field_index_by_name<FieldName, Fields...>>(data_);
    }

    template <std::size_t Index>
    requires (Index < sizeof...(Fields))
    decltype(auto) get() & {
        return std::get<Index>(data_);
    }

    template <std::size_t Index>
    requires (Index < sizeof...(Fields))
    decltype(auto) get() const & {
        return std::get<Index>(data_);
    }

    template <std::size_t Index>
    requires (Index < sizeof...(Fields))
    decltype(auto) get() && {
        return std::move(std::get<Index>(data_));
    }

private:
    using NamedTupleData = typename internal::GetTypesFromFields<Fields...>::template To<std::tuple>;

    NamedTupleData data_{};
};

template <internal::IsField ...CtorFields>
NamedTuple(CtorFields&& ...fields) -> NamedTuple<CtorFields...>;

#define dacr_field(field_name) dacr::internal::FieldForward<field_name>{}

} // namespace dacr

#if !defined(DACR_DISABLE_FIELD_LITERAL)
// Inspired by MeetingC++ Talk: https://krzysztof-jusiak.github.io/talks/++namedtuple/index-meetingcpp.html
template<dacr::FixedString A>
constexpr auto operator"" _field()
{
    return dacr::internal::FieldForward<A>{};
}
#endif // DACR_DISABLE_FIELD_LITERAL

// ############################################################################
// Traits: Structured Bindings
// ############################################################################
template <typename ...Fields>
struct std::tuple_size<dacr::NamedTuple<Fields...>> {
    static constexpr int value = sizeof...(Fields);
};

template <std::size_t Index, typename ...Fields>
struct std::tuple_element<Index, dacr::NamedTuple<Fields...>> {
    using type = dacr::internal::GetTypeFromFieldsByIndex<Index, Fields...>;
};

#endif // DATA_CRUNCHING_NAMEDTUPLE_HPP