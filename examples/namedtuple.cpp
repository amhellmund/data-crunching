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

#include <data_crunching/namedtuple.hpp>
#include <iostream>

int main (int argc, char*argv[]) {
    // variant 1: NamedTuple definition
    using NT = dacr::NamedTuple<dacr::Field<"a", int>, dacr::Field<"d", double>>;
    NT namedtuple1{10, 20.0};

    // variant 2: direct field initialization
    auto namedtuple2 = dacr::NamedTuple(dacr_field("a") = 10, dacr_field("d") = 20.0);

    // variant 3: direct-field initialization via custom literals
    auto namedtuple3 = dacr::NamedTuple("a"_field = 10, "d"_field = 20.0);

    // access to members
    namedtuple3.get<"a">() = 2.0 * namedtuple3.get<"d">();

    // structured bindings
    auto [a, d] = namedtuple3;
}