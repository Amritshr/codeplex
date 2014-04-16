#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(AdjacentFindTest)
	{
		const static size_t MARKER = 0;
		const static size_t MARKER_COUNT = 2;

		template<typename _IterCat>
		struct AdjacentFindAlgoTest :
			public AlgoTest<AdjacentFindAlgoTest<_IterCat>, _IterCat, size_t>
		{
		public:
			AdjacentFindAlgoTest()
			{
				std::iota(std::begin(_Ct), std::end(_Ct), 1);

				for (int _Count = 0; _Count < MARKER_COUNT; ++_Count) {

					for (;;) {
						size_t _Current_pos = rand() % _Ct.size();

						if (_Current_pos > MARKER_COUNT)
							_Current_pos -= MARKER_COUNT;

						if (_Ct[_Current_pos] != MARKER) {
							_Ct[_Current_pos] = MARKER;
							_Ct[_Current_pos + 1] = MARKER;
							break;
						}
					}
				}
			}

			~AdjacentFindAlgoTest()
			{
				auto _It = std::find(std::begin(_Ct), std::end(_Ct), MARKER);

				Assert::IsTrue(_Result == _It);

				// Next element must be marker
				++_Result;
				Assert::IsTrue(_Result != std::end(_Ct));
				Assert::IsTrue(*_Result == MARKER);
			}

			std::function<bool(size_t, size_t)> callback()
			{
				return std::function<bool(size_t, size_t)>([](size_t _Val, size_t _Val2){
					return _Val == _Val2;
				});
			}
		};

		template<typename _IterCat>
		void RunAdjacentFind()
		{
			{  // seq
				AdjacentFindAlgoTest<_IterCat> _Alg;
				_Alg.set_result(adjacent_find(seq, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par
			AdjacentFindAlgoTest<_IterCat> _Alg;
				_Alg.set_result(adjacent_find(par, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //vec
				AdjacentFindAlgoTest<_IterCat> _Alg;
				_Alg.set_result(adjacent_find(vec, _Alg.begin_in(), _Alg.end_in()));
			}
		}

		TEST_METHOD(AdjacentFind)
		{
			RunAdjacentFind<random_access_iterator_tag>();
			RunAdjacentFind<forward_iterator_tag>();		
		}

		template<typename _IterCat>
		void RunAdjacentFindCallback()
		{
			{  // seq
				AdjacentFindAlgoTest<random_access_iterator_tag> _Alg;
				_Alg.set_result(adjacent_find(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par
				AdjacentFindAlgoTest<random_access_iterator_tag> _Alg;
				_Alg.set_result(adjacent_find(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //vec
				AdjacentFindAlgoTest<random_access_iterator_tag> _Alg;
				_Alg.set_result(adjacent_find(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(AdjacentFindCallback)
		{
			RunAdjacentFindCallback<random_access_iterator_tag>();
			RunAdjacentFindCallback<forward_iterator_tag>();
		}

		TEST_METHOD(AdjacentFindEdgeCases)
		{
			std::vector<int> vec_empty;			
			Assert::IsTrue(adjacent_find(seq, std::begin(vec_empty), std::end(vec_empty)) == std::end(vec_empty));

			std::vector<int> vec_one = { 2 };
			Assert::IsTrue(adjacent_find(seq, std::begin(vec_one), std::end(vec_one)) == std::end(vec_one));

			std::vector<int> vec_two = { 3, 3 };
			Assert::IsTrue(adjacent_find(seq, std::begin(vec_two), std::end(vec_two)) == std::begin(vec_two));

			std::vector<int> vec_two_negative = { 3, 4 };
			Assert::IsTrue(adjacent_find(seq, std::begin(vec_two_negative), std::end(vec_two_negative)) == std::end(vec_two_negative));
		}
	};
} // ParallelSTL_Tests
