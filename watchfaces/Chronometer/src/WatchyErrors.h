#pragma once

#include "../../../src/watchy/Watchy.h"

namespace Watchy {

typedef enum {
  OK = 0,           // success
  TIMEOUT = 1,      // request timed out
  WIFI_FAILED = 2,  // couldn't connect to wifi
  NOT_READY = 3,  // returned a cached value because the result is still pending
  RATE_LIMITED = 4,    // returned a cached value beacuse of a rate limit
  REQUEST_FAILED = 5,  // the network request had an error
} Error;

extern Error err;
};  // namespace Watchy
