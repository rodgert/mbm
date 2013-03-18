// =====================================================================================
// 
//       Filename:  opt.hpp
// 
//    Description:  Core measurement functions
// 
//        Version:  1.0
//        Created:  03/18/2013 01:03:52 PM
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
#ifndef MBM_OPT_HPP_
#define MBM_OPT_HPP_

#include <string>

#include <boost/program_options.hpp>
#include <boost/make_shared.hpp>

namespace mbm {
namespace opt {
    struct opt_t {
        const char * name;
        const char * descr;
    };

    struct easy_init {
	typedef boost::program_options::value_semantic value_semantic;
        typedef boost::program_options::option_description option_description;

        boost::program_options::options_description & options;
        easy_init(boost::program_options::options_description & options) : options(options) { }

        easy_init & operator()(const opt_t & opt) {
            auto v = new boost::program_options::untyped_value(true);
            options.add(boost::make_shared<option_description>(opt.name, v, opt.descr));
            return *this;
        }

        easy_init & operator()(const opt_t & opt, const value_semantic * v) {
            options.add(boost::make_shared<option_description>(opt.name, v, opt.descr));
            return *this;
        }
    };

    easy_init add(boost::program_options::options_description & options) {
        return easy_init(options);
    }

    std::string long_name(const opt_t & opt) {
        std::string name(opt.name);
        auto n = name.find(',');
        return n != std::string::npos ? name.substr(0, n) : name;
    }

    const opt_t help = { "help,h", "produce help message" };
    const opt_t version = { "version,V", "report version" };
    const opt_t verbose = { "verbose,v", "logging verbosity" };
}
}
#endif // MBM_OPT_HPP_
