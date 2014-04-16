#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(ReverseTest)
	{
		template<typename _IterCat>
		struct ReverseAlgoTest :
			public AlgoTest<ReverseAlgoTest<_IterCat>, _IterCat, size_t>
		{
			ReverseAlgoTest()
			{
				std::iota(std::begin(_Ct), std::end(_Ct), 0);
			}

			~ReverseAlgoTest()
			{
				size_t _Last = _Ct.size() - 1;

				for (auto _It = std::begin(_Ct); _It != std::end(_Ct); ++_It, --_Last) {
					if (*_It != _Last) {
						Assert::Fail();
						break;
					}
				}
			}
		};

		template<typename _IterCat>
		void RunReverse()
		{
			{  // seq
				ReverseAlgoTest<_IterCat> _Alg;
				reverse(seq, _Alg.begin_in(), _Alg.end_in());
			}

			{  //par
				ReverseAlgoTest<_IterCat> _Alg;
				reverse(par, _Alg.begin_in(), _Alg.end_in());
			}

			{  //vec
				ReverseAlgoTest<_IterCat> _Alg;
				reverse(vec, _Alg.begin_in(), _Alg.end_in());
			}
		}

		TEST_METHOD(Reverse)
		{
			RunReverse<random_access_iterator_tag>();
			RunReverse<bidirectional_iterator_tag>();		
		}
	}; // ReverseTest

	TEST_CLASS(ReverseCopyTest)
	{
		template<typename _IterCat>
		struct ReverseCopyAlgoTest :
			public AlgoTest<ReverseCopyAlgoTest<_IterCat>, _IterCat, size_t>
		{
			ReverseCopyAlgoTest()
			{
				std::iota(std::begin(_Ct), std::end(_Ct), 0);
				_Ct_copy.resize(_Ct.size());
			}

			~ReverseCopyAlgoTest()
			{
				auto _Rev = std::reverse_iterator<std::vector<size_t>::iterator >(std::end(_Ct));

				for (auto _It = std::begin(_Ct_copy); _It != std::end(_Ct_copy); ++_It, ++_Rev) {
					if (*_It != *_Rev) {
						Assert::Fail();
						break;
					}
				}
			}
		};

		TEST_METHOD(ReverseCopyEdgeCases)
		{
			std::vector<int> vec, dest;

			auto _Iter = reverse_copy(par, std::begin(vec), std::end(vec), std::begin(dest));
			Assert::IsTrue(_Iter == std::end(dest));
			Assert::AreEqual(dest.size(), size_t{ 0 });

			std::vector<int> vec_one(1, 1), dest_one(1, 2);
			_Iter = reverse_copy(par, std::begin(vec_one), std::end(vec_one), std::begin(dest_one));
			Assert::IsTrue(_Iter == std::end(dest_one));
			Assert::AreEqual(dest_one[0], 1);
		}

		template<typename _IterCat>
		void RunReverseCopy()
		{
			{  // seq
				ReverseCopyAlgoTest<_IterCat> _Alg;
				_Alg.set_result(reverse_copy(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}

			{  //par
				ReverseCopyAlgoTest<_IterCat> _Alg;
				_Alg.set_result(reverse_copy(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}

			{  //vec
				ReverseCopyAlgoTest<_IterCat> _Alg;
				_Alg.set_result(reverse_copy(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}
		}

		TEST_METHOD(ReverseCopy)
		{
			RunReverseCopy<random_access_iterator_tag>();
			RunReverseCopy<bidirectional_iterator_tag>();
		}
	};
} // ParallelSTL_Tests
