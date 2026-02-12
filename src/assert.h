#pragma once

#include <stdexcept>
#include <string>

#define ASSERT_IMPL(cond)                                                                                         \
  do {                                                                                                            \
    if (!(cond)) {                                                                                                \
      throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": condition '" + #cond + \
                               "' is not satistfied");                                                            \
    }                                                                                                             \
  } while (false)

#define ASSERT_WITH_MESSAGE_IMPL(cond, message)                                                                   \
  do {                                                                                                            \
    if (!(cond)) {                                                                                                \
      throw std::runtime_error(std::string(__FILE__) + ":" + std::to_string(__LINE__) + ": condition '" + #cond + \
                               "' is not satistfied: " + (message));                                              \
    }                                                                                                             \
  } while (false)

#ifdef NDEBUG
#define ASSERT(cond)
#else
#define ASSERT(cond) ASSERT_IMPL(cond)
#endif

#ifdef NDEBUG
#define ASSERT_WITH_MESSAGE(cond, message)
#else
#define ASSERT_WITH_MESSAGE(cond, message) ASSERT_WITH_MESSAGE_IMPL(cond, message)
#endif