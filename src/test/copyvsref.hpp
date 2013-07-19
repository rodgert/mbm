// =====================================================================================
// 
//       Filename:  copyvsref.hpp
// 
//    Description:  Benchmark cost of const & vs copy
// 
//        Version:  1.0
//        Created:  04/02/2013 08:08:33 PM
//       Revision:  none
//       Compiler:  g++ 4.7 or later
// 
//         Author:  Thomas Rodgers (twr), rodgert@twrodgers.com
//
//	Copyright (c) 2013, Thomas Rodgers
//
//      This file is part of FOOBAR.
//
//      FOOBAR is free software: you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation, either version 3 of the License, or
//      (at your option) any later version.
//
//      FOOBAR is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with FOOBAR.  If not, see <http://www.gnu.org/licenses/>.
//
// =====================================================================================
#ifndef MBM_COPYVSREF_HPP_
#define MBM_COPYVSREF_HPP_

#include <mbm/mbm.hpp>

#include <array>
#include <vector>
#include <algorithm>

namespace cr {
struct rect {
    int ul;
    int ur;
    int ll;
    int lr;
};

struct fixture_state {
    std::vector<rect> data;
    std::vector<int> res;

    void init(size_t n) {
        data.resize(n);
        res.resize(n);

        std::random_device rd;
        std::default_random_engine e(rd());

        std::uniform_int_distribution<int> uniform_dist(0, 100);
        data.clear();
        res.clear();

        for (auto i = 0u; i < n; i++)
            data.emplace_back(rect { uniform_dist(e), uniform_dist(e), uniform_dist(e), uniform_dist(e) });
    }
};

struct copy_fixture : mbm::fixture { 
    fixture_state state;
    virtual void setup(const boost::any & v) {
        state.init(boost::any_cast<int>(v));
    }

    virtual void go() {
        std::for_each(std::begin(state.data), std::end(state.data),
                [&](rect r) {
                    state.res.emplace_back((r.ll - r.ul) * (r.lr - r.ur));           
                });
    }
};

struct ref_fixture : mbm::fixture { 
    fixture_state state;
    virtual void setup(const boost::any & v) {
        state.init(boost::any_cast<int>(v));
    }

    virtual void go() {
        std::for_each(std::begin(state.data), std::end(state.data),
                [&](const rect & r) {
                    state.res.emplace_back((r.ll - r.ul) * (r.lr - r.ur));           
                });
    }
};

struct str_fixture_state {
    const static size_t size = 18;
    typedef std::array<char, size> array_t;
    std::vector<array_t> data;
    std::vector<size_t> res;

    void init(size_t n) {
        data.resize(n);
        res.resize(n);

        std::random_device rd;
        std::default_random_engine e(rd());

        std::uniform_int_distribution<char> char_dist('a', 'z');
        data.clear();
        res.clear();

        for (auto i = 0u; i < n; i++) {
            array_t res;
            for (auto j = 0u; j < size; j++)
                res[j] = char_dist(e);
            data.emplace_back(res);
        }
    }
};

struct copy_str_fixture : mbm::fixture {
    str_fixture_state state;
    
    static size_t fn(const str_fixture_state::array_t & r) {
        return std::accumulate(std::begin(r), std::end(r), 0u);
    }

    virtual void setup(const boost::any & v) {
        state.init(boost::any_cast<int>(v));
    }

    virtual void go() {
        std::for_each(std::begin(state.data), std::end(state.data),
                [&](str_fixture_state::array_t r) {
                    state.res.emplace_back(fn(r));           
                });
    }

};

struct ref_str_fixture : mbm::fixture {
    str_fixture_state state;

    static size_t fn(const str_fixture_state::array_t & r) {
        return std::accumulate(std::begin(r), std::end(r), 0u);
    }
    
    virtual void setup(const boost::any & v) {
        state.init(boost::any_cast<int>(v));
    }

    virtual void go() {
        std::for_each(std::begin(state.data), std::end(state.data),
                [&](const str_fixture_state::array_t & r) {
                    state.res.emplace_back(fn(r));           
                });
    }

};
}
#endif // MBM_COPYVSREF_HPP_
