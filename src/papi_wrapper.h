#ifndef BENCHMARK_PAPI_H
#define BENCHMARK_PAPI_H

#include "benchmark/benchmark.h"
#include <vector>
#include <string>

namespace benchmark {
namespace internal {

class Papi
{
public:
  explicit Papi(std::vector<int> events);
  ~Papi();

  bool StartEvents();
  bool StopEvents();

  void IncrementCounters(UserCounters&) const;

  static std::vector<int> GetAvailableEvents();
  static std::vector<int> GetSpecifiedEvents(const std::string& input);

private:
  int events_;
  std::vector<long long> counters_;
  std::vector<std::string> eventNames_;
};

}  // namespace internal
}  // namespace benchmark

#endif  // BENCHMARK_PAPI_H
