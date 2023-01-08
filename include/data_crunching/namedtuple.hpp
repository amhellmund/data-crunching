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

template <internal::IsField ...Fields>
requires (sizeof...(Fields) > 0)
class NamedTuple {
public:
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

private:
    using NamedTupleData = typename internal::GetTypesFromFields<Fields...>::template To<std::tuple>;

    NamedTupleData data_{};
};

template <internal::IsField ...CtorFields>
NamedTuple(CtorFields&& ...fields) -> NamedTuple<CtorFields...>;

template <FixedString ...FieldNames, typename ...FieldTypes>
requires (sizeof...(FieldNames) == sizeof...(FieldTypes))
auto makeNamedTuple(FieldTypes&& ...values) {
    return NamedTuple<Field<FieldNames, FieldTypes>...>(std::forward<FieldTypes>(values)...);
}

#define dacr_field(field_name) dacr::internal::FieldForward<field_name>{}

} // namespace dacr

// Inspired by MeetingC++ Talk: https://krzysztof-jusiak.github.io/talks/++namedtuple/index-meetingcpp.html
template<dacr::FixedString A>
constexpr auto operator"" _field()
{
    return dacr::internal::FieldForward<A>{};
}

#endif // DATA_CRUNCHING_NAMEDTUPLE_HPP