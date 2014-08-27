#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(CountTest)
	{
		const static size_t COMPARE_VAL = 1;

		template<typename _IterCat>
		struct CountAlgoTest :
			public AlgoTest<CountAlgoTest<_IterCat>, _IterCat, size_t, size_t>
		{
			CountAlgoTest()
			{
				size_t _Idx = 0;
				std::for_each(std::begin(_Ct), std::end(_Ct), [&_Idx](size_t& _Val){
					_Val = _Idx % 2;
					++_Idx;
				});
			}

			~CountAlgoTest()
			{
				Assert::AreEqual(_Ct.size() / 2, _Result);
			}

			std::function<bool(size_t)> callback()
			{
				return std::function<bool(size_t)>([](size_t _Val){
					return _Val % 2 == 1;
				});
			}
		};

		TEST_METHOD(CountEdgeCases)
		{
			vector<int> vec(10);
			std::atomic<long> _Count = 0;

			count_if(par, std::begin(vec), std::end(vec), [&_Count](int){
				++_Count;
				return true;
			});

			Assert::IsTrue(_Count == static_cast<long>(vec.size()));
		}

		template<typename _IterCat>
		void RunCount()
		{
			{  // seq
				CountAlgoTest<_IterCat> _Alg;
				_Alg.set_result(count(seq, _Alg.begin_in(), _Alg.end_in(), COMPARE_VAL));
			}

			{  //par
				CountAlgoTest<_IterCat> _Alg;
				_Alg.set_result(count(par, _Alg.begin_in(), _Alg.end_in(), COMPARE_VAL));
			}

			{  //par_vec
				CountAlgoTest<_IterCat> _Alg;
				_Alg.set_result(count(par_vec, _Alg.begin_in(), _Alg.end_in(), COMPARE_VAL));
			}
		}

		TEST_METHOD(Count)
		{
			RunCount<random_access_iterator_tag>();
			RunCount<forward_iterator_tag>();
			RunCount<input_iterator_tag>();
		}

		template<typename _IterCat>
		void RunCountIf()
		{
			{  // seq
				CountAlgoTest<_IterCat> _Alg;
				_Alg.set_result(count_if(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par
				CountAlgoTest<_IterCat> _Alg;
				_Alg.set_result(count_if(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par_vec
				CountAlgoTest<_IterCat> _Alg;
				_Alg.set_result(count_if(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(CountIf)
		{
			RunCountIf<random_access_iterator_tag>();
			RunCountIf<forward_iterator_tag>();
			RunCountIf<input_iterator_tag>();
		}
	};
} // ParallelSTL_Tests
