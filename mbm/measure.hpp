// =====================================================================================
// 
//       Filename:  measure.hpp
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
#ifndef MBM_MEASURE_HPP_
#define MBM_MEASURE_HPP_

#include <boost/date_time/posix_time/posix_time.hpp>

#include <cstdint>
#include <type_traits>
#include <ostream>
#include <sstream>

namespace mbm {
    inline uint64_t read_initial_tsc() {
        union {
            uint64_t ui64;
            uint32_t ui32[2];
        };
        __asm__ __volatile__ ("cpuid\n\trdtsc" : "=a"(ui32[0]), "=d"(ui32[1]) :: "%rbx", "%rcx");
        return ui64;
    }

    inline uint64_t read_final_tsc(std::false_type) {
        union {
            uint64_t ui64;
            uint32_t ui32[2];
        };
        __asm__ __volatile__("rdtscp\n\tmov %%eax, %0\n\tmov %%edx, %1\n\tcpuid" : "=r"(ui32[0]), "=r"(ui32[1]) ::
            "%rax", "%rbx", "%rcx", "%rdx");
        return ui64;
    }

    inline uint64_t read_final_tsc(std::true_type) {
        union {
            uint64_t ui64;
            uint32_t ui32[2];
        };
        __asm__ __volatile__("xorl %%eax, %%eax\ncpuid\n" ::: "%rax", "%rbx", "%rcx", "%rdx");
        __asm__ __volatile__("rdtsc\n" : "=a"(ui32[0]), "=d"(ui32[1]));
        __asm__ __volatile__("xorl %%eax, %%eax\ncpuid\n" ::: "%rax", "%rbx", "%rcx", "%rdx");
        return ui64;
    }

    inline bool has_rdtscp() {
        uint32_t res;
        __asm__ __volatile__("movl $0x80000001, %%eax\ncpuid\n" : "=d"(res) :: "%rax", "%rbx", "%rcx");
        return res & (1 << 27);
    }
    
    inline uint64_t clocks_per_microsecond() {
        static uint64_t clocks;
        if (clocks) return clocks;
        auto now = boost::posix_time::microsec_clock::universal_time();
        boost::posix_time::ptime next;
        while ((next = boost::posix_time::microsec_clock::universal_time()) == now); // spin
        auto tsc = read_initial_tsc();
        next += boost::posix_time::microseconds(1000);
        while (boost::posix_time::microsec_clock::universal_time() < next); // spin
        auto ticks = read_final_tsc(std::false_type()) - tsc;
        return clocks = ticks / 1000;
    }

    struct as_cycles {
        uint64_t cycles;

        as_cycles(uint64_t cycles) : cycles(cycles) { }

        friend std::ostream & operator<<(std::ostream & stm, const as_cycles & that) {
            std::ostringstream str;    
            str << std::fixed << std::setprecision(1) << 
                static_cast<double>(that.cycles) / clocks_per_microsecond() << "us (" <<
                    that.cycles << "clk)";
            return stm << str.str();
        }        
    };
}
#endif // MBM_MEASUER_HPP_
