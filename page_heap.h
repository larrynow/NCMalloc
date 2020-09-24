#ifndef _NC_MALLOC_PAGE_HEAP_
#define _NC_MALLOC_PAGE_HEAP_

#include"nc_malloc.h"
#include"span.h"

namespace NCMalloc
{
	class PageHeap
	{
	public:

		// Allocate n pages.
		Span* New(PageNum n);

	private:

		// Search a span with page length n first in SpanList then SpanSet.
		Span* Search(PageNum n);

		// Search a span with length n in SpanSet.
		Span* SearchInSpanSet(PageNum n);

		// Release at most N pages(if there is so much used memory) back to system.
		PageNum ReleaserNPages(PageNum n);

		// Release a span back to system.
		PageNum ReleaseSpan(Span* span);

		// There is two span lists in page heap, one for normal, one for returned.
		struct SpanList
		{
			Span normal;
			Span returned;// Those pages has been returned to system.
		};

		// Free span double circular linked list(with header) in page heap.
		SpanList mFreeSpans[MAX_PAGES];// From page length 1-MAX_PAGES.

		// For spans with length > MAX_PAGES, use sets instead.
		SpanSet mSpanSet_Normal;
		SpanSet mSpanSet_Returned;

		// Carve extra pages(span->length-n) from a span.
		Span* Carve(Span* span, PageNum n);

		// Remove a span (from list or set depends on where this span belongs).
		void RemoveSpan(Span* span);

		// Add a span to span list.
		void AddSpan(Span* span);
	};

	/*Double circular linked list operations.*/

	// Is this span list(input list header) empty or not.
	inline bool DLL_IsEmpty(const Span* list_header) {
		return list_header->next == list_header;// Empty when next is self(header). 
	}

	// Remove a span from span list.
	inline void DLL_Remove(Span* span)
	{
		span->prev->next = span->next;
		span->next->prev = span->prev;
		span->next = nullptr;
		span->prev = nullptr;
	}

	// Add a span after list header.
	inline void DLL_Prepend(Span* header, Span* span)
	{
		span->next = header->next;
		span->prev = header;
		header->next->prev = span;
		header->next = span;
	}
}

#endif // !_NC_MALLOC_PAGE_HEAP_
