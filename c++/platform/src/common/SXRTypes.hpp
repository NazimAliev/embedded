#ifndef SXRTYPES_HPP
#define SXRTYPES_HPP

#include <cstdint>
#include <dlt/injector/DltInjector.hpp>
#include "GateWayTypes.hpp"

namespace sendxreceive
{
enum class SXRError : uint8_t
{
    SUCCESS = 0,
    GENERAL_ERROR = 1
};

}  // namespace sendxreceive

#endif
