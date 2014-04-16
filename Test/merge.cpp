#include "stdafx.h"

namespace ParallelSTL_Tests
{
	template<typename _ExecutionPolicy>
	void MergeImpl(const _ExecutionPolicy& _Policy)
	{
		const size_t size1 = 5000 + rand() % 10000, size2 = 10000 + rand() % 20000;
		vector<pair<int, size_t>> data1(size1), data2(size2), data3(size1 + size2);
		for (size_t i = 0; i < data1.size(); i++)
			data1[i].first = i % 100;
		for (size_t i = 0; i < data2.size(); i++)
			data2[i].first = i % 100;
		sort(_Policy, data1.begin(), data1.end());
		sort(_Policy, data2.begin(), data2.end());
		for (size_t i = 0; i < data1.size(); i++)
			data1[i].second = i;
		for (size_t i = 0; i < data2.size(); i++)
			data2[i].second = i + data1.size();
		merge(_Policy, data1.begin(), data1.end(), data2.begin(), data2.end(), data3.begin(), [](const pair<int, size_t> &left, const pair<int, size_t> &right) { return left.first < right.first; });
		Assert::IsTrue(std::is_sorted(data3.begin(), data3.end()));

		std::reverse(data1.begin(), data1.end());
		std::reverse(data2.begin(), data2.end());

		merge(_Policy, data2.begin(), data2.end(), data1.begin(), data1.end(), data3.begin(), [](pair<int, size_t> left, pair<int, size_t> right) { return left.first > right.first; });
		Assert::IsTrue(std::is_sorted(data3.begin(), data3.end(), std::greater<pair<int, size_t>>()));
	}

	template<typename _ExecutionPolicy>
	void InplaceMergeImpl(const _ExecutionPolicy& _Policy)
	{
		vector<pair<size_t, size_t>> data1(20000 + rand() % 30000);
		int middle = rand() % data1.size();
		for (size_t i = 0; i < data1.size(); i++)
			data1[i] = std::pair<size_t, size_t>(i % 1000, i);

		std::random_shuffle(data1.begin(), data1.end());
		sort(_Policy, data1.begin(), data1.begin() + middle);
		sort(_Policy, data1.begin() + middle, data1.end());
		// test the stabalness
		for (size_t i = 0; i < data1.size(); i++)
			data1[i].second = i;

		inplace_merge(_Policy, data1.begin(), data1.begin() + middle, data1.end(), [](const pair<size_t, size_t> &left, const pair<size_t, size_t> &right) { return left.first < right.first; });
		Assert::IsTrue(std::is_sorted(data1.begin(), data1.end()));
	}

	TEST_CLASS(merge_tests)
	{

		template<typename _IterCat, typename _IterCat2>
		struct MergeAlgoTest :
			public AlgoTest<MergeAlgoTest<_IterCat, _IterCat2>, _IterCat, int, void, _IterCat2>
		{
			MergeAlgoTest() : _Ct2(_Ct.size() + 100)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), rand());
				std::iota(std::begin(_Ct2), std::end(_Ct2), rand());
				_Ct_copy.resize(_Ct.size() + _Ct2.size(), 0);
			}

			~MergeAlgoTest()
			{
				container _Res(_Ct.size() + _Ct2.size());
				std::merge(std::begin(_Ct), std::end(_Ct), std::begin(_Ct2), std::end(_Ct2), std::begin(_Res));
				Assert::IsTrue(_Result == _Ct_copy.end());
				Assert::IsTrue(_Res == _Ct_copy);
			}

			in_iterator begin_in2()
			{
				return make_test_iterator<in_iterator::iterator_category>(_Ct2.begin());
			}

			in_iterator end_in2()
			{
				return make_test_iterator<in_iterator::iterator_category>(_Ct2.end());
			}

			container _Ct2;
		};

		TEST_METHOD(MergeSpecial)
		{
			MergeImpl(seq);
			MergeImpl(par);
			MergeImpl(vec);
		}

		TEST_METHOD(ImplaceMergeSpecial)
		{
			InplaceMergeImpl(seq);
			InplaceMergeImpl(par);
			InplaceMergeImpl(vec);
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunMerge()
		{
			{
				MergeAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(merge(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_in2(), _Alg.end_in2(), _Alg.begin_dest()));
			}
			{
				MergeAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(merge(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_in2(), _Alg.end_in2(), _Alg.begin_dest()));
			}
			{
				MergeAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(merge(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_in2(), _Alg.end_in2(), _Alg.begin_dest()));
			}

		}

		TEST_METHOD(Merge)
		{
			RunMerge<std::input_iterator_tag, std::output_iterator_tag>();
			RunMerge<std::forward_iterator_tag>();
			RunMerge<std::bidirectional_iterator_tag>();
			RunMerge<std::random_access_iterator_tag>();
		}


		template<typename _IterCat>
		struct InplaceMergeAlgoTest :
			public AlgoTest<InplaceMergeAlgoTest<_IterCat>, _IterCat, int>
		{
			InplaceMergeAlgoTest()
			{
				size_t _Mid = _Ct.size() / 2;
				_MidItr = std::begin(_Ct);
				std::advance(_MidItr, _Mid);
				std::iota(make_test_iterator<in_iterator::iterator_category>(std::begin(_Ct)), _MidItr, rand());
				std::iota(_MidItr, make_test_iterator<in_iterator::iterator_category>(std::end(_Ct)), rand());
				_Ct_copy = _Ct;
				auto _MidItr2 = _Ct_copy.begin();
				std::advance(_MidItr2, _Mid);
				std::inplace_merge(_Ct_copy.begin(), _MidItr2, _Ct_copy.end());
			}

			~InplaceMergeAlgoTest()
			{
				Assert::IsTrue(_Ct == _Ct_copy);
			}

			in_iterator _MidIterator()
			{
				return _MidItr;
			}

			in_iterator _MidItr;
		};

		template<typename _IterCat>
		void RunMergeImplace()
		{
			{
				InplaceMergeAlgoTest<_IterCat> _Alg;
				inplace_merge(seq, _Alg.begin_in(), _Alg._MidIterator(), _Alg.end_in());
			}
			{
				InplaceMergeAlgoTest<_IterCat> _Alg;
				inplace_merge(par, _Alg.begin_in(), _Alg._MidIterator(), _Alg.end_in());
			}
			{
				InplaceMergeAlgoTest<_IterCat> _Alg;
				inplace_merge(vec, _Alg.begin_in(), _Alg._MidIterator(), _Alg.end_in());
			}
		}

		TEST_METHOD(ImplaceMerge)
		{
			RunMergeImplace<std::bidirectional_iterator_tag>();
			RunMergeImplace<std::random_access_iterator_tag>();
		}
	};


}
