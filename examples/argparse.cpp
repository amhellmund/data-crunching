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

#include <iostream>
#include <data_crunching/argparse.hpp>


using namespace dacr;

struct Namespace {
    std::string name;
};

std::ostream& operator<< (std::ostream& os, const Namespace& ns) {
    os << "Namespace: " << ns.name;
    return os;
}

int main (int argc, char*argv[]) {
    auto argparser = ArgumentParser(
        Arg<"namespace", Namespace>(mnemonic("n"), help("The namespace"), optional("cde")),
        Arg<"switch", bool>(mnemonic("s"), help("Help text"), store(true)),
        Arg<"input", std::vector<int>>(positional())
    );
    auto args = argparser.parse(argc, argv);
    std::cout << args.get<"namespace">() << "\n";
    std::cout << std::boolalpha << args.get<"switch">() << "\n";
    std::cout << args.get<"input">().size() << "\n";
    for (auto v : args.get<"input">()) {
        std::cout << "  " << v << "\n";
    }
}