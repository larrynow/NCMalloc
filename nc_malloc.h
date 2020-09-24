#ifndef _NC_MALLOC_H_
#define _NC_MALLOC_H_

#include<assert.h>

namespace NCMalloc
{
	using PageNum = unsigned long long;
	using PageID = unsigned long long;

	static const size_t MAX_SMALL_SIZE = 1024;
	static const size_t MAX_SIZE_IN_THREAD = 256 * 1024;

	static const size_t NUM_SIZE_INDEXS =
		((MAX_SIZE_IN_THREAD + 127 + (120 << 7)) >> 7) + 1;

	static const size_t NUM_MAX_CLASSES = 128;

	static const size_t PAGE_SHIFT = 13;
	static const size_t PAGE_SIZE = 1 << PAGE_SHIFT;// 1 << 13 = 8KB

	static const size_t MAX_PAGES = 1 << (20 - PAGE_SHIFT);

	// From ptr get its pageID.
	static inline PageID GetPageID(void* ptr)
	{
		return reinterpret_cast<PageID>(ptr) >> PAGE_SHIFT;
	}
}

#endif // !_NC_MALLOC_H_
