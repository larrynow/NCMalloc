#ifndef _NC_MALLOC_SPAN_H_
#define _NC_MALLOC_SPAN_H_

#include "nc_malloc.h"
#include<set>

namespace NCMalloc
{
	// struct Span for a string of pages.
	// Span may exisits in span list(in centralFreeList) or page heap.
	struct Span {
		PageID start;// start page number.
		PageNum length;// pages num in span.
		Span* next;
		Span* prev;
		
		union {
			void* objects;// free objects.
			char iter_space[sizeof(SpanSet::iterator)];//Iterator point back to SpanSet.
		};

		unsigned int refcount : 16;// How many used objects in this span.
		unsigned int sizeClass : 8;
		unsigned int status : 2;
		// Span 
		enum SpanStatus {
			IN_USE, ON_NORMAL_LIST, ON_RETURNED_LIST, IN_HEAP
		};
	};

	/*PageHeap*/

	// Use set to store large spans.
	struct SpanPtr {
		explicit SpanPtr(Span* s) : span(s), length(s->length) {}
		Span* span;
		PageNum length;
	};

	// Compare span by their length(less).
	struct SpanPtrCompare {
		bool operator()(SpanPtr a, SpanPtr b) const {
			if (a.length < b.length) return true;
			if (a.length > b.length) return false;
			return a.span->start < b.span->start; //Choose spans with samller adress.
		}
	};

	using SpanSet = std::set<SpanPtr, SpanPtrCompare>;

}

#endif // !_NC_MALLOC_SPAN_H_
