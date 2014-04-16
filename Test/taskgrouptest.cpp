#include "stdafx.h"
#include <vector>
#include <algorithm>

namespace ParallelSTL_Tests
{
	using namespace std::experimental::parallel::details;

	int specialFib(int n)
	{
		if (n <= 2)
			return n;
		TaskGroup tg;
		int res1, res2;
		auto handle = make_task([=, &res1] {return res1 = specialFib(n - 1); });
		tg.run(handle);
		auto handle2 = make_task([=, &res2] {return res2 = specialFib(n - 2); });
		tg.run(handle2);
		int res3 = specialFib(n - 3);
		tg.wait();
		return res1 + res2 + res3;
	}

	int fib(int n)
	{
		if (n <= 1)
			return n;
		TaskGroup tg;
		int res1, res2;
		auto handle = make_task([=, &res1] {return res1 = fib(n - 1); });
		tg.run(handle);
		res2 = fib(n - 2);
		tg.wait();
		return res1 + res2;
	}

	template <typename Itr, typename Comp>
	void quickSort(Itr begin, Itr end, Comp comp)
	{
		size_t len = end - begin;
		if (len < 2)
			return;

		// find a pivot
		auto pivot = begin[len / 2];
		std::swap(*(end - 1), begin[len / 2]);
		auto mid = std::partition(begin, end - 1, [&](typename std::iterator_traits<Itr>::value_type v) { return comp(v, pivot); });
		std::swap(*mid, *(end - 1));

		TaskGroup stg;
		auto chore = make_task([&] {
			quickSort(begin, mid, comp);
		});
		stg.run(chore);
		quickSort(mid + 1, end, comp);
		stg.wait();
	}

	template <typename Itr, typename Func>
	void pfor(Itr begin, Itr end, Func func)
	{
		int len = end - begin;
		if (len <= 1)
		{
			if (len == 1)
				func(begin);
			return;
		}

		TaskGroup stg;
		auto chore = make_task([&] {
			pfor(begin, begin + len / 2, func);
		});
		stg.run(chore);
		pfor(begin + len / 2, end, func);
		stg.wait();
	}

	TEST_CLASS(taskgroup_tests)
	{
		TEST_METHOD(singletaskgroup)
		{
			std::atomic<int> counter = 0;

			auto doWork = [&] {
				++counter;
			};
			
			TaskGroup tg;
			auto a = make_task(doWork);
			tg.run(a);
			auto b = make_task(doWork);
			tg.run(b);
			auto c = make_task(doWork);
			tg.run(c);
			auto d = make_task(doWork);
			tg.run(d);
			auto e = make_task(doWork);
			tg.run(e);
			auto f = make_task(doWork);
			tg.run(f);
			auto g = make_task(doWork);
			tg.run(g);
			auto h = make_task(doWork);
			tg.run(h);
			auto i = make_task(doWork);
			tg.run(i);
			auto j = make_task(doWork);
			tg.run(j);
			auto k = make_task(doWork);
			tg.run(k);
			auto l = make_task(doWork);
			tg.run(l);
			auto m = make_task(doWork);
			tg.run(m);
			auto n = make_task(doWork);
			tg.run(n);
			tg.wait();
			Assert::AreEqual(14, counter.load());
		}

		TEST_METHOD(taskgroup_fib25)
		{
			Assert::AreEqual(75025, fib(25));
			Assert::AreEqual(2145013, specialFib(25));
		}
		
		TEST_METHOD(taskgroup_quicksort)
		{
			srand(100);
			vector<int> list(100000);
			for (size_t i = 0; i < list.size(); i++)
				list[i] = rand();

			quickSort(list.begin(), list.end(), std::less<>());
			Assert::IsTrue(std::is_sorted(list.begin(), list.end()));
		}

		TEST_METHOD(taskgroup_parallelfor)
		{
			std::atomic<int> counter[100] = {0};

			pfor(0, 100000, [&](int a) {
				++counter[a % 100];
			});

			for (int i = 0; i < 100; i++)
			{
				Assert::AreEqual(1000, counter[i].load());
			}
		}
	};
} // namespace ParallelSTL_Tests
