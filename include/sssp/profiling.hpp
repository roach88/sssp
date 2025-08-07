#ifndef SSSP_PROFILING_HPP
#define SSSP_PROFILING_HPP

#include <chrono>
#include <atomic>
#include <iostream>

namespace sssp {

struct ProfCounters {
    std::atomic<long long> basecase_ns{0};
    std::atomic<long long> findpivots_ns{0};
    std::atomic<long long> bmssp_ns{0};
};

inline ProfCounters& prof() {
    static ProfCounters pc;
    return pc;
}

struct ScopeTimer {
    std::chrono::high_resolution_clock::time_point start;
    std::atomic<long long>* sink;
    explicit ScopeTimer(std::atomic<long long>* s) : start(std::chrono::high_resolution_clock::now()), sink(s) {}
    ~ScopeTimer(){
        auto end = std::chrono::high_resolution_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        (*sink) += ns;
    }
};

inline void dump_profile(){
    std::cout << "SSSP profile (ms): basecase=" << prof().basecase_ns.load()/1e6
              << " findpivots=" << prof().findpivots_ns.load()/1e6
              << " bmssp=" << prof().bmssp_ns.load()/1e6 << "\n";
}

} // namespace sssp

#endif
