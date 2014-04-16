#include "stdafx.h"

namespace ParallelSTL_Tests
{
	struct SwapableType {
		SwapableType() : _Val(rand()), _Swap(false)
		{
		}

		int _Val;
		bool _Swap;
	};

	void swap(SwapableType& _A, SwapableType& _B) {
		std::swap(_A._Val, _B._Val);
		_A._Swap = true;
		_B._Swap = true;
	}

	TEST_CLASS(SwapRangesTest)
	{
		template<typename _IterCat>
		struct SwapRangesAlgoTest :
			public AlgoTest<SwapRangesAlgoTest<_IterCat>, _IterCat, size_t>
		{
			size_t _Ct_start_pos;
			size_t _Ct_copy_start_pos;
		public:
			SwapRangesAlgoTest()
			{
				_Ct_start_pos = rand();
				_Ct_copy_start_pos = rand();

				std::iota(std::begin(_Ct), std::end(_Ct), _Ct_start_pos);
				_Ct_copy.resize(_Ct.size());
				std::iota(std::begin(_Ct_copy), std::end(_Ct_copy), _Ct_copy_start_pos);
			}

			~SwapRangesAlgoTest()
			{
				size_t _Idx = _Ct_copy_start_pos;
				for (auto _It = std::begin(_Ct); _It != std::end(_Ct); ++_It, ++_Idx) {
					if (*_It != _Idx) {
						Assert::Fail();
						break;
					}
				}

				_Idx = _Ct_start_pos;
				for (auto _It = std::begin(_Ct_copy); _It != std::end(_Ct_copy); ++_It, ++_Idx) {
					if (*_It != _Idx) {
						Assert::Fail();
						break;
					}
				}
			}
		};

		TEST_METHOD(SwapRangesEdgeCases)
		{
			std::vector<int> vec, vec2;
			auto _Iter = swap_ranges(par, std::begin(vec), std::end(vec), std::begin(vec2));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec2), _Iter)) == vec.size());
			Assert::AreEqual(vec.size(), size_t{ 0 });

			// Validation for one element
			std::vector<int> vec_one(1, 2), vec_one2(1, 3);
			_Iter = swap_ranges(par, std::begin(vec_one), std::end(vec_one), std::begin(vec_one2));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec_one2), _Iter)) == vec_one.size());
			Assert::AreEqual(vec_one[0], 3);
			Assert::AreEqual(vec_one2[0], 2);

			// Validation for two elements
			std::vector<int> vec_two = { 4, 5 };
			std::vector<int> vec_two2 = { 6, 7 };
			_Iter = swap_ranges(par, std::begin(vec_two), std::end(vec_two), std::begin(vec_two2));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec_two2), _Iter)) == vec_two.size());

			Assert::AreEqual(vec_two[0], 6);
			Assert::AreEqual(vec_two2[0], 4);
			Assert::AreEqual(vec_two[1], 7);
			Assert::AreEqual(vec_two2[1], 5);

			std::vector<SwapableType> vec_swapable(10);
			std::vector<SwapableType> vec_swapable2(10);

			swap_ranges(par, std::begin(vec_swapable), std::end(vec_swapable), std::begin(vec_swapable2));

			auto _Cpr = std::begin(vec_swapable2);
			for (auto _It = std::begin(vec_swapable); _It != std::end(vec_swapable); ++_It, ++_Cpr) {
				Assert::AreEqual((*_It)._Swap, true);
				Assert::AreEqual((*_Cpr)._Swap, true);
			}
		}

		template<typename _IterCat>
		void RunSwapRanges()
		{
			{  // seq
				SwapRangesAlgoTest<_IterCat> _Alg;
				swap_ranges(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest());
			}

			{  //par
				SwapRangesAlgoTest<_IterCat> _Alg;
				swap_ranges(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest());
			}

			{  //vec
				SwapRangesAlgoTest<_IterCat> _Alg;
				swap_ranges(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest());
			}
		}

		TEST_METHOD(SwapRanges)
		{
			RunSwapRanges<random_access_iterator_tag>();
			RunSwapRanges<forward_iterator_tag>();
		}
	};
} // ParallelSTL_Tests
