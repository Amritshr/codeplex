#include "stdafx.h"
#include "utils.h"
#include <time.h>

namespace ParallelSTL_Tests {
	TEST_MODULE_INITIALIZE(ModuleInitialize)
	{
		srand((unsigned int)time(nullptr));
#ifdef _DEBUG
		utils::Algo::Settings(100000);
#else
		utils::Algo::Settings(20000000);
#endif
	}
}
