// =====================================================================================
// 
//       Filename:  listvsvect.hpp
// 
//    Description:  Benchmark list vs vector
// 
//        Version:  1.0
//        Created:  03/22/2013 04:17:42 PM
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
#ifndef MBM_LISTVSVECT_HPP_
#define MBM_LISTVSVECT_HPP_

#include <mbm/mbm.hpp>

#include <list>
#include <vector>
#include <algorithm>

namespace lv {
struct fixture_state {
    size_t noents;
    std::vector<int> data;

    void init(size_t n) {
        noents = n;
        std::random_device rd;
        std::default_random_engine e(rd());

        std::uniform_int_distribution<int> uniform_dist(0, n);
        data.clear();
        for (auto i = 0u; i < noents; i++)
            data.emplace_back(uniform_dist(e));
    }
};

struct list_fixture : mbm::fixture { 
    fixture_state state;
    virtual void setup(const boost::any & v) {
        state.init(boost::any_cast<int>(v));
    }

    virtual void go() {
        std::list<int> ilist;
        std::for_each(std::begin(state.data), std::end(state.data), 
                [&](int p) {
                    auto it = std::find_if(std::begin(ilist), std::end(ilist), [p](int v) { return p < v; });
                    ilist.insert(it, p);   
                });
        std::for_each(std::begin(state.data), std::end(state.data),
                [&](int p) {
                    auto it = std::find(std::begin(ilist), std::end(ilist), p);
                    ilist.erase(it);
                });
    }
};

struct vect_fixture : mbm::fixture { 
    fixture_state state;
    virtual void setup(const boost::any & v) {
        state.init(boost::any_cast<int>(v));
    }

    virtual void go() {
        std::vector<int> ivect;
        std::for_each(std::begin(state.data), std::end(state.data),
                [&](int p) {
                    auto it = std::find_if(std::begin(ivect), std::end(ivect), [p](int v) { return p < v; });
                    if (it != std::end(ivect))
                        ivect.insert(it, p);
                    else
                        ivect.emplace_back(p); 
                });
        std::for_each(std::begin(state.data), std::end(state.data),
               [&](int p) {
                   auto it = std::find(std::begin(ivect), std::end(ivect), p);
                   ivect.erase(it);
               });  
    }
};
} // namespace lv
#endif // MBM_LISTVSVECT_HPP_
