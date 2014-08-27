#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(LexicographicalCompareTest)
	{
		static const size_t MARKER_VALUE_LESS = 2;
		static const size_t MARKER_VALUE_GRATER = 3;

		enum CompareDataType {
			Equal,
			Less,
			Greater
		};

		template<typename _IterCat>
		struct LexicographicalCompareAlgoTest :
			public AlgoTest<LexicographicalCompareAlgoTest<_IterCat>, _IterCat, size_t, bool>
		{
			CompareDataType _ExpectedResultType;

			LexicographicalCompareAlgoTest(CompareDataType _Type) : _ExpectedResultType(_Type)
			{
				auto _Start = MARKER_VALUE_GRATER + rand();

				std::iota(std::begin(_Ct), std::end(_Ct), _Start);
				_Ct_copy.resize(_Ct.size());
				std::iota(std::begin(_Ct_copy), std::end(_Ct_copy), _Start);

				if (_Type != Equal) {
					auto _Pos = rand() % _Ct.size();

					_Ct[_Pos] = _Type == Less ? MARKER_VALUE_GRATER : MARKER_VALUE_LESS;
					_Ct_copy[_Pos] = _Type == Less ? MARKER_VALUE_LESS : MARKER_VALUE_GRATER;
				}
			}

			~LexicographicalCompareAlgoTest()
			{
				if (_ExpectedResultType == Equal || _ExpectedResultType == Less)
					Assert::IsFalse(_Result);
				else Assert::IsTrue(_Result);
			}

			std::function<bool(size_t, size_t)> callback() {
				return std::function<bool(size_t, size_t)>([](size_t _Val, size_t _Val2) {
					if (_Val == MARKER_VALUE_GRATER || _Val2 == MARKER_VALUE_GRATER)
						return _Val != MARKER_VALUE_GRATER;

					return false;
				});
			}
		};

		template<typename _IterCat>
		void RunLexicographicalCompare(CompareDataType _Type)
		{
			{  // seq
				LexicographicalCompareAlgoTest<_IterCat> _Alg(_Type);
				_Alg.set_result(lexicographical_compare(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest()));
			}

			{  //par
				LexicographicalCompareAlgoTest<_IterCat> _Alg(_Type);
				_Alg.set_result(lexicographical_compare(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest()));
			}

			{  //par_vec
				LexicographicalCompareAlgoTest<_IterCat> _Alg(_Type);
				_Alg.set_result(lexicographical_compare(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest()));
			}
		}

		TEST_METHOD(LexicographicalCompare)
		{
			RunLexicographicalCompare<random_access_iterator_tag>(Equal);
			RunLexicographicalCompare<random_access_iterator_tag>(Less);
			RunLexicographicalCompare<random_access_iterator_tag>(Greater);

			RunLexicographicalCompare<forward_iterator_tag>(Equal);
			RunLexicographicalCompare<forward_iterator_tag>(Less);
			RunLexicographicalCompare<forward_iterator_tag>(Greater);

			RunLexicographicalCompare<input_iterator_tag>(Equal);
			RunLexicographicalCompare<input_iterator_tag>(Less);
			RunLexicographicalCompare<input_iterator_tag>(Greater);
		}

		template<typename _IterCat>
		void RunLexicographicalCompareCallback(CompareDataType _Type)
		{
			{  // seq
				LexicographicalCompareAlgoTest<_IterCat> _Alg(_Type);
				_Alg.set_result(lexicographical_compare(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest(), _Alg.callback()));
			}

			{  //par
				LexicographicalCompareAlgoTest<_IterCat> _Alg(_Type);
				_Alg.set_result(lexicographical_compare(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest(), _Alg.callback()));
			}

			{  //par_vec
				LexicographicalCompareAlgoTest<_IterCat> _Alg(_Type);
				_Alg.set_result(lexicographical_compare(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest(), _Alg.callback()));
			}
		}

		TEST_METHOD(LexicographicalCompareCallback)
		{
			RunLexicographicalCompareCallback<random_access_iterator_tag>(Equal);
			RunLexicographicalCompareCallback<random_access_iterator_tag>(Less);
			RunLexicographicalCompareCallback<random_access_iterator_tag>(Greater);

			RunLexicographicalCompareCallback<forward_iterator_tag>(Equal);
			RunLexicographicalCompareCallback<forward_iterator_tag>(Less);
			RunLexicographicalCompareCallback<forward_iterator_tag>(Greater);

			RunLexicographicalCompareCallback<input_iterator_tag>(Equal);
			RunLexicographicalCompareCallback<input_iterator_tag>(Less);
			RunLexicographicalCompareCallback<input_iterator_tag>(Greater);
		}

		TEST_METHOD(LexicographicalCompareEdgeCases)
		{
			std::vector<int> vec_empty;
			Assert::IsFalse(lexicographical_compare(par, std::begin(vec_empty), std::end(vec_empty), std::begin(vec_empty), std::end(vec_empty)));		
			
			std::vector<int> vec_one_0 = { 0 };
			std::vector<int> vec_one_1 = { 1 };

			Assert::IsTrue(lexicographical_compare(par, std::begin(vec_empty), std::end(vec_empty), std::begin(vec_one_0), std::end(vec_one_0)));
			Assert::IsFalse(lexicographical_compare(par, std::begin(vec_one_0), std::end(vec_one_0), std::begin(vec_empty), std::end(vec_empty)));			

			Assert::IsTrue(lexicographical_compare(par, std::begin(vec_one_0), std::end(vec_one_0), std::begin(vec_one_1), std::end(vec_one_1)));
			Assert::IsFalse(lexicographical_compare(par, std::begin(vec_one_1), std::end(vec_one_1), std::begin(vec_one_0), std::end(vec_one_0)));
			Assert::IsFalse(lexicographical_compare(par, std::begin(vec_one_1), std::end(vec_one_1), std::begin(vec_one_1), std::end(vec_one_1)));

			std::vector<int> vec_two_10 = { 1, 0 };
			std::vector<int> vec_two_01 = { 0, 1 };

			Assert::IsTrue(lexicographical_compare(par, std::begin(vec_two_01), std::end(vec_two_01), std::begin(vec_two_10), std::end(vec_two_10)));
			Assert::IsFalse(lexicographical_compare(par, std::begin(vec_two_10), std::end(vec_two_10), std::begin(vec_two_01), std::end(vec_two_01)));
			Assert::IsFalse(lexicographical_compare(par, std::begin(vec_two_10), std::end(vec_two_10), std::begin(vec_two_10), std::end(vec_two_10)));

			std::vector<int> vec_short = { 0, 0, 2, 3, 4 };
			std::vector<int> vec_long = { 0, 0, 2, 3, 5, 6, 10, 10, 12, 12, 12 };

			Assert::IsTrue(lexicographical_compare(par, std::begin(vec_short), std::end(vec_short), std::begin(vec_long), std::end(vec_long)));
			Assert::IsFalse(lexicographical_compare(par, std::begin(vec_long), std::end(vec_long), std::begin(vec_short), std::end(vec_short)));

			std::vector<int> vec_mismatch = { 1, 2  }; 
			std::vector<int> vec_mismatch2 = { 1, 2, 3, 4, 5 }; // Prefix of vec_mismatch2

			Assert::IsTrue(lexicographical_compare(par, std::begin(vec_mismatch), std::end(vec_mismatch), std::begin(vec_mismatch2), std::end(vec_mismatch2)));
			Assert::IsFalse(lexicographical_compare(par, std::begin(vec_mismatch2), std::end(vec_mismatch2), std::begin(vec_mismatch), std::end(vec_mismatch)));
		}

		TEST_METHOD(LexicographicalIteratorCombinations)
		{
			// This test must just instantiate all template no runtime execution is required
			bool _Exit = true; // Removes constant expresssion warning
			if (_Exit) return;

			std::vector<int> vec;

			auto _RaIter = make_test_iterator<random_access_iterator_tag>(std::begin(vec));
			auto _FwdIter = make_test_iterator<forward_iterator_tag>(std::begin(vec));
			auto _InIter = make_test_iterator<input_iterator_tag>(std::begin(vec));

			lexicographical_compare(par, _RaIter, _RaIter, _FwdIter, _FwdIter);
			lexicographical_compare(par, _RaIter, _RaIter, _InIter, _InIter);

			lexicographical_compare(par, _FwdIter, _FwdIter, _RaIter, _RaIter);
			lexicographical_compare(par, _FwdIter, _FwdIter, _InIter, _InIter);

			lexicographical_compare(par, _InIter, _InIter, _FwdIter, _FwdIter);
			lexicographical_compare(par, _InIter, _InIter, _RaIter, _RaIter);
		}
	};
}
