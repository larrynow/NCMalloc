#include "thread_cache.h"
#include "global.h"

void* NCMalloc::ThreadCache::Allocate(size_t siz)
{
	size_t index = SizeMap::SizeIndex(siz);
	size_t cls = Global::sizeMap()->ClassIndexForSize(index);

	if (mFreeLists[cls].Length()!=0)
	{
		return mFreeLists[cls].Pop();
	}
	else
	{
		//Fetch from central cache.
		return FetchFromCentralCache(cls, siz);
	}
}

void NCMalloc::ThreadCache::Deallocate(void* src, size_t siz)
{
	size_t index = SizeMap::SizeIndex(siz);

	mFreeLists[index].Push(src);

	// If freelist is too long.
	//Allocate FIRST!!! IDIOT!!!
}

void* NCMalloc::ThreadCache::FetchFromCentralCache(size_t cls_index, size_t siz)
{
	auto list = &mFreeLists[cls_index];
	const int batch_size = Global::sizeMap()->BatchSizeForClass(cls_index);

	// Object number to fetch from central cache(slow start up to batch_size).
	const int obj_num_to_move = MIN(batch_size, list->MaxLength());

	// Realase objects from central cache.
	void* start, *end;
	int fetch_num = Global::centralCache()[cls_index].ReleaseRange(
		&start, &end, obj_num_to_move);

	// Latter return start, push rest.
	if (--fetch_num >= 0)// If have rest(more than one obj).
	{
		list->PushRange(SLL_Next(start), end, fetch_num);
	}

	/* Slow start. */

	// Increase list max length in every fetching.
	// Increase 1 when max length under batch size,
	// then increase batch size each time.
	if (list->MaxLength() < batch_size)
	{
		list->SetMaxLength(list->MaxLength() + 1);
	}
	else
	{
		int new_len = MIN(list->MaxLength() + batch_size,
			MAX_DYNAMIC_FREELIST_LENGTH);
		//If new length is MAX_DYNAMIC_FREELIST_LENGTH, 
		//make new length as multiple of batch_size.
		new_len -= new_len % batch_size;

		list->SetMaxLength(new_len);
	}

	return start;
}

void NCMalloc::ThreadCache::ReleaseToCentralCache(FreeList* p_list, size_t cls, int len)
{
	if (len > p_list->Length()) len = p_list->Length();//Truncation.

	int batch_size = Global::sizeMap()->BatchSizeForClass(cls);
	//while (len > batch_size)//Insert to cental cache with batch_size every time.
	//{
	//	void* head, * tail;
	//	p_list->PopRange(&head, &tail, batch_size);
	//	Global::centralCache()
	//}
	//Allocate FIRST!!! IDIOT!!!
}
