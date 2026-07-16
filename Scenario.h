#ifndef IOT_STUDY_SCENARIO_H
#define IOT_STUDY_SCENARIO_H

#include <cstdint>

struct ScenarioParameters {
  uint32_t nodes = 200;
  double duration = 300.0;
  uint32_t payload = 256;
};

#endif // IOT_STUDY_SCENARIO_H
