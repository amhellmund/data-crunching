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

void operator"" _print(const char* str)
{
    std::cout << str << '\n';
}


int main (int argc, char*argv[]) {
    // variant A: NamedTuple definition
    using NT = dacr::NamedTuple<dacr::Field<"a", int>, dacr::Field<"d", double>>;
    NT namedtupleA{10, 20.0};

    // variant B: makeNamedTuple
    auto namedtupleB = dacr::makeNamedTuple<"a", "d">(10, 20.0);

    // variant C: direct field initialization
    auto namedtupleC = dacr::NamedTuple(dacr_field("a") = 10, dacr_field("d") = 20.0);

    // variant D: direct-field initialization via custom literals (equivalent to variant C)
    auto namedtupleD = dacr::NamedTuple("a"_field = 10, "d"_field = 20.0);

    namedtupleD.get<"a">() = 2.0 * namedtupleD.get<"d">();
}