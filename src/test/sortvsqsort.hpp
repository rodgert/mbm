// =====================================================================================
// 
//       Filename:  sortvsqsort.hpp
// 
//    Description:  Benchmarking c qsort vs std C++ sort
// 
//        Version:  1.0
//        Created:  03/20/2013 06:15:00 PM
//       Revision:  none
//       Compiler:  g++ 4.7 or later
// 
//         Author:  Thomas Rodgers (twr), rodgert@twrodgers.com
//
//	Copyright (c) 2013, Thomas Rodgers
//
// 	Licensed under the Apache License, Version 2.0 (the "License");
//	you may not use this file except in compliance with the License.
//	You may obtain a copy of the License at
//
//	   http://www.apache.org/licenses/LICENSE-2.0
//
//	Unless required by applicable law or agreed to in writing, software
//	distributed under the License is distributed on an "AS IS" BASIS,
//	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//	See the License for the specific language governing permissions and
//	limitations under the License.
// =====================================================================================
#ifndef MBM_SORTVSQSORT_HPP_
#define MBM_SORTVSQSORT_HPP_

#include <mbm/mbm.hpp>

#include <algorithm>
#include <cstdlib>
#include <random>
#include <vector>
#include <limits>

struct fixture_state {
    size_t noents;
    std::vector<double> data;

    void init(size_t n) {
        noents = n;
        std::random_device rd;
        std::default_random_engine e(rd());

        std::uniform_real_distribution<double> uniform_dist(0.0, 10000000.0);
        data.clear();
        for (auto i = 0u; i < noents; i++)
            data.emplace_back(uniform_dist(e));
    }
};

int compf(const void * v1, const void * v2) {
    auto vv1 = *reinterpret_cast<const int*>(v1);
    auto vv2 = *reinterpret_cast<const int*>(v2);

    if (vv1 < vv2) return -1;
    if (vv1 > vv2) return 1;
    return 0;
}

struct qsort_fixture : mbm::fixture {
    fixture_state state;
    virtual void setup(const boost::any & v) { 
        state.init(boost::any_cast<int>(v)); 
    }

    virtual void go() {
        qsort(state.data.data(), state.noents, sizeof(double), compf);
        result(&state);
    }
};

struct sort_fixture : mbm::fixture {
    fixture_state state;
    virtual void setup(const boost::any & v) { 
        state.init(boost::any_cast<int>(v)); 
    }

    virtual void go() {
        std::sort(std::begin(state.data), std::end(state.data));
        result(&state);
    }
};
#endif // MBM_SORTVSQSORT_HPP_
