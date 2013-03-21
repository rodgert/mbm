// =====================================================================================
// 
//       Filename:  veclist.cpp
// 
//    Description:  Microbenchmarking List and Vector operations 
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

#include <mbm/mbm.hpp>

#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <stdexcept>

int main(int argc, char **argv) {
    mbm::suite suite;
    try {
        auto counts = { 1000, 10000, 100000 };
        suite.add("qsort vs std::sort") 
            ("qsort", [] { return new qsort_fixture(); }, counts)
            ("std::sort", [] { return new sort_fixture(); }, counts);

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
