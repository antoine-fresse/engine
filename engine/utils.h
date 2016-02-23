#pragma once

#ifdef ASSERT
#undef ASSERT
#endif

#include <iostream>


inline int assert_handler(const char* expr, const char* file, int line)
{
	std::cout << "Expression : " << expr << " failed at line" << line << " in " << file << std::endl;
	return 1;
}

inline void assert_block()
{
	for (;;){}
}

#if defined(_DEBUG) || defined(DEBUG)
#define ASSERT(x) ((void)(!(x) && assert_handler(#x, __FILE__, __LINE__) && (assert_block(), 1)))
#else
#define ASSERT(x) ((void)sizeof(x))
#endif


using uint8 = uint8_t;
using int8 = int8_t;

using uint16 = uint16_t;
using int16 = int16_t;

using uint32 = uint32_t;
using int32 = int32_t;

using uint64 = uint64_t;
using int64 = int64_t;

using float32 = float;
using float64 = double;