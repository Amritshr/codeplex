#include "stdafx.h"
#include <vector>
#include <numeric>
#include <iostream>
#include <ctime>

using namespace std;
namespace ParallelSTL_Tests
{
	template <typename ExecutionPolicy, typename Itr1, typename Itr2>
	void genericSetOperationTest(const ExecutionPolicy &policy, Itr1 begin1, Itr1 end1, Itr2 begin2, Itr2 end2, size_t maxSize)
	{
		typedef typename std::iterator_traits<Itr1>::value_type ValueType;
		vector<ValueType> res1(maxSize), res2(maxSize);

		res1.resize(set_union(policy, begin1, end1, begin2, end2, res1.begin()) - res1.begin());
		res2.resize(std::set_union(begin1, end1, begin2, end2, res2.begin()) - res2.begin());

		Assert::IsTrue(res1 == res2);
		res1.clear();
		res2.clear();
		res1.resize(maxSize);
		res2.resize(maxSize);

		res1.resize(set_difference(policy, begin1, end1, begin2, end2, res1.begin()) - res1.begin());
		res2.resize(std::set_difference(begin1, end1, begin2, end2, res2.begin()) - res2.begin());

		Assert::IsTrue(res1 == res2);
		res1.clear();
		res2.clear();
		res1.resize(maxSize);
		res2.resize(maxSize);

		res1.resize(set_intersection(policy, begin1, end1, begin2, end2, res1.begin()) - res1.begin());
		res2.resize(std::set_intersection(begin1, end1, begin2, end2, res2.begin()) - res2.begin());

		Assert::IsTrue(res1 == res2);
		res1.clear();
		res2.clear();
		res1.resize(maxSize);
		res2.resize(maxSize);

		res1.resize(set_symmetric_difference(policy, begin1, end1, begin2, end2, res1.begin()) - res1.begin());
		res2.resize(std::set_symmetric_difference(begin1, end1, begin2, end2, res2.begin()) - res2.begin());
		Assert::IsTrue(res1 == res2);
	}

	TEST_CLASS(set_operations_tests)
	{
		template<typename _IterCat>
		struct SetOperationAlgoTest :
			public AlgoTest<SetOperationAlgoTest<_IterCat>, _IterCat, int>
		{
			SetOperationAlgoTest() : _Ct2(_Ct.size() + 100)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), rand());
				std::iota(std::begin(_Ct2), std::end(_Ct2), rand());
			}

			in_iterator begin_in2()
			{
				return make_test_iterator<in_iterator::iterator_category>(_Ct2.begin());
			}

			in_iterator end_in2()
			{
				return make_test_iterator<in_iterator::iterator_category>(_Ct2.end());
			}

			size_t maxSize() const
			{
				return _Ct.size() + _Ct2.size();
			}

			container _Ct2;
		};

		template <typename _IterCat>
		void RunBasicSetOperationsTest()
		{
			{
				SetOperationAlgoTest<_IterCat> _Alg;
				genericSetOperationTest(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_in2(), _Alg.end_in2(), _Alg.maxSize());
			}

			{
				SetOperationAlgoTest<_IterCat> _Alg;
				genericSetOperationTest(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_in2(), _Alg.end_in2(), _Alg.maxSize());
			}
			{
				SetOperationAlgoTest<_IterCat> _Alg;
				genericSetOperationTest(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_in2(), _Alg.end_in2(), _Alg.maxSize());
			}
		}

		TEST_METHOD(BasicSetOperationsTest)
		{
			RunBasicSetOperationsTest<std::input_iterator_tag>();
			RunBasicSetOperationsTest<std::forward_iterator_tag>();
			RunBasicSetOperationsTest<std::bidirectional_iterator_tag>();
			RunBasicSetOperationsTest<std::random_access_iterator_tag>();
		}

		TEST_METHOD(randomSetOperationsTest)
		{
			vector<int> a(1000 + rand() % 1000), b(2000 + rand() % 2000);

			for (int i = 0; i < 10; i++)
			{
				generate(a.begin(), a.end(), [] { return std::rand() % 100; });
				generate(b.begin(), b.end(), [] { return std::rand() % 100; });
				sort(a.begin(), a.end());
				sort(b.begin(), b.end());
				genericSetOperationTest(par, a.begin(), a.end(), b.begin(), b.end(), a.size() + b.size());
				genericSetOperationTest(par_vec, a.begin(), a.end(), b.begin(), b.end(), a.size() + b.size());
				genericSetOperationTest(seq, a.begin(), a.end(), b.begin(), b.end(), a.size() + b.size());
			}
		}

		TEST_METHOD(specialContainerSetOperationsTest)
		{
			vector<int> a(1000, 1), b(1, 0);
			list<int> c(2000, 1);
			
			// we only test par execution policy here because we knew all others are either depending on this or using standard std version
			genericSetOperationTest(par, a.begin(), a.end(), b.begin(), b.end(), 1001);
			genericSetOperationTest(par, a.begin(), a.end(), c.begin(), c.end(), 3000);
			genericSetOperationTest(par, c.begin(), c.end(), c.begin(), c.end(), 2001);
			genericSetOperationTest(par, b.begin(), b.end(), b.begin(), b.end(), 2001);

			b.clear(); // when one container is empty
			genericSetOperationTest(par, a.begin(), a.end(), b.begin(), b.end(), 1001);
			genericSetOperationTest(par, b.begin(), b.end(), a.begin(), a.end(), 1001);

			genericSetOperationTest(par, b.begin(), b.end(), c.begin(), c.end(), 2001);

		}
	};

} // namespace ParallelSTL_Tests
