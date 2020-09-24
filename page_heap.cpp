#include "page_heap.h"

NCMalloc::Span* NCMalloc::PageHeap::New(PageNum n)
{
	Span* res = Search(n);
	if (res) return res;

	/*
	��û�п��õĿ���spanʱ����Ҫ��ϵͳ�����µ��ڴ棬
	���ڴ�֮ǰ������һ�α�����ϵͳ�������ڴ�Ļ��᣺�ͷ����п����ڴ档
	��ϵͳ������ڴ�ÿ�ﵽ128MB���ҿ����ڴ泬����ϵͳ��������ڴ��1/4��
	����Ҫ�����еĿ����ڴ��ͷš�
	��ΪTCMalloc��normal��returned���ڴ�ֿ��������������ڴ治��ϲ���һ��
	��ˣ�������һ�������Ŀ����ڴ����Ҫ��k��page��С��������Ϊ������normal�Ĳ��֣�
	����returned�Ĳ��֣����ǰ������������Ѳ����������ͷ������ڴ���Խ�normal���ڴ�
	Ҳ��Ϊreturned�ģ�Ȼ��Ϳ��Ժϲ��ˡ�
	*/
	// Release all free memorys(both normal and returned).
	ReleaserNPages(INT_MAX);

	return nullptr;
}

NCMalloc::Span* NCMalloc::PageHeap::Search(PageNum n)
{
	// Find first span with pages >= n.
	for (PageNum i = n; i < MAX_PAGES; i++)
	{
		// First search in Normal first.
		Span* list = &mFreeSpans[i - 1].normal;
		if (!DLL_IsEmpty(list))// If non-empty, cut n Span.
		{
			return Carve(list->next, n);
		}

		// Then for Returned span.
		list = &mFreeSpans[i - 1].returned;
		if (!DLL_IsEmpty(list))
		{
			// TODO : heap limits : coalescing returned spans.
			return Carve(list->next, n);
		}

	}

	// Nothing fit in freelist, find in larger SpanSet finally.
	return SearchInSpanSet(n);
}

NCMalloc::Span* NCMalloc::PageHeap::SearchInSpanSet(PageNum n)
{
	Span* best_fits = nullptr;

	Span bound;
	bound.start = 0;
	bound.length = n;
	SpanPtr sp(&bound);

	// Find first span fits(in normal), upper_bound:length > n;
	Span* best_in_normal = nullptr;
	SpanSet::iterator place = mSpanSet_Normal.upper_bound(sp);
	if (place != mSpanSet_Normal.end())// Find in set.
	{
		best_in_normal = place->span;
		best_fits = best_in_normal;
	}

	// Also search in returned.
	place = mSpanSet_Returned.upper_bound(sp);
	if (place != mSpanSet_Returned.end())
	{
		Span* best_in_ret = place->span;
		if (best_fits == nullptr// Find nothing in normal;
			|| best_in_ret->length < best_fits->length// Smaller length;
			|| (best_in_ret->length == best_fits->length &&
				best_in_ret->start < best_fits->start))// Starts first.
			best_fits = best_in_ret;
	}

	if (best_fits == best_in_normal)// Best fit span is in normal set.
	{
		return best_fits == nullptr ? 
			nullptr : Carve(best_fits, n);// Carve if in not empty.
	}
	else
	{
		// Best fits is in returned.
		// TODO : Heap Limit to release unused spans.
		return Carve(best_fits, n);
	}
}

NCMalloc::PageNum NCMalloc::PageHeap::ReleaserNPages(PageNum n)
{
	PageNum released_pages = 0;

	int TEMP_bytes2release = 100;
	int TEMP_release_span = 0;

	while (TEMP_bytes2release > 0)
	{
		// Loop MAX_PAGES + 1 : Release every spanList once and spanSet.
		// Unless released n pages already.
		for (int i = 0; i < MAX_PAGES + 1 && released_pages < n; i++)
		{
			Span* span2release;
			TEMP_release_span %= (MAX_PAGES + 1);// Mod so that loop MAX_PAGES+1 times.

			if (TEMP_release_span == MAX_PAGES)// This time for SpanSet.
			{
				if (mSpanSet_Normal.empty()) 
					continue;// Do nothing.
				span2release = mSpanSet_Normal.begin()->span;
			}
			else// For small span in list.
			{
				SpanList* list = &mFreeSpans[TEMP_release_span];
				if (DLL_IsEmpty(&list->normal))
					continue;// Do nothing.
				span2release = list->normal.prev;// Get last in spanList(DLL).
			}

			PageNum pages = ReleaseSpan(s);
			if (pages == 0) return released_pages;// Do not support release.
			released_pages += pages;
		}
	}
	return PageNum();
}

NCMalloc::PageNum NCMalloc::PageHeap::ReleaseSpan(Span* span)
{

}

NCMalloc::Span* NCMalloc::PageHeap::Carve(Span* span, PageNum n)
{
	// Take span down from Sapn-list or SpanSet first.
	RemoveSpan(span);
	const int prev_status = span->status;
	span->status = Span::IN_USE;// Status

	const int len2carve = span->length - n;
	if (len2carve > 0)
	{
		// Make the rest part of old span be a new span.
		Span* newSpan = NewSpan(span->start + n, len2carve);
		newSpan->status = prev_status;// Same status as prev.
		RecordSpan(newSpan);// Record new span in spanMap.
		
		AddSpan(newSpan);
	
		span->length = n;
		//TODO : pagemap.
	}

	return span;
}

void NCMalloc::PageHeap::RemoveSpan(Span* span)
{
	if (span->length > MAX_PAGES)// Large span.
	{
		SpanSet* set = &mSpanSet_Normal;
		// TODO : SpanSet iter.
		//auto iter = span->ExtractIter();
		set->erase(set->begin());// Erase span from SpanSet.
	}
	else// Remove span in double linked list.
	{
		DLL_Remove(span);
	}
}

void NCMalloc::PageHeap::AddSpan(Span* span)
{
	if (span->length > MAX_PAGES)// Insert into large set.
	{
		SpanSet* set = (span->status == Span::ON_NORMAL_LIST ?
			&mSpanSet_Normal : &mSpanSet_Returned);
		auto insert_ret = set->insert(SpanPtr(span));
		assert(insert_ret.second);// Insert FAILURE.
		//span->SetInterator.
	}
	else// Prepend span in list.
	{
		Span* list_header = (span->status == Span::ON_NORMAL_LIST ?
			&mFreeSpans[span->length - 1].normal :
			&mFreeSpans[span->length - 1].returned);
		DLL_Prepend(list_header, span);
	}
}
