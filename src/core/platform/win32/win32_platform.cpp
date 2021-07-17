#include "core/platform/platform.h"

#if PLATFORM_WINDOWS

// TODO: define window lean and mean or whatever...
#include <Windows.h>

void* copy_memory(void* dest, const void* src, size_t size)
{
	return memcpy(dest, src, size);
}

#endif // PLATFORM_WINDOWS