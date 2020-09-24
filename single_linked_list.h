#ifndef _NC_SINGLE_LINKED_LIST_
#define _NC_SINGLE_LINKED_LIST_

namespace NCMalloc
{
	// Get next for ptr.
	inline void* SLL_Next(void* ptr){
		return *(reinterpret_cast<void**>(ptr));
	}
	// Let ptr point to nex.
	inline void SLL_SetNext(void* ptr, void* nex) {
		*(reinterpret_cast<void**>(ptr)) = nex;
	}
	// Push ptr before head.
	inline void SLL_Push(void** head, void* ptr){
		SLL_SetNext(ptr, *head);
		*head = ptr;
	}
	// Pop from list.
	inline void* SLL_Pop(void** list) {
		void* res = *list;
		*list = SLL_Next(*list);
		return res;
	}
	// Push objects between start and end before head.
	inline void SLL_PushRange(void** list, void* start, void* end) {
		if (!start) return;
		SLL_SetNext(end, *list);
		*list = start;
	}
	// Pop N objects from list into start and end.
	inline void SLL_PopRange(void** list, int siz, void** start, void** end) {
		if (siz == 0) {
			start = nullptr;
			end = nullptr;
			return;
		}

		void* cur = *list;
		for (int i = 1; i < siz; i++) cur = SLL_Next(cur);

		*start = *list;
		*end = cur;

		*list = SLL_Next(cur);
		SLL_SetNext(cur, nullptr);// cut off.
	}

}

#endif // !_NC_SINGLE_LINKED_LIST_
