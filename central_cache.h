#ifndef _NC_CENTRAL_CACHE_H_
#define _NC_CENTRAL_CACHE_H_

#include"nc_malloc.h"
#include"span.h"
#include<mutex>

namespace NCMalloc
{
	static const int MAX_BUFFER_NUM = 64;

	// FreeList in cental cache.
	class CentralFreeList
	{
	public:

		// Release siz objects 
		int ReleaseRange(void** ret_start, void** ret_end, int siz)
		{
			std::unique_lock<std::mutex> lock(mutex);

			// If need objects size is batchSize. Return objects in slots.
			if (siz == Global::sizeMap()->BatchSizeForClass(mSizeClass) &&
				mSlotsUsed > 0)
			{
				int slot = --mSlotsUsed;

				auto ob = &mSlots[slot];
				*ret_start = ob->head;
				*ret_end = ob->tail;
				lock.unlock();
				return siz;
			}

			// Or fetch from spans.
			int ret_siz = 0;
			*ret_start = nullptr;
			*ret_end = nullptr;
			// Fetch from span, also make up span from page heap.
			ret_siz = FetchFromSpanSafe(ret_start, ret_end, siz);

			if (ret_siz != 0)
			{
				while (ret_siz < siz)// Not enough, keep fetch from next span.
				{
					int n;
					void* head = nullptr;
					void* tail = nullptr;
					n = FetchFromSpan(&head, &tail, siz - ret_siz);
					if (!n) break;// ALL span empty, stop fetching.
					ret_siz += n;
					// Insert.
					*(reinterpret_cast<void**>(tail)) = ret_start;
					*ret_start = head;
				}
			}

			return ret_siz;
		}

		// Get span from page heap.
		void GenerateSpan()
		{
			const size_t npages = Global::sizeMap()->BatchSizeForClass(mSizeClass);

			Span* span;
			{
				// Make span from page heap.

			}
			if (span == nullptr)
				return;

			for (int i = 0; i < npages; i++) 
			{
				Global::
			}

		}

		// Only return 0 when allocation from pageheap error.
		int FetchFromSpanSafe(void** ret_start, void** ret_end, int siz)
		{
			int res = FetchFromSpan(ret_start, ret_end, siz);
			if (res == 0)// If empty, first get spans, then fetch. 
			{
				GenerateSpan();
				res = FetchFromSpan(ret_start, ret_end, siz);
			}

			return res;
		}

		int FetchFromSpan(void** ret_start, void** ret_end, int siz)
		{
			// Current span on list.
			Span* span = mSpanList.next;

			// Cut siz objects from span;
			int res = 0;
			void* cur = span->objects, *pre;
			do {
				pre = cur;
				cur = *(reinterpret_cast<void**>(pre));
			} while (++res < siz && cur != nullptr);

			// Remove empty span.
			if (cur == nullptr)// span empty now.
			{

			}

			// Ret.
			*ret_start = span->objects;
			*ret_end = pre;

			span->objects = cur;
			*(reinterpret_cast<void**>(*ret_end)) = nullptr;

			return res;
		}

		

		/*bool HaveSpareSpace()
		{
			if (mSlotsUsed < mSlotsTotal) return true;
			if (mSlotsUsed == mSlotsMax) return false;

			if ()
			{
				if (mSlotsTotal < mSlotsMax)
				{
					mSlotsTotal++;
					return true;
				}
			}

		}*/

		void InsertRange(void* start, void* end, int siz)
		{
			std::unique_lock<std::mutex> lock(mutex);

			/*if(siz == Global::sizeMap()->BatchSizeForClass(mSizeClass) &&
				HaveSpareSpace())*/
		}

	private:

		// Size class in this list.
		size_t mSizeClass;

		// A buffer with class batchSize objects.
		struct ObjBuffer {
			void* head;
			void* tail;
		};

		// Each slot stores batchSize objects.
		ObjBuffer mSlots[MAX_BUFFER_NUM];
		int mSlotsUsed;// Current used slots num, updated under a lock.
		int mSlotsTotal;// Current total slots num(adaptive), always less than mSlotsMax.
		int mSlotsMax;// A maximum size of slots.

		Span mSpanList;// Dummy header for span list.

		std::mutex mutex;
	};

	// Padding CentralFreeList to multiple of 64 bytes.
	template<int SizeModTo64>
	class CentralFreeListPaddedTo : public CentralFreeList
	{
		char pad[64 - SizeModTo64];
	};
	template<>
	class CentralFreeListPaddedTo<0> : public CentralFreeList {};

	class CentralFreeListPadded :
		public CentralFreeListPaddedTo<sizeof(CentralFreeList) % 64> {};
}
class
{

};

#endif // !_NC_CENTRAL_CACHE_H_
