#include "benchmark_api_internal.h"

namespace benchmark {
namespace internal {

State BenchmarkInstance::Run(
    size_t iters, int thread_id, internal::ThreadTimer* timer, internal::Papi* papi,
    internal::ThreadManager* manager) const {
  State st(iters, arg, thread_id, threads, timer, papi, manager);
  benchmark->Run(st);
  return st;
}

}  // internal
}  // benchmark
