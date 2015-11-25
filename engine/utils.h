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
