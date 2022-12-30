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

template <internal::FixedString FieldName, typename FieldType>
struct Field {};

namespace internal {

// ############################################################################
// Trait: Get Types From Fields
// ############################################################################
template <typename ...>
struct GetTypesFromFieldsImpl {
    using type = TypeList<>;
};

template <internal::FixedString FirstFieldName, typename FirstFieldType, typename ...RestFields>
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
template <std::size_t LoopIndex, internal::FixedString, typename...>
struct GetFieldIndexByNameImpl {
    static constexpr std::size_t index = LoopIndex;
};

template <std::size_t LoopIndex, internal::FixedString NameToSearch, internal::FixedString FirstFieldName, typename FirstFieldType, typename ...RestFields>
struct GetFieldIndexByNameImpl<LoopIndex, NameToSearch, Field<FirstFieldName, FirstFieldType>, RestFields...>
{
    static constexpr std::size_t index = (
        areFixedStringsEqual(NameToSearch, FirstFieldName) ?
            LoopIndex :
            GetFieldIndexByNameImpl<LoopIndex + 1, NameToSearch, RestFields...>::index
    ); 
};

template <internal::FixedString NameToSearch, typename ...Fields>
constexpr std::size_t get_field_index_by_name = GetFieldIndexByNameImpl<0, NameToSearch, Fields...>::index;

// ############################################################################
// Concept: Is Field
// ############################################################################
template <typename>
struct IsFieldImpl : std::false_type {};

template <internal::FixedString FieldName, typename FieldType>
requires (FieldName.getLength() > 0)
struct IsFieldImpl<Field<FieldName, FieldType>> : std::true_type {};

template <typename FieldType>
concept IsField = IsFieldImpl<FieldType>::value;

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

    template <internal::FixedString FieldName, typename T>
    void set(T&& value) {
        return std::get<internal::get_field_index_by_name<FieldName, Fields...>>(data_) = std::forward<T>(value);
    }

    template <internal::FixedString FieldName>
    const auto& get() const {
        return std::get<internal::get_field_index_by_name<FieldName, Fields...>>(data_);
    }

private:
    using NamedTupleData = internal::ConvertTypeListToTuple<internal::GetTypesFromFields<Fields...>>;

    NamedTupleData data_{};
};

template <internal::FixedString ...FieldNames, typename ...FieldTypes>
requires (sizeof...(FieldNames) == sizeof...(FieldTypes))
auto makeNamedTuple(FieldTypes&& ...values) {
    return NamedTuple<Field<FieldNames, FieldTypes>...>(std::forward<FieldTypes>(values)...);
}

// template <std::size_t LoopIndex, FixedString Name, typename NameList>
// struct GetIndexByNameHelper {
//     static constexpr std::size_t index = -1;
// };

// template <std::size_t LoopIndex, FixedString Name, FixedString FirstNameInList, FixedString ...RestNames>
// struct GetIndexByNameHelper<LoopIndex, Name, Names<FirstNameInList, RestNames...>> {
//     static constexpr std::size_t index = areFixedStringsEqual(Name, FirstNameInList) ? LoopIndex : GetIndexByNameHelper<LoopIndex + 1, Name, Names<RestNames...>>::index;
// };

// template <FixedString Name, typename NameList>
// constexpr std::size_t GetIndexByName = GetIndexByNameHelper<0, Name, NameList>::index;

// template <std::size_t LoopIndex, std::size_t SearchIndex, typename ...Types>
// struct GetTypeByIndexHelper {
//     using type = void;
// };

// template <std::size_t LoopIndex, std::size_t SearchIndex, typename First, typename ...Rest>
// struct GetTypeByIndexHelper<LoopIndex, SearchIndex, First, Rest...> {
//     using type = std::conditional_t<
//         LoopIndex == SearchIndex,
//         First,
//         typename GetTypeByIndexHelper<LoopIndex+1, SearchIndex, Rest...>::type
//     >;
// };

// template <std::size_t Index, typename ...Types>
// using GetTypeByIndex = typename GetTypeByIndexHelper<0, Index, Types...>::type;



} // namespace dacr

#endif // DATA_CRUNCHING_NAMEDTUPLE_HPP