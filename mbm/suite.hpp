// =====================================================================================
// 
//       Filename:  suite.hpp
// 
//    Description:  Define a benchmark suite
// 
//        Version:  1.0
//        Created:  03/18/2013 11:20:55 AM
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
#ifndef MBM_SUITE_HPP_
#define MBM_SUITE_HPP_

#include "measure.hpp"
#include "opt.hpp"

#include <boost/assert.hpp>
#include <boost/scope_exit.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/any.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <cstring>
#include <cmath>
#include <regex>
#include <stdexcept>
#include <cstdint>
#include <sched.h>
#include <unistd.h>
#include <functional>
#include <map>
#include <set>
#include <vector>
#include <limits>
#include <algorithm>
#include <iterator>
#include <initializer_list>

namespace mbm {
    struct fixture {
        virtual void setup() { }
        virtual void setup(const boost::any&) { }
        virtual void teardown() { }

        virtual void fixture_setup() { }
        virtual void fixture_teardown() { }

        template<typename T>
        void result(T && t) {
            asm volatile("" : "+r" (t));
        }

        virtual void go() = 0;
    };

    namespace detail {
        struct fixture_runner {
            typedef std::vector<uint64_t> run_res_t;
            typedef std::vector<std::pair<std::string, run_res_t>> run_table_t;

            explicit fixture_runner(const std::string & group, bool use_rdtsc, std::function<fixture*(void)> factory) : 
                group_(group),
                use_rdtsc_(use_rdtsc), 
                factory_(std::move(factory)) { } 

            template<typename T>
            explicit fixture_runner(const std::string & group, bool use_rdtsc, std::function<fixture*(void)> factory, std::initializer_list<T> table_data) :
                group_(group),
                use_rdtsc_(use_rdtsc),
                factory_(std::move(factory)),
                generator_(new model<T>(std::move(table_data))) { }

            fixture_runner(const fixture_runner & other) :
                group_(other.group_),
                use_rdtsc_(other.use_rdtsc_), 
                factory_(other.factory_) {
                    if (other.generator_)
                        generator_.reset(other.generator_->clone());
                }

            fixture_runner() = default;

            fixture_runner & operator=(const fixture_runner & rhs) {
                group_ = rhs.group_;
                use_rdtsc_ = rhs.use_rdtsc_;
                factory_ = rhs.factory_;
                if (rhs.generator_)
                    generator_.reset(rhs.generator_->clone());
                return *this;
            }

            std::unique_ptr<fixture> setup() const {
                std::unique_ptr<fixture> res(factory_());
                res->fixture_setup();
                if (use_rdtsc_)
                    dummy_read_tsc(std::true_type());
                else
                    dummy_read_tsc(std::false_type());
                return res;
            }

            void teardown(std::unique_ptr<fixture> & fixture) const {
                fixture->fixture_teardown();
            }

            std::string group() const { return group_; }
            bool is_table() const { return generator_.get() != nullptr; }

            run_table_t run_table(unsigned numruns, std::unique_ptr<fixture> & fixture) const {
                BOOST_ASSERT(is_table());
                run_table_t res;
                res.reserve(generator_->size());
                std::generate_n(std::inserter(res, std::begin(res)), generator_->size(),
                        [&] { 
                            auto v = generator_->next();
                            return std::make_pair(generator_->to_string(v), run(numruns, fixture, v));
                        });
                return res;
            }

            run_res_t run(unsigned numruns, std::unique_ptr<fixture> & fixture, boost::any v = boost::any()) const {
                run_res_t res; 
                res.reserve(numruns);
                std::generate_n(std::inserter(res, std::begin(res)), numruns, [&] { return go(fixture, v); });
                return res;
            }

        private:
            struct concept {
                virtual ~concept() { }

                virtual concept* clone() const = 0;
                virtual size_t size() const = 0;
                virtual std::string to_string(const boost::any &) const = 0;
                virtual boost::any next() = 0;
            };

            template<typename T> 
            struct model : concept {
                typedef std::initializer_list<T> list_t;
                model(list_t t) : 
                    data(std::move(t)),
                    cur(std::begin(data)) { }

