// =====================================================================================
// 
//       Filename:  maps.hpp
// 
//    Description:  
// 
//        Version:  1.0
//        Created:  04/18/2013 08:22:44 AM
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
#ifndef MBM_MAPS_HPP_
#define MBM_MAPS_HPP_

#include <cpp-btree/btree_set.h>

#include <mbm/mbm.hpp>
#include <array>
#include <vector>
#include <set>
#include <unordered_set>
#include <iterator>
#include <algorithm>
#include <random>
#include <iostream>
#include <fstream>

namespace maps {
    typedef std::vector<std::string> string_vec;
    string_vec read_words(const std::string & fname = "words.txt") {
        std::ifstream stm(fname);
        std::array<char, 1024> buf;
        string_vec res;
        while (stm.getline(buf.data(), buf.size())) {
           res.emplace_back(buf.data()); 
        }
        return res;
    }

    struct vec_insert_fixture : mbm::fixture {
        string_vec words;
        std::vector<int> data;

        vec_insert_fixture(const string_vec & words) : words(words) { }

        virtual void setup(const boost::any & v) {
            auto n = boost::any_cast<int>(v);

            std::random_device rd;
            std::default_random_engine e(rd());

            std::uniform_int_distribution<int> uniform_dist(0, words.size() - 1);
            data.clear();

            std::generate_n(std::inserter(data, std::end(data)), n,
                    [&] { return uniform_dist(e);});
        }

        virtual void go() {
            string_vec res;

            std::transform(std::begin(data), std::end(data), std::inserter(res, std::end(res)),
                    [&] (int i) { return words[i]; });
            std::sort(std::begin(res), std::end(res));
            result(&res);
        }
    };

    struct vec_insert_fixture_sorteach : vec_insert_fixture {
        vec_insert_fixture_sorteach(const string_vec & words) : vec_insert_fixture(words) { }
        virtual void go() {
            string_vec res;

            std::for_each(std::begin(data), std::end(data), 
                    [&] (int i) { 
                        res.emplace_back(words[i]);
                        if (i % 10 == 0) 
                            std::sort(std::begin(res), std::end(res));
                     });
            result(&res);
        }
    };

    template<typename T>
    struct multiset_insert_fixture_t : vec_insert_fixture {
        multiset_insert_fixture_t(const string_vec & words) : vec_insert_fixture(words) { }

        virtual void go() {
            T res;

            std::transform(std::begin(data), std::end(data), std::inserter(res, std::end(res)),
                    [&] (int i) { return words[i]; });
            result(&res);
        }
    };

    typedef std::pair<std::string, std::string> string_pair;
    struct vec_range_fixture : mbm::fixture {
        string_vec words;
        std::vector<string_pair> data;

        vec_range_fixture(const string_vec & words) : words(words) { }

        virtual void setup(const boost::any & v) {
            auto n = boost::any_cast<int>(v);
            
            std::random_device rd;
            std::default_random_engine e(rd());

            std::uniform_int_distribution<int> uniform_dist(0, words.size() - 1);
            data.clear();
            
            std::generate_n(std::inserter(data, std::end(data)), n,
                    [&] { 
                        auto ix1 = uniform_dist(e);
                        auto ix2 = uniform_dist(e);
                        auto k1 = words[ix1];
                        auto k2 = words[ix2];

                        return k1 < k2 ? std::make_pair(k1, k2) : std::make_pair(k2, k1);
                    });
        }

        virtual void go() {
            auto find_ct = 0;
            std::for_each(std::begin(data), std::end(data),
                    [&](decltype(*std::end(data)) _) { 
                        auto l = std::lower_bound(std::begin(words), std::end(words), _.first);
                        auto u = std::upper_bound(std::begin(words), std::end(words), _.second);
                        if (u - l > 0) find_ct++;
                    });
            result(&find_ct);
        }
    };

    struct vec_fixture : mbm::fixture {
        string_vec words;
        string_vec data;

        vec_fixture(const string_vec & words) : words(words) { }

        virtual void setup(const boost::any & v) {
            auto n = boost::any_cast<int>(v);

            std::random_device rd;
            std::default_random_engine e(rd());

            std::uniform_int_distribution<int> uniform_dist(0, words.size() - 1);
            data.clear();
            
            std::generate_n(std::inserter(data, std::end(data)), n,
                    [&] { 
                        auto ix = uniform_dist(e);
                        return words[ix]; 
                    });
        }

        virtual void go() {
            auto find_ct = 0;
            std::for_each(std::begin(data), std::end(data),
                    [&](decltype(*std::end(data)) _) { 
                        if (std::binary_search(std::begin(words), std::end(words), _)) 
                            find_ct++;
                    });
            result(&find_ct);
        }
    };

    template<typename T>
    struct set_fixture_t : vec_fixture {
        T word_set;

        set_fixture_t(const string_vec & words) : vec_fixture(words) { }

        virtual void setup (const boost::any & v) {
            vec_fixture::setup(v);
            word_set.insert(std::begin(words), std::end(words));
        }

        virtual void go() {
            auto find_ct = 0;
            std::for_each(std::begin(data), std::end(data),
                    [&](decltype(*std::end(data)) _) { 
                        find_ct += word_set.count(_); });
            result(&find_ct);
        }
    };

    template<typename T>
    struct set_range_fixture_t : vec_range_fixture {
        T word_set;

        set_range_fixture_t(const string_vec & words) : vec_range_fixture(words) { }

        virtual void setup (const boost::any & v) {
            vec_range_fixture::setup(v);
            word_set.insert(std::begin(words), std::end(words));
        }

        struct func {
            T & word_set;
            size_t ct;
            func(T & word_set) : word_set(word_set), ct(0) { }

            void operator()(const string_pair & pair) {
                auto l = word_set.lower_bound(pair.first);
                auto u = word_set.upper_bound(pair.second);
                if (l != word_set.end() && u != word_set.end()) ct++;
            }
        };

        virtual void go() {
            func f(word_set);
            std::for_each(std::begin(data), std::end(data), f);
            result(&f.ct);
        }
    };

    using multiset_insert_fixture = multiset_insert_fixture_t<std::multiset<std::string>>;
    using btree_multiset_insert_fixture = multiset_insert_fixture_t<btree::btree_multiset<std::string>>;
    using set_fixture = set_fixture_t<std::set<std::string>>;
    using unordered_set_fixture = set_fixture_t<std::unordered_set<std::string>>;
    using btree_set_fixture = set_fixture_t<btree::btree_set<std::string>>;
    using set_range_fixture = set_range_fixture_t<std::set<std::string>>;
    using btree_set_range_fixture = set_range_fixture_t<btree::btree_set<std::string>>;
}
#endif // MBM_MAPS_HPP_
