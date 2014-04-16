// Sort_Sample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <experimental/algorithm>
#include <ppl.h>

template<typename F>
void measure_time(F&& f, const char* name)
{
	using namespace std::chrono;

	auto begin = high_resolution_clock::now();
	f();
	auto end = high_resolution_clock::now();

	printf("%s %llu.%03llu seconds\n", name, 
		duration_cast<seconds>(end - begin).count(),
			duration_cast<milliseconds>(end - begin).count() % 1000);
}

int test_sort(int size)
{
	std::vector<int> v;
	v.resize(size);
	std::iota(v.rbegin(), v.rend(), 0);

	printf("\nTesting sort of %d ints:\n", size);

	// Using serial implementation
	measure_time([v]() mutable 
	{
		std::sort(v.begin(), v.end());
	}, "serial:       ");

	// Using Parallel STL implementation
	measure_time([v]() mutable 
	{
		std::experimental::parallel::sort(std::experimental::parallel::par, v.begin(), v.end());
	}, "parallel STL: ");

	// Using PPL implementation
	measure_time([v]() mutable 
	{
		concurrency::parallel_sort(v.begin(), v.end());
	}, "PPL:          ");

	return 0;
}
 
int _tmain(int /* argc */, _TCHAR* /* argv */ [])
{
	test_sort(1000 * 100);
	test_sort(1000 * 1000);
	test_sort(1000 * 1000 * 10);
	test_sort(1000 * 1000 * 50);
	test_sort(1000 * 1000 * 100);
	return 0;
}