                virtual concept* clone() const { return new model<T>(*this); }
                virtual size_t size() const { return data.size(); }
                virtual std::string to_string(const boost::any & v) const {
                    BOOST_ASSERT(!v.empty());
                    auto vv = boost::any_cast<T>(v);
                    return boost::lexical_cast<std::string>(vv);
                }

                virtual boost::any next() { return cur != std::end(data) ? boost::any(*cur++) : boost::any(); }

                list_t data;
                typename list_t::iterator cur;
            };

            uint64_t go(std::unique_ptr<fixture> & fixture, boost::any v) const {
                if (v.empty())
                    fixture->setup();
                else
                    fixture->setup(v);

                BOOST_SCOPE_EXIT(&fixture) {
                    fixture->teardown();
                } BOOST_SCOPE_EXIT_END
                return use_rdtsc_ ? go(fixture, std::true_type()) : go(fixture, std::false_type());
            }

            template<typename T>
            uint64_t go(std::unique_ptr<fixture> & fixture, T t) const {
                auto t0 = read_initial_tsc(); 
                fixture->go();
                auto t1 = read_final_tsc(t);
                return t1 - t0;
            }

            template<typename T>
            void dummy_read_tsc(T t) const {
                read_initial_tsc();
                read_final_tsc(t);
                read_final_tsc(t);
                read_final_tsc(t);
            }

            std::string group_;
            bool use_rdtsc_;
            std::function<fixture*(void)> factory_;
            std::unique_ptr<concept> generator_;
        };

        struct easy_init {
            typedef std::map<std::string, fixture_runner> fixture_map_t;
            fixture_map_t & fixtures;
            bool use_rdtsc;
            std::string group;

            easy_init(fixture_map_t & fixtures, bool use_rdtsc, const std::string & group) : 
                fixtures(fixtures), use_rdtsc(use_rdtsc), group(group) { }

            easy_init & operator()(const std::string & name, std::function<fixture*(void)> factory) { 
                fixtures[name] = fixture_runner(group, use_rdtsc, std::move(factory));
                return *this;
            }

            template<typename T>
            easy_init & operator()(const std::string & name, std::function<fixture*(void)> factory, std::initializer_list<T> table_data) { 
                fixtures[name] = fixture_runner(group, use_rdtsc, std::move(factory), std::move(table_data));
                return *this;
            }
        };

        struct empty_fixture : fixture {
            int dummy;
            virtual void go() { result(&dummy); }
        };
    }

    namespace opt {
        const opt_t pincore { "pincore", "Pin execution to specified core" };
        const opt_t numruns { "numruns,n", "Average execution times over numruns" };
        const opt_t filter { "filter,f", "Filter benchmarks, running only those matching a regexp" };
        const opt_t group { "group,g", "Filter benchmark groups, running only those matching a regexp" };
        const opt_t rdtsc { "rdtsc", "Force use of rdtsc even if CPU supports rdtscp" };
    }

    struct usage_error : std::runtime_error {
        explicit usage_error(const std::string & what = std::string()) : std::runtime_error(what) { }
    };

    class suite {
    public:
        typedef std::vector<std::string> strs_t;

        boost::program_options::options_description cmdline;

        bool version;
        bool verbose;
        int pincore;
        unsigned numruns;
        strs_t filters;
        strs_t groups;
        bool use_rdtsc;

        suite(bool add_generic_opts = true) {
            namespace po = boost::program_options;
            if (add_generic_opts) {
                po::options_description generic("Generic Options");
                opt::add(generic)
                    (opt::help)
                    (opt::version, po::value<bool>(&version)
                        ->implicit_value(true)
                        ->default_value(false))
                    (opt::verbose, po::value<bool>(&verbose)
                        ->implicit_value(true)
                        ->default_value(false));
                cmdline.add(generic);
            }

            po::options_description benchmark("Benchmark Options"); 
            opt::add(benchmark)
                (opt::pincore, po::value<int>(&pincore)->default_value(0))
                (opt::numruns, po::value<unsigned>(&numruns)->default_value(128u))
                (opt::filter, po::value<strs_t>(&filters)
                    ->default_value(strs_t(), ""))
                (opt::rdtsc, po::value<bool>(&use_rdtsc)
                    ->implicit_value(true)
                    ->default_value(!has_rdtscp()));
            cmdline.add(benchmark);
        }

