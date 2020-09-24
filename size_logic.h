#ifndef _NCMALLOC_SIZE_LOGIC_H_
#define _NCMALLOC_SIZE_LOGIC_H_

#define MIN(a, b) (a) < (b) ? (a) : (b)

namespace NCMalloc
{
	static const size_t MAX_SMALL_SIZE = 1024;
	static const size_t MAX_SIZE_IN_THREAD = 256 * 1024;

	static const size_t NUM_SIZE_INDEXS = 
		((MAX_SIZE_IN_THREAD + 127 + (120 << 7)) >> 7) + 1;

	static const size_t NUM_MAX_CLASSES = 128;

	static const size_t PAGE_SHIFT = 13;
	static const size_t PAGE_SIZE = 1 << PAGE_SHIFT;// 1 << 13 = 8KB

	// Split sizes into different classes.
	class SizeMap
	{
		/*
		* Sizes : object sizes, round up with 8 when <= 1024, or 128.
		* SizeIndex : indexs for sizes above.
		* SizeClasses : split sizes above into different classes.
		*/
		
		// Stores each class's size.
		int mClassSizes[NUM_MAX_CLASSES];

		// Stores class id which this size(mapped to index) belong.
		int mSizeClasses[NUM_SIZE_INDEXS];

		// Stores object batch size for a class
		// (transfering between ThreadCache and CentralCache).
		int mClassBatchSize[NUM_MAX_CLASSES];

		// Stores each class's page num.
		int mClassPageNum[NUM_MAX_CLASSES];

		size_t mSizeClassNum;

	public:

		SizeMap() = delete;

		void Init();

		/*
		* Size indexs, set all sizes with a size index.
		* This size index can be used in mSizeClasses to get class id.
		* size	:	0, 1, ..., 7, 8, 9, ..., 1024, 1025, 1026, ..., 256 * 1024.
		* index	:	0, 1, ..., 1, 1, 2, ..., 128,  120+9, 129, ..., 120+2048.
		*/

		// Round up class size for smaller(<=1024) with 8.
		static inline size_t IndexSmall(size_t siz)
		{
			return (static_cast<unsigned int>(siz) + 7) >> 3;
		}
		// Round up class size for largger(>1024) with 128.
		static inline size_t IndexLarge(size_t siz)
		{
			return (static_cast<unsigned int>(siz) + 127 + (120 << 7)) >> 7;
		}
		// Transfer size into size indexs. 
		static inline size_t SizeIndex(size_t siz)
		{
			if (siz <= MAX_SMALL_SIZE)
			{
				return IndexSmall(siz);
			}
			else
			{
				return IndexLarge(siz);
			}
		}

		// Alignment for sizes to split into size classes.
		static int SizeAlignment(size_t siz);

		// Calculate batch size for siz between central and thread cache.
		static int BatchSize(size_t siz);

		/* Getters. */

		// From size(index) to class id.
		inline int ClassIndexForSize(size_t siz_index)
		{
			return mSizeClasses[siz_index];
		}

		// From class id to size.
		inline int SizeForClass(size_t cls)
		{
			return mClassSizes[cls];
		}

		// From class id to batch size(between central cache and thread cache).
		inline int BatchSizeForClass(size_t cls)
		{
			return mClassBatchSize[cls];
		}

		inline int PageNumForClass(size_t cls)
		{
			return mClassPageNum[cls];
		}
		
	};

	

}

#endif // !_NCMALLOC_SIZE_LOGIC_H_
