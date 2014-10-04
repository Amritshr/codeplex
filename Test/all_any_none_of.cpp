#include "stdafx.h"

namespace ParallelSTL_Tests
{
	enum FillType {
		None = 0,
		Any,
		All
	};

	template<typename _IterCat>
	struct AllAnyNoneAlgoTest :
		public AlgoTest<AllAnyNoneAlgoTest<_IterCat>, _IterCat, size_t, bool>
	{
		// Marker values
		static const size_t NEGATIVE_MARKER = 1;
		static const size_t POSITIVE_MARKER = 2;
		static const size_t MARKER_COUNT = 10; // number of positive markers when select any

		bool _ExpectedResult;

		AllAnyNoneAlgoTest(FillType _Type, bool _Expected) : _ExpectedResult(_Expected)
		{
			size_t _Marker_value = NEGATIVE_MARKER;

			if (_Type == All)
				_Marker_value = POSITIVE_MARKER;

			std::fill(std::begin(_Ct), std::end(_Ct), _Marker_value);

			if (_Type == Any) {
				for (size_t _I = 0; _I < MARKER_COUNT; ++_I) {
					size_t _Pos = rand() % _Ct.size();
					_Ct[_Pos] = POSITIVE_MARKER;
				}
			}
		}

		~AllAnyNoneAlgoTest()
		{
			Assert::IsTrue(_Result == _ExpectedResult);
		}

		std::function<bool(size_t)> predicate()
		{
			return std::function<bool(size_t)>([](size_t _Val){
				return _Val == POSITIVE_MARKER;
			});
		}
	};

	TEST_CLASS(AnyTest)
	{
		TEST_METHOD(AnyOf_AllOf_NoneOf_EdgeCases)
		{
			std::vector<int> vec_empty;

			// Empty collection
			Assert::IsFalse(any_of(par, std::begin(vec_empty), std::end(vec_empty), [](int){ return true; }));
			Assert::IsTrue(none_of(par, std::begin(vec_empty), std::end(vec_empty), [](int){ return true; }));
			Assert::IsTrue(all_of(par, std::begin(vec_empty), std::end(vec_empty), [](int){ return true; }));
		}

		template<typename _IterCat>
		void RunAnyOf(FillType _Type, bool _Expected)
		{
			{  // seq
				AllAnyNoneAlgoTest<_IterCat> _Alg(_Type, _Expected);
				_Alg.set_result(any_of(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate()));
			}

			{  //par
				AllAnyNoneAlgoTest<_IterCat> _Alg(_Type, _Expected);
				_Alg.set_result(any_of(par, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate()));
			}

			{  //par_vec
				AllAnyNoneAlgoTest<_IterCat> _Alg(_Type, _Expected);
				_Alg.set_result(any_of(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate()));
			}
		}

		TEST_METHOD(AnyOf)
		{
			RunAnyOf<random_access_iterator_tag>(Any, true);
			RunAnyOf<forward_iterator_tag>(Any, true);
			RunAnyOf<input_iterator_tag>(Any, true);		
		}

		TEST_METHOD(AnyOf_NoneAll)
		{
			RunAnyOf<random_access_iterator_tag>(None, false);
			RunAnyOf<random_access_iterator_tag>(All, true);
		}
	};

	TEST_CLASS(NoneTest)
	{
		template<typename _IterCat>
		void RunNoneOf(FillType _Type, bool _Expected)
		{
			{  // seq
				AllAnyNoneAlgoTest<_IterCat> _Alg(_Type, _Expected);
				_Alg.set_result(none_of(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate()));
			}

			{  //par
				AllAnyNoneAlgoTest<_IterCat> _Alg(_Type, _Expected);
				_Alg.set_result(none_of(par, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate()));
			}

			{  //par_vec
				AllAnyNoneAlgoTest<_IterCat> _Alg(_Type, _Expected);
				_Alg.set_result(none_of(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate()));
			}
		}

		TEST_METHOD(NoneOf)
		{
			RunNoneOf<random_access_iterator_tag>(None, true);
			RunNoneOf<forward_iterator_tag>(None, true);
			RunNoneOf<input_iterator_tag>(None, true);		
		}

		TEST_METHOD(NoneOf_AnyAll)
		{
			RunNoneOf<random_access_iterator_tag>(Any, false);
			RunNoneOf<random_access_iterator_tag>(All, false);
		}
	};

	TEST_CLASS(AllTest)
	{
		template<typename _IterCat>
		void RunAllOf(FillType _Type, bool _Expected)
		{
			{  // seq
				AllAnyNoneAlgoTest<_IterCat> _Alg(_Type, _Expected);
				_Alg.set_result(all_of(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate()));
			}

			{  //par
				AllAnyNoneAlgoTest<_IterCat> _Alg(_Type, _Expected);
				_Alg.set_result(all_of(par, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate()));
			}

			{  //par_vec
				AllAnyNoneAlgoTest<_IterCat> _Alg(_Type, _Expected);
				_Alg.set_result(all_of(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate()));
			}
		}

		TEST_METHOD(AllOf)
		{
			RunAllOf<random_access_iterator_tag>(All, true);
			RunAllOf<forward_iterator_tag>(All, true);
			RunAllOf<input_iterator_tag>(All, true);
		}

		TEST_METHOD(AllOf_NoneAny)
		{
			RunAllOf<random_access_iterator_tag>(Any, false);
			RunAllOf<random_access_iterator_tag>(None, false);		
		}
	};
} // ParallelSTL_Tests