        void parse_cmdline_opts(int argc, char **argv) { 
            auto parser = boost::program_options::command_line_parser(argc, argv)
                            .options(cmdline);
            parse_cmdline_opts(parser);
        }

        void parse_cmdline_opts(boost::program_options::command_line_parser parser) {
            namespace po = boost::program_options;
            po::variables_map vm;
            po::store(parser.run(), vm);
            vm.notify();

            if (vm.count(opt::long_name(opt::help))) throw usage_error();

            if (verbose) std::cout << "Iterations: " << numruns << std::endl;
            if (verbose) std::cout << "Using rdtsc: " << (use_rdtsc ? "yes" : "no") << std::endl;
        }

        void add(const std::string & name, std::function<fixture*(void)> factory) { 
            fixtures_[name] = detail::fixture_runner("", use_rdtsc, std::move(factory));
        }

        template<typename T>
        void add(const std::string & name, std::function<fixture*(void)> factory, std::initializer_list<T> table_data) { 
            fixtures_[name] = detail::fixture_runner("", use_rdtsc, std::move(factory), std::move(table_data));
        }

        detail::easy_init add(const std::string & group) {
            return detail::easy_init(fixtures_, use_rdtsc, group);
        }
        
        void run() const {
            set_affinity();
            auto overhead = compute_overhead();
            auto filtered = filter_fixtures();
            if (!filtered.size()) {
                std::cout << "No fixtures matched filter specs" << std::endl;
                return;
            }
            run(overhead, filtered);
        }

    private:
        typedef detail::fixture_runner::run_res_t run_res_t;
        typedef detail::fixture_runner::run_table_t run_table_t;
        typedef std::vector<std::regex> filters_t;
        typedef std::set<std::string> fixture_names_t;
        typedef std::map<std::string, detail::fixture_runner> fixture_map_t;
        fixture_map_t fixtures_;

        void set_affinity() const {
            cpu_set_t cpus;
            CPU_ZERO(&cpus);
            CPU_SET(pincore, &cpus);
            if (sched_setaffinity(0, 1, &cpus)) 
               throw std::runtime_error(strerror(errno)); 
            if (verbose) std::cout << "Pinning to core: " << pincore << std::endl;
        }

        uint64_t compute_overhead() const {
            if (verbose) std::cout << "Computing loop overhead..." << std::flush;
            uint64_t res = std::numeric_limits<uint64_t>::max();
            detail::fixture_runner empty_runner("", use_rdtsc, [] { return new detail::empty_fixture(); });
            for (auto i = 0u; i < 1000000u / numruns; i++) {
                auto runres = run(empty_runner);
                runres.emplace_back(res);
                res = *(std::min_element(std::begin(runres), std::end(runres)));
            }
            if (verbose) std::cout << "Done. " << res << "clk" << std::endl;
            return res;
        }

        fixture_map_t filter_fixtures() const {
            auto res = filter(groups, fixtures_, 
                          [&](const filters_t & filters, const fixture_map_t::value_type & v) {
                                return should_run(filters, v.second.group());
                          });
                            
            return filter(filters, res, 
                          [&](const filters_t & filters, const fixture_map_t::value_type & v) {
                                return should_run(filters, v.first);
                          });
        }
        
        template<typename Predicate>
        static fixture_map_t filter(const strs_t exprs, const fixture_map_t & fixtures, const Predicate & pred) {
            if (exprs.empty()) return fixtures; // run everything

            fixture_map_t res;
            filters_t regexes;
            std::transform(std::begin(exprs), std::end(exprs), 
                    std::inserter(regexes, std::begin(regexes)), parse_regex); 

            std::copy_if(std::begin(fixtures), std::end(fixtures), std::inserter(res, std::begin(res)), 
                    [&](decltype(*std::end(fixtures)) _) { return pred(regexes, _); });
            return res;
        }

        static std::regex parse_regex(const std::string & expr) {
            try {
                return std::regex(expr);
            } catch(...) {
                std::ostringstream stm;
                stm << "Error parsing filter spec " << expr;
                std::throw_with_nested(std::runtime_error(stm.str()));
            }
        }

        static bool should_run(const filters_t & filters, const std::string & name) {
            if (name.empty()) return true; // always run
            auto res = std::find_if(std::begin(filters), std::end(filters),
                            [&](const std::regex & expr) { return std::regex_match(name, expr); });
            return res != filters.end();
        }

