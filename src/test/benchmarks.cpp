// =====================================================================================
// 
//       Filename:  veclist.cpp
// 
//    Description:  Microbenchmarking various operations 
// 
//        Created:  03/18/2013 10:18:36 AM
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

#include "sortvsqsort.hpp"
#include "listvsvect.hpp"
#include "copyvsref.hpp"
#include "maps.hpp"

#include <mbm/mbm.hpp>

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <stdexcept>

int main(int argc, char **argv) {
    mbm::suite suite;
    try {
        auto counts = { 10, 100, 1000 };
//        auto counts2 = { 10, 100, 1000, 10000, 100000 }; 
//        suite.add("qsort vs std::sort") 
//            ("qsort", [] { return new qsort_fixture(); }, counts)
//            ("std::sort", [] { return new sort_fixture(); }, counts);
//        suite.add("list vs vector")
//            ("list", [] { return new lv::list_fixture(); }, counts2)
//            ("vector", [] { return new lv::vect_fixture(); }, counts2);
//        suite.add("copy vs const ref")
//            ("copy", [] { return new cr::copy_fixture(); }, counts)
//            ("const ref", [] { return new cr::ref_fixture(); }, counts)
//            ("copy str", [] { return new cr::copy_str_fixture(); }, counts)
//            ("const str ref", [] { return new cr::ref_str_fixture(); }, counts);


        auto words = maps::read_words();        
        suite.add("maps and sets")
            ("insert - sorted vector", [&] { return new maps::vec_insert_fixture(words); }, counts)
            ("insert - sorted vector (resort each)", [&] { return new maps::vec_insert_fixture_sorteach(words); }, counts)
            ("insert - multiset", [&] { return new maps::multiset_insert_fixture(words); }, counts)
            ("insert - btree multiset", [&] { return new maps::btree_multiset_insert_fixture(words); }, counts)
            ("range - btree set", [&] { return new maps::btree_set_range_fixture(words); }, counts)
            ("range - sorted vector", [&] { return new maps::vec_range_fixture(words); }, counts)
            ("range - STL set", [&] { return new maps::set_range_fixture(words); }, counts)
            ("lookup - btree set", [&] { return new maps::btree_set_fixture(words); }, counts)
            ("lookup - sorted vector", [&] { return new maps::vec_fixture(words); }, counts)
            ("lookup - STL set", [&] { return new maps::set_fixture(words); }, counts)
            ("lookup - STL unordered set", [&] { return new maps::unordered_set_fixture(words); }, counts);

        suite.parse_cmdline_opts(argc, argv);
        suite.run();
    } catch (const mbm::usage_error & e) {
        std::string what(e.what());
        if (!what.empty())
            std::cerr << "Error - " << what << std::endl;
        std::cout << "Usage: benchmarks [opts]\n" << suite.cmdline << std::endl;
        return static_cast<int>(!what.empty());
    } catch (const std::exception & e) {
        std::cout << "Error - " << e.what() << std::endl;
        return 255;
    }
    return 0;
}
