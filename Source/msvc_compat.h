// MSVC compatibility shims for GCC/Clang extensions used in third-party headers.
#pragma once
#ifdef _MSC_VER
#  ifndef __attribute__
#    define __attribute__(x)
#  endif
#endif