        void run(uint64_t overhead, const fixture_map_t & fixtures) const {
            std::cout << "Running benchmarks..." << std::flush;
            std::multimap<std::string, std::string> results;
            std::transform(std::begin(fixtures), std::end(fixtures), 
                    std::inserter(results, std::begin(results)),
                    [&](decltype(*std::end(fixtures_)) _) { 
                        return std::make_pair(_.second.group(), run(overhead, _.first, _.second)); 
                    });
            std::cout << "Done." << std::endl;

            std::string last;
            std::for_each(std::begin(results), std::end(results), [&](decltype(*std::end(results)) r) { 
                        auto group = r.first.empty() ? "Ungrouped" : r.first;
                        if (group != last) 
                            std::cout << std::string(10, '=') << ' ' << group << ' ' << std::string(10, '=') << std::endl;
                        std::cout << r.second << std::endl; 
                        last = group;
                    });
        }

        std::string run(uint64_t overhead, const std::string & name, const detail::fixture_runner & runner) const {
            std::ostringstream res_stm;
            if (runner.is_table()) {
                auto res = run_table(runner);
                std::for_each(std::begin(res), std::end(res), [&](decltype(*std::end(res)) _) {
                            res_stm << name << "(" << _.first << ")";
                            report(res_stm, overhead, _.second);
                            res_stm << '\n';
                        });
            } else {
                auto res = run(runner);
                report(res_stm, overhead, res);
            }
            std::cout << "." << std::flush;
            return res_stm.str();
        }

        static std::string indent(size_t chars = 8) { return std::string(chars, ' '); }
        template<typename Iterator>
        static double compute_stddev(Iterator begin, Iterator end, double avg) {
            double res = std::accumulate(begin, end, 0.0, [avg](double r, decltype(*end) t) {
                        auto offs = t - avg;
                        return r + offs * offs;
                    });
            return sqrt(res / std::distance(begin, end));
        }

        void report(std::ostream & stm, uint64_t overhead, const run_res_t & res) const {
            run_res_t adj;
            std::transform(std::begin(res), std::end(res), std::inserter(adj, std::begin(adj)), 
                    [overhead](uint64_t t) { return t - overhead; });
            auto total = std::accumulate(std::begin(adj), std::end(adj), 0u);
            auto avg = static_cast<double>(total) / numruns;

            auto stddev = compute_stddev(std::begin(adj), std::end(adj), avg);

            run_res_t sorted(adj);
            std::sort(std::begin(sorted), std::end(sorted));
            if (verbose) {
                stm << std::endl
                    << indent() << "      min: " << as_cycles(sorted.front()) << std::endl
                    << indent() << "      avg: " << as_cycles(avg) << std::endl
                    << indent() << "  std_dev: " << as_cycles(stddev) << std::endl
                    << indent() << "     % 75: " << as_cycles(sorted[static_cast<int>(numruns * 0.75)]) << std::endl
                    << indent() << "     % 90: " << as_cycles(sorted[static_cast<int>(numruns * 0.90)]) << std::endl
                    << indent() << "     % 99: " << as_cycles(sorted[static_cast<int>(numruns * 0.99)]) << std::endl
                    << indent() << "      max: " << as_cycles(sorted.back());
            } else {
                stm << ", avg=" << as_cycles(avg) << " +/- " << as_cycles(stddev) << ", range=[" << as_cycles(sorted.front()) 
                    << ", " << as_cycles(sorted.back()) << "]";
            }
        }

        run_table_t run_table(const detail::fixture_runner & runner) const {
            auto fixture = runner.setup();
            BOOST_SCOPE_EXIT(&runner, &fixture) {
                runner.teardown(fixture);
            } BOOST_SCOPE_EXIT_END

            return runner.run_table(numruns, fixture);
        }

        run_res_t run(const detail::fixture_runner & runner) const {
            auto fixture = runner.setup();
            BOOST_SCOPE_EXIT(&runner, &fixture) {
                runner.teardown(fixture);
            } BOOST_SCOPE_EXIT_END

            return runner.run(numruns, fixture);
        }
    };
}
#endif // MBM_SUITE_HPP_
