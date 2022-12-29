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

#ifndef DATA_CRUNCHING_DATAFRAME_HPP
#define DATA_CRUNCHING_DATAFRAME_HPP

#include "data_crunching/internal/column.hpp"
#include "data_crunching/internal/name_list.hpp"

namespace dacr {

template <internal::IsColumn ...Columns>
requires internal::are_names_unique_v<internal::GetColumnNames<Columns...>>
class DataFrame {
public:
    template<internal::IsColumn ...OtherColumns>
    requires internal::are_names_unique_v<internal::GetColumnNames<OtherColumns...>>
    friend class DataFrame;

private:
};

} // namespace dacr

#endif // DATA_CRUNCHING_DATAFRAME_HPP