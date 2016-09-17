#include "aligned_malloc.hpp"

#if defined(__linux__) || defined(EMSCRIPTEN)
#include <stdlib.h>
#include <malloc.h>
#endif

void * malloc_simd(const size_t size) 
{
	const size_t alignment = 16;
#if defined(__linux__) || defined(EMSCRIPTEN)
#ifdef _ISOC11_SOURCE
	return aligned_alloc(alignment, size);
#else
    return memalign(alignment, size);
#endif
#elif defined(WIN32)
	return _aligned_malloc(size, alignment);
#else
    #error Unsupported platform
#endif
}

void free_simd(void * ptr) 
{
#if defined(__linux__) || defined(EMSCRIPTEN)
    free(ptr);
#elif defined(WIN32)
	_aligned_free(ptr);
#else
    #error Unsupported platform
#endif
}

