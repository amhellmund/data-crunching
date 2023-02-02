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

#include <data_crunching/string.hpp>

using namespace dacr;

struct Namespace {
    std::string name;
};

int main (int argc, char*argv[]) {
    std::vector<Namespace> ns = dacr::split("abc,cde", ",");
    std::set<int> unique_ids = dacr::split("10,20,30", ",");

    auto tuple = dacr::split<int,double>("10,20.12", ",");
    auto [id, name, size] = dacr::split<int,dacr::Skip<2>,std::string,double>("101,Toronto,James,Last,1.76", ",");
}