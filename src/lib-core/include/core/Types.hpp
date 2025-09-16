#pragma once

#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#endif

namespace core {
using Int8 = ::std::int8_t;
using Int16 = ::std::int16_t;
using Int32 = ::std::int32_t;
using Int64 = ::std::int64_t;

using UInt8 = ::std::uint8_t;
using UInt16 = ::std::uint16_t;
using UInt32 = ::std::uint32_t;
using UInt64 = ::std::uint64_t;

using Byte = ::std::uint8_t;

using Float32 = float;
using Float64 = double;

using Bool = bool;

#ifdef __linux__
using SocketFD = int;
#elif _WIN32
using SocketFD = SOCKET;
#endif

using AcceptorID = Int32;
using SessionID = Int32;

const SocketFD sINVALID_SOCKET_FD = -1;
const Int32 sINVALID_ID = -1;
} // namespace core