#ifndef _NCMALLOC_THREAD_CACHE_H_
#define _NCMALLOC_THREAD_CACHE_H_

#include"size_logic.h"
#include"single_linked_list.h"

#include<thread>

namespace NCMalloc
{
	static const unsigned int NUM_FREE_LIST = NUM_MAX_CLASSES;
	static const int MAX_DYNAMIC_FREELIST_LENGTH = 8192;

	class FreeList
	{
		/*
		* FreeList : Manage cache for one size class. 
		* Use first 4/8 bytes memory in block as a pointer towards next block.
		* list -> | p_nex | obj  |
		*			  ->  | p_nex| obj  |
		*					  -> | nullptr
		*/
	public:

		/* List operations. */

		// Pop first.
		void* Pop()
		{
			void* obj = mList;
			mList = *reinterpret_cast<void**>(obj);
			--mSize;

			return obj;
		}

		// Push in head.
		void Push(void* ptr)
		{
			*(reinterpret_cast<void**>(ptr)) = mList;
			mList = ptr;
			++mSize;
		}
		

		// Push a string of obj in head.
		void PushRange(void* start, void* end, int siz)
		{
			SLL_PushRange(&mList, start, end);
			mSize += siz;
		}

		void PopRange(void** ret_start, void** ret_end, int len)
		{
			SLL_PopRange(&mList, len, ret_start, ret_end);
			
			mSize -= len;
		}

		inline size_t Length() { return mSize; }
		inline size_t MaxLength() { return mMaxSize; }
		inline void SetMaxLength(size_t new_len) { mMaxSize = new_len; }


	private:

		void* mList;//objects linked list.
		size_t mSize;//current size.
		size_t mMaxSize;//max size to fill in object.
	};

	// TC for each thread, memory less than 64k.
	class ThreadCache
	{
	public:
		void* Allocate(size_t siz);
		void Deallocate(void* src, size_t siz);

		// Fetching memory from central cache, fetch serveral obj to improve efficiency.
		void* FetchFromCentralCache(size_t cls_index, size_t siz);
		void ReleaseToCentralCache(FreeList* p_list, size_t cls, int len);

	private:

		FreeList mFreeLists[NUM_FREE_LIST];// FreeLists for every sizeClass.
		std::thread mT;

		size_t mTotalSize;// Total data size.
		size_t mThreadMaxSize;// Max data size in thread.   
	};

}

#endif // !_NCMALLOC_THREAD_CACHE_H_
