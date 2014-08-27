#include "stdafx.h"

namespace ParallelSTL_Tests
{
	template<typename _ExecutionPolicy>
	void SortImpl(const _ExecutionPolicy& _Policy)
	{
		vector<int> numbers(5000 + rand() % 10000);
		std::iota(numbers.begin(), numbers.end(), 0);

		sort(_Policy, numbers.begin(), numbers.end(), std::greater<int>()); // dsc order
		Assert::IsTrue(std::is_sorted(numbers.begin(), numbers.end(), std::greater<int>()));

		std::random_shuffle(numbers.begin(), numbers.end());
		sort(_Policy, numbers.begin(), numbers.end()); // asc order
		Assert::IsTrue(std::is_sorted(numbers.begin(), numbers.end()));
	}

	template<typename _ExecutionPolicy>
	void StableSortImpl(const _ExecutionPolicy& _Policy)
	{
		vector<pair<size_t, size_t>> numbers(5000 + rand() % 10000);
		for (size_t i = 0; i < numbers.size(); i++)
			numbers[i] = make_pair(i % 10, numbers.size() - i);

		std::random_shuffle(numbers.begin(), numbers.end());
		for (size_t i = 0; i < numbers.size(); i++)
			numbers[i].second = i;

		stable_sort(_Policy, numbers.begin(), numbers.end(), [](const pair<size_t, size_t> &left, const pair<size_t, size_t> &right) { return left.first < right.first; }); // asc order
		Assert::IsTrue(std::is_sorted(numbers.begin(), numbers.end()));
	}

	template<typename _ExecutionPolicy>
	void PartialSortImpl(const _ExecutionPolicy& _Policy)
	{
		vector<int> numbers(5000 + rand() % 10000);
		int midPos = 1 + rand() % (numbers.size() - 1);
		std::iota(numbers.begin(), numbers.end(), 0);

		partial_sort(_Policy, numbers.begin(), numbers.begin() + midPos, numbers.end(), std::greater<int>()); // dsc order
		Assert::IsTrue(std::is_sorted(numbers.begin(), numbers.begin() + midPos, std::greater<int>()));
		Assert::IsTrue(numbers[midPos - 1] >= *std::max_element(numbers.begin() + midPos, numbers.end()));

		std::random_shuffle(numbers.begin(), numbers.end());
		partial_sort(_Policy, numbers.begin(), numbers.begin() + midPos, numbers.end()); // asc order
		Assert::IsTrue(std::is_sorted(numbers.begin(), numbers.begin() + midPos));
		Assert::IsTrue(numbers[midPos - 1] <= *std::min_element(numbers.begin() + midPos, numbers.end()));
	}

	TEST_CLASS(sort_tests)
	{
		TEST_METHOD(Sort)
		{
			SortImpl(seq);
			SortImpl(par);
			SortImpl(par_vec);
		}

		TEST_METHOD(StableSort)
		{
			StableSortImpl(seq);
			StableSortImpl(par);
			StableSortImpl(par_vec);
		}

		TEST_METHOD(PartialSort)
		{
			PartialSortImpl(seq);
			PartialSortImpl(par);
			PartialSortImpl(par_vec);
		}
	};

} // namespace ParallelSTL_Tests
