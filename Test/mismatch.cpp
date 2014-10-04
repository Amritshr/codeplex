#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(MismatchTest)
	{
		static const size_t MARKER_VALUE = 2;

		template<typename _IterCat>
		struct MismatchAlgoTest :
			public AlgoTest<MismatchAlgoTest<_IterCat>, _IterCat, size_t>
		{
			bool _ExpectedResult;

			MismatchAlgoTest(bool _Mismatch) : _ExpectedResult(_Mismatch)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), MARKER_VALUE + rand());
				_Ct_copy.resize(_Ct.size());
				std::copy(std::begin(_Ct), std::end(_Ct), std::begin(_Ct_copy));
		
				if (_Mismatch) {
					_Pos = rand() % _Ct.size();
					_Ct[_Pos] = MARKER_VALUE;
				}
				else _Pos = _Ct.size();
			}

			~MismatchAlgoTest()
			{
				if (_ExpectedResult) {
					auto _M1 = std::begin(_Ct);
					auto _M2 = std::begin(_Ct_copy);

					std::advance(_M1, _Pos);
					std::advance(_M2, _Pos);

					Assert::IsTrue(_Res.first == _M1);
					Assert::IsTrue(_Res.second == _M2);
				}
				else {
					Assert::IsTrue(_Res.first == std::end(_Ct));
					Assert::IsTrue(_Res.second == std::end(_Ct_copy));
				}
			}

			std::function<bool(size_t, size_t)> callback() {
				return std::function<bool(size_t, size_t)>([](size_t _Val, size_t _Val2){
					return _Val != MARKER_VALUE && _Val2 != MARKER_VALUE;
				});
			}

			void set_pair_result(const std::pair<in_iterator, in_iterator>& _R)
			{
				_Res = _R;
			}
		private:
			std::pair<in_iterator, in_iterator> _Res;
			size_t _Pos;
		};

		template<typename _IterCat>
		void RunMismatch(bool _Mismatch)
		{
			{  // seq
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}

			{  //par
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}

			{  //par_vec
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}
		}

		template<typename _IterCat>
		void RunMismatch4Params(bool _Mismatch)
		{
			{  // seq
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest()));
			}

			{  //par
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest()));
			}

			{  //par_vec
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest()));
			}
		}

		TEST_METHOD(Mismatch)
		{
			RunMismatch<random_access_iterator_tag>(true);
			RunMismatch<forward_iterator_tag>(true);			
			RunMismatch<input_iterator_tag>(true);

			RunMismatch<random_access_iterator_tag>(false);
			RunMismatch<forward_iterator_tag>(false);
			RunMismatch<input_iterator_tag>(false);

			RunMismatch4Params<random_access_iterator_tag>(true);
			RunMismatch4Params<forward_iterator_tag>(true);
			RunMismatch4Params<input_iterator_tag>(true);
			
			RunMismatch4Params<random_access_iterator_tag>(false);
			RunMismatch4Params<forward_iterator_tag>(false);
			RunMismatch4Params<input_iterator_tag>(false);
		}

		template<typename _IterCat>
		void RunMismatchCallback(bool _Mismatch)
		{
			{  // seq
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}

			{  //par
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}

			{  //par_vec
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}
		}

		template<typename _IterCat>
		void RunMismatchCallback4Params(bool _Mismatch)
		{
			{  // seq
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest(), _Alg.callback()));
			}

			{  //par
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest(), _Alg.callback()));
			}

			{  //par_vec
				MismatchAlgoTest<_IterCat> _Alg(_Mismatch);
				_Alg.set_pair_result(mismatch(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.end_dest(), _Alg.callback()));
			}
		}

		TEST_METHOD(MismatchCallback)
		{
			RunMismatchCallback<random_access_iterator_tag>(true);
			RunMismatchCallback<forward_iterator_tag>(true);
			RunMismatchCallback<input_iterator_tag>(true);

			RunMismatchCallback<random_access_iterator_tag>(false);
			RunMismatchCallback<forward_iterator_tag>(false);
			RunMismatchCallback<input_iterator_tag>(false);

			RunMismatchCallback4Params<random_access_iterator_tag>(true);
			RunMismatchCallback4Params<forward_iterator_tag>(true);
			RunMismatchCallback4Params<input_iterator_tag>(true);

			RunMismatchCallback4Params<random_access_iterator_tag>(false);
			RunMismatchCallback4Params<forward_iterator_tag>(false);
			RunMismatchCallback4Params<input_iterator_tag>(false);
		}

		TEST_METHOD(MismatchEdgeCases)
		{
			std::vector<int> vec_empty, vec2_empty;
			std::vector<int> vec_one = { 0 }, vec_one2 = { 1 };

			// Empty comparation
			auto _Iter = mismatch(par, std::begin(vec_empty), std::end(vec_empty), std::begin(vec2_empty));
			Assert::IsTrue(_Iter.first == std::end(vec_empty));
			Assert::IsTrue(_Iter.second == std::end(vec2_empty));

			// First collection empty, second collection one element
			_Iter = mismatch(par, std::begin(vec_empty), std::end(vec_empty), std::begin(vec_one2));
			Assert::IsTrue(_Iter.first == std::end(vec_empty));
			Assert::IsTrue(_Iter.second == std::begin(vec_one2));

			// Mismatch on the first element
			_Iter = mismatch(par, std::begin(vec_one), std::end(vec_one), std::begin(vec_one2));
			Assert::IsTrue(_Iter.first == std::begin(vec_one));
			Assert::IsTrue(_Iter.second == std::begin(vec_one2));

			// One element no mismatch
			_Iter = mismatch(par, std::begin(vec_one2), std::end(vec_one2), std::begin(vec_one2));
			Assert::IsTrue(_Iter.first == std::end(vec_one2));
			Assert::IsTrue(_Iter.second == std::end(vec_one2));
		
			std::vector<int> vec = { 0, 2, 4, 6, 8, 10, 12, 13, 16, 18 }, vec2 = { 0, 2, 4, 5 /* first mismatch */, 8, 10, 12, 14 /* second mismatch */, 16, 18 };

			// Mismatch in the middle of collection - two mismatched values
			_Iter = mismatch(par, std::begin(vec), std::end(vec), std::begin(vec2));

			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _Iter.first)) == size_t{ 3 } );
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec2), _Iter.second)) == size_t{ 3 });
			Assert::AreEqual(*_Iter.first, int{ 6 });
			Assert::AreEqual(*_Iter.second, int{ 5 });
		}

		TEST_METHOD(MismatchEdgeCases4Params)
		{
			std::vector<int> vec_empty, vec2_empty;
			std::vector<int> vec_one = { 0 }, vec_one2 = { 1 };

			// Empty comparation
			auto _Pair = mismatch(par, std::begin(vec_empty), std::end(vec_empty), std::begin(vec2_empty), std::end(vec2_empty));
			Assert::IsTrue(_Pair.first == std::end(vec_empty));
			Assert::IsTrue(_Pair.second == std::end(vec2_empty));

			// First collection empty, second collection one element
			_Pair = mismatch(par, std::begin(vec_empty), std::end(vec_empty), std::begin(vec_one2), std::end(vec_one2));
			Assert::IsTrue(_Pair.first == std::end(vec_empty));
			Assert::IsTrue(_Pair.second == std::begin(vec_one2));

			// First collection none empty, second collection empty
			_Pair = mismatch(par, std::begin(vec_one2), std::end(vec_one2), std::begin(vec_empty), std::end(vec_empty));
			Assert::IsTrue(_Pair.first == std::begin(vec_one2));
			Assert::IsTrue(_Pair.second == std::end(vec_empty));

			// Mismatch on the first element
			_Pair = mismatch(par, std::begin(vec_one), std::end(vec_one), std::begin(vec_one2), std::end(vec_one2));
			Assert::IsTrue(_Pair.first == std::begin(vec_one));
			Assert::IsTrue(_Pair.second == std::begin(vec_one2));

			// One element no mismatch
			_Pair = mismatch(par, std::begin(vec_one2), std::end(vec_one2), std::begin(vec_one2), std::end(vec_one2));
			Assert::IsTrue(_Pair.first == std::end(vec_one2));
			Assert::IsTrue(_Pair.second == std::end(vec_one2));

			std::vector<int> vec = { 1, 2, 4, 6, 8, 10, 12, 13, 16, 18 }, vec2 = { 1, 2, 4, 5 /* first mismatch */, 8, 10, 12, 14 /* second mismatch */, 16, 18 };

			_Pair = mismatch(par, std::begin(vec_one2), std::end(vec_one2), std::begin(vec), std::end(vec));
			Assert::IsTrue(_Pair.first == std::end(vec_one2));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _Pair.second)) == 1);
			Assert::AreEqual(*_Pair.second, int{ 2 });

			_Pair = mismatch(par, std::begin(vec), std::end(vec), std::begin(vec_one2), std::end(vec_one2));
			Assert::AreEqual(*_Pair.first, int{ 2 });
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _Pair.first)) == 1);
			Assert::IsTrue(_Pair.second == std::end(vec_one2));

			// Mismatch in the middle of collection - two mismatched values
			_Pair = mismatch(par, std::begin(vec), std::end(vec), std::begin(vec2), std::end(vec2));

			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _Pair.first)) == size_t{ 3 });
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec2), _Pair.second)) == size_t{ 3 });
			Assert::AreEqual(*_Pair.first, int{ 6 });
			Assert::AreEqual(*_Pair.second, int{ 5 });
		}

		TEST_METHOD(MismatchIteratorCombinations)
		{
			// This test must just instantiate all template no runtime execution is required
			bool _Exit = true; // Removes constant expresssion warning
			if (_Exit) return;

			std::vector<int> vec;

			auto _RaIter = make_test_iterator<random_access_iterator_tag>(std::begin(vec));
			auto _FwdIter = make_test_iterator<forward_iterator_tag>(std::begin(vec));
			auto _InIter = make_test_iterator<input_iterator_tag>(std::begin(vec));		
			
			// Compile time test with mixing input iterators for mismatch
			mismatch(par, _RaIter, _RaIter, _FwdIter);
			mismatch(par, _RaIter, _RaIter, _InIter);

			mismatch(par, _FwdIter, _FwdIter, _RaIter);
			mismatch(par, _FwdIter, _FwdIter, _InIter);

			mismatch(par, _InIter, _InIter, _FwdIter);
			mismatch(par, _InIter, _InIter, _FwdIter);

			// Compile time test with mixing input iterators for mismatch with 4 parameters
			mismatch(par, _RaIter, _RaIter, _FwdIter, _FwdIter);
			mismatch(par, _RaIter, _RaIter, _InIter, _InIter);

			mismatch(par, _FwdIter, _FwdIter, _RaIter, _RaIter);
			mismatch(par, _FwdIter, _FwdIter, _InIter, _InIter);

			mismatch(par, _InIter, _InIter, _FwdIter, _FwdIter);
			mismatch(par, _InIter, _InIter, _FwdIter, _FwdIter);
		}
	};
}
