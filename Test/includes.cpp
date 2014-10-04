#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(IncludesTest)
	{
		const static size_t VALUE_STEP = 4;

		template<typename _IterCat>
		struct IncludesAlgoTest :
			public AlgoTest<IncludesAlgoTest<_IterCat>, _IterCat, size_t, bool>
		{
			bool _ExpectedResult;
		public:
			IncludesAlgoTest(bool _Includes, bool _Callback = false) : _ExpectedResult(_Includes)
			{
				auto _Start = rand();
				if (_Callback)
					std::iota(_Ct.rbegin(), _Ct.rend(), _Includes ? _Start : _Start + VALUE_STEP);
				else std::iota(std::begin(_Ct), std::end(_Ct), _Includes ? _Start : _Start + VALUE_STEP);

				_Ct_copy.resize(_Ct.size() / VALUE_STEP);

				if (_Callback) {
					for (auto _It = _Ct_copy.rbegin(); _It != _Ct_copy.rend(); ++_It) {
						*_It = _Start;
						_Start += VALUE_STEP;
					}
				}
				else {
					for (auto _It = std::begin(_Ct_copy); _It != std::end(_Ct_copy); ++_It) {
						*_It = _Start;
						_Start += VALUE_STEP;
					}
				}
			}

			~IncludesAlgoTest()
			{
				Assert::AreEqual(_ExpectedResult, _Result);
			}

			std::function<bool(size_t, size_t)> callback()
			{
				return std::function<bool(size_t, size_t)>([&](size_t _Val, size_t _Val2){
					return _Val > _Val2;
				});
			}
		};

		template<typename _IterCat>
		void RunIncludes(bool _Includes)
		{
			{  // seq
				IncludesAlgoTest<_IterCat> _Alg(_Includes);
				_Alg.set_result(includes(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest()));
			}

			{  //par
				IncludesAlgoTest<_IterCat> _Alg(_Includes);
				_Alg.set_result(includes(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest()));
			}

			{  //par_vec
				IncludesAlgoTest<_IterCat> _Alg(_Includes);
				_Alg.set_result(includes(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest()));
			}
		}

		TEST_METHOD(Includes)
		{
			RunIncludes<random_access_iterator_tag>(true);
			RunIncludes<forward_iterator_tag>(true);
			RunIncludes<input_iterator_tag>(true);
			
			RunIncludes<random_access_iterator_tag>(false);
			RunIncludes<forward_iterator_tag>(false);
			RunIncludes<input_iterator_tag>(false);
		}

		template<typename _IterCat>
		void RunIncludesCallback(bool _Includes)
		{
			{  // seq
				IncludesAlgoTest<_IterCat> _Alg(_Includes, true);
				_Alg.set_result(includes(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest(), _Alg.callback()));
			}

			{  //par
				IncludesAlgoTest<_IterCat> _Alg(_Includes, true);
				_Alg.set_result(includes(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest(), _Alg.callback()));
			}

			{  //par_vec
				IncludesAlgoTest<_IterCat> _Alg(_Includes, true);
				_Alg.set_result(includes(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest(), _Alg.callback()));
			}
		}

		TEST_METHOD(IncludesCallback)
		{
			RunIncludesCallback<random_access_iterator_tag>(true);
			RunIncludesCallback<forward_iterator_tag>(true);
			RunIncludesCallback<input_iterator_tag>(true);

			RunIncludesCallback<random_access_iterator_tag>(false);
			RunIncludesCallback<forward_iterator_tag>(false);
			RunIncludesCallback<input_iterator_tag>(false);
		}

		TEST_METHOD(IncludesEdgeCases)
		{
			std::vector<int> vec_empty;
			std::vector<int> vec_one = { 0 };

			Assert::IsTrue(includes(par, std::begin(vec_empty), std::end(vec_empty), std::begin(vec_empty), std::end(vec_empty)));
			Assert::IsFalse(includes(par, std::begin(vec_empty), std::end(vec_empty), std::begin(vec_one), std::end(vec_one)));
			Assert::IsTrue(includes(par, std::begin(vec_one), std::end(vec_one), std::begin(vec_empty), std::end(vec_empty)));
			Assert::IsTrue(includes(par, std::begin(vec_one), std::end(vec_one), std::begin(vec_one), std::end(vec_one)));

			std::vector<int> vec =  { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
			std::vector<int> vec2 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 11 /* different value */, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };
			std::vector<int> vec3 = { 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24 };

			// Matching collection size equal the comparing one
			Assert::IsTrue(includes(par, std::begin(vec), std::end(vec), std::begin(vec), std::end(vec)));
			// Matching collection size equal the comparing one but not matching collection
			Assert::IsFalse(includes(par, std::begin(vec), std::end(vec), std::begin(vec2), std::end(vec2)));
			Assert::IsFalse(includes(par, std::begin(vec2), std::end(vec2), std::begin(vec), std::end(vec)));
			// The collection that we are searching in is smaller than the searching colection
			Assert::IsFalse(includes(par, std::begin(vec3), std::end(vec3), std::begin(vec2), std::end(vec2)));

			// Check for superset
			std::vector<int> vec_sup = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			std::vector<int> vec_needle = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };

			Assert::IsFalse(includes(par, std::begin(vec_sup), std::end(vec_sup), std::begin(vec_needle), std::end(vec_needle)));
		}
	};
}