#ifndef _NCMALLOC_GLOBAL_H_
#define _NCMALLOC_GLOBAL_H_

#include "size_logic.h"
#include "central_cache.h"

namespace NCMalloc
{
	class Global
	{
	public:
		static SizeMap* sizeMap() { return &mSizeMap; }

		static CentralFreeListPadded* centralCache() { return mCentralFreeLists; }

	private:
		static SizeMap mSizeMap;
		static CentralFreeListPadded mCentralFreeLists[128];

	};
}

#endif // !_NCMALLOC_GLOBAL_H_
