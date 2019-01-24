#include "papi_wrapper.h"
extern "C"
{
#include <papi.h>
}
#include <stdexcept>

namespace benchmark {
namespace internal {

namespace {

  void Init()
  {
    auto inited = PAPI_is_initialized();
    switch (inited)
    {
    case PAPI_NOT_INITED:
      if (PAPI_VER_CURRENT != PAPI_library_init(PAPI_VER_CURRENT))
        throw std::runtime_error("Could not init papi library");
      if (PAPI_OK != PAPI_thread_init(pthread_self))
        throw std::runtime_error("Could not init papi library");
      break;
    case PAPI_LOW_LEVEL_INITED:
    case PAPI_HIGH_LEVEL_INITED:
      if (PAPI_OK != PAPI_thread_init(pthread_self))
        throw std::runtime_error("Could not init papi library");
    case PAPI_THREAD_LEVEL_INITED:
      break;
    }
  }

}

Papi::Papi(std::vector<int> events)
    : events_(PAPI_NULL)
    , counters_(events.size())
{
  Init();
  if (PAPI_OK != PAPI_create_eventset(&events_))
    throw std::runtime_error("Could not create papi eventset");
  for (const auto event: events)
  {
    char name[PAPI_MAX_STR_LEN];
    if (PAPI_OK != PAPI_event_code_to_name(event, name))
      throw std::runtime_error("Could not get event name");
    eventNames_.push_back(std::string(name).substr(5));
    if (PAPI_OK != PAPI_add_event(events_, event))
      throw std::runtime_error("Could not add events to papi eventset");
  }
}

Papi::~Papi()
{
  PAPI_cleanup_eventset(events_);
  PAPI_destroy_eventset(&events_);
}

bool Papi::StartEvents()
{
  return PAPI_OK == PAPI_start(events_);
}

bool Papi::StopEvents()
{
  if (PAPI_OK != PAPI_accum(events_, counters_.data()))
    return false;
  std::vector<long long> dummies(counters_.size());
  return PAPI_OK == PAPI_stop(events_, dummies.data());
}

void Papi::IncrementCounters(UserCounters& counters) const
{
  for (std::size_t i = 0; i < counters_.size(); ++i)
  {
    auto it = counters.find(eventNames_[i]);
    if (it == counters.end())
      counters.emplace(eventNames_[i], Counter(counters_[i], Counter::kAvgIterations));
    else
      it->second.value += counters_[i];
  }
}

std::vector<int> Papi::GetAvailableEvents()
{
  Init();
  std::vector<int> events;
  int eventCode = PAPI_TOT_INS;
  for (auto result = PAPI_enum_event(&eventCode, PAPI_ENUM_FIRST);
       result != PAPI_ENOEVNT;
       result = PAPI_enum_event(&eventCode, PAPI_PRESET_ENUM_AVAIL))
  {
    if (result != PAPI_OK)
      throw std::runtime_error("Could not read papi events");
    events.push_back(eventCode);
  }
  events.resize(5);
  return events;
}

std::vector<int> Papi::GetSpecifiedEvents(const std::string& input)
{
  if (input.empty())
    return {};
  if (input == "all")
    return GetAvailableEvents();
  Init();
  std::vector<int> events;
  std::string::size_type start = 0;
  for (;;)
  {
    auto next = input.find(',', start);
    const auto name = "PAPI_" + input.substr(start, next-start);
    int code = 0;
    if (PAPI_OK != PAPI_event_name_to_code(name.data(), &code))
      throw std::runtime_error("Unknown PAPI event: '" + name + "'");
    events.push_back(code);
    if (next >= input.size())
      break;
    start = next + 1;
  }
  return events;
}

}  // namespace internal
}  // namespace benchmark
