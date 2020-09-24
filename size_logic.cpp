#include "size_logic.h"

void NCMalloc::SizeMap::Init()
{
	// Split sizes into size classes(with alignment in func SizeAlignment(size)).
	int sc = 1;
	int alignment = 8;// alignment to get next size.
	for (size_t size = 8; size <= MAX_SIZE_IN_THREAD; size += alignment)
	{
		alignment = SizeAlignment(size);

		int batch_size = BatchSize(size) / 4;//???
		size_t psize = 0;
		{
			do {
				psize += PAGE_SIZE;

				// Make pages total size leftover less than 1/8 of total.
				while ((psize % size) > (psize >> 3))
				{
					psize += PAGE_SIZE;
				}
			} while ((psize % size) < (batch_size));
		}
		const size_t pages = psize >> PAGE_SHIFT;

		// If this class have same page and same obj num with last class,
		// Merge it into last class.
		if (sc > 1 && pages == mClassPageNum[sc - 1]) {
			const size_t my_objects = (pages << PAGE_SHIFT) / size;
			const size_t prev_objects = (mClassPageNum[sc - 1] << PAGE_SHIFT)
				/ mClassSizes[sc - 1];
			if (my_objects == prev_objects) {
				// Adjust last class to include this size.
				mClassSizes[sc - 1] = size;
				continue;// Do not add this class
			}
		}

		mClassPageNum[sc] = pages;
		mClassSizes[sc] = size;
		sc++;
	}
	mSizeClassNum = sc;

	// Get each size's size-class index.
	int size = 0;
	for (int cls = 1; cls < mSizeClassNum; cls++)
	{
		const int max_size_for_class = mClassSizes[cls];
		for (int s = size; s <= max_size_for_class; s += 8)
		{
			// All sizes under ClassSizes[cls] belong to this size class.
			mSizeClasses[SizeIndex(s)] = cls;
		}
		size = max_size_for_class + 8;// Next size section.

		mClassBatchSize[cls] = SizeForClass(cls);
	}
}

// Helper function to get size's biggest bit.
static inline int LogFloor(size_t n)
{
	// Get n's max bit.
	int log = 0;
	for (int i = 4; i >= 0; --i)
	{
		int shift = (1 << i);// 16 8 4 2 1, combines get log;
		size_t x = n >> shift;
		if (x != 0)
		{
			n = x;
			log += shift;
		}
	}

	return log;
}

int NCMalloc::SizeMap::SizeAlignment(size_t size)
{
	int min_alignment = 16;
	int alignment = 8;
	if (size > MAX_SIZE_IN_THREAD)// Bigger than thread max size.
	{
		// For pagesize.
	}
	else if (size >= 128)// [128, 256*1024) : 
	{
		alignment = (1 << (LogFloor(size) - 3));// Biggest bit value/8.
	}
	else if (size >= min_alignment)// [16, 128) : 16.
	{
		alignment = min_alignment;
	}
	else// [0, 16) : 8.
	{
		alignment = 8;
	}

	return alignment;
}

int NCMalloc::SizeMap::BatchSize(size_t siz)
{
	if (siz == 0) return 0;
	int num = static_cast<int>(64 * 1024 / siz);// Transfer 64k every time.
	if (num < 2) num = 2;// At least 2 obj.

	//TODO : shrink.
	return num;
}

