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

#ifndef DATA_CRUNCHING_INTERNAL_FIXED_STRING_HPP
#define DATA_CRUNCHING_INTERNAL_FIXED_STRING_HPP

#include <algorithm>
#include <cstdlib>
#include <utility>

namespace dacr {

/**
 * The FixedString is used to store string literals as template-arguments in the
 * DataFrame API.
*/
template<std::size_t N>
struct FixedString {
    constexpr FixedString() = default;

    constexpr FixedString(const char (&str)[N]) {
        std::copy_n(str, N, data);
    }

    constexpr std::size_t getLength() const {
        return N - 1;
    }

    template <size_t M>
    constexpr auto append (const char (&other)[M]) const {
        FixedString<M + N - 1> result;
        for (size_t i = 0; i < getLength(); ++i) {
            result.data[i] = data[i];
        }
        for (size_t i = 0; i < M-1; ++i) {
            result.data[getLength()+i] = other[i];
        }
        result.data[M + N - 2] = '\0';
        return result;
    }

    char data[N];
};

/**
 * CTAD for FixedString
*/
template <std::size_t N>
FixedString(const char (&str)[N]) -> FixedString<N>;

namespace internal {

/**
 * Equality comparison for FixedString
*/
template <std::size_t N1, std::size_t N2, std::size_t ...Indices>
constexpr bool areFixedStingsEqualImpl (FixedString<N1> lhs, FixedString<N2> rhs, std::integer_sequence<std::size_t, Indices...>) {
    return ((lhs.data[Indices] == rhs.data[Indices]) && ...);
}

template <std::size_t N1, std::size_t N2>
constexpr bool areFixedStringsEqual (FixedString<N1> lhs, FixedString<N2> rhs) {
    return (N1 == N2) && areFixedStingsEqualImpl(lhs, rhs, std::make_index_sequence<std::min(N1, N2)>{});
}

} // namespace internal

} // namespace dacr

#endif // DATA_CRUNCHING_INTERNAL_COLUMN_HPP