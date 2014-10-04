#include "stdafx.h"
#include <execution_policy_utils.h>

namespace ParallelSTL_Tests
{
	TEST_CLASS(MinMaxElementTest)
	{
		const static size_t RAND_MUN_COUNT = 2;

		enum MinMaxType {
			Min = 0,
			Max,
			MinMax,
		};

		template<typename _IterCat>
		struct MinMaxAlgoTest :
			public AlgoTest<MinMaxAlgoTest<_IterCat>, _IterCat, size_t>
		{
		public:
			MinMaxAlgoTest(MinMaxType _Type, bool _Callback = false) : _TypeValue(_Type)
			{
				_ASSERTE(_Ct.size() > 2 * RAND_MUN_COUNT);
				
				auto _MinValue = rand();
				auto _MaxValue = _MinValue + _Ct.size() + 1;

				std::iota(std::begin(_Ct), std::end(_Ct), _MinValue + 1);				

				auto _Begin = std::begin(_Ct);
				for (size_t _I = 0; _I < RAND_MUN_COUNT; ++_I, ++_Begin)
					*_Begin = _MinValue;

				for (size_t _I = 0; _I < RAND_MUN_COUNT; ++_I, ++_Begin)
					*_Begin = _MaxValue;

				std::random_shuffle(std::begin(_Ct), std::end(_Ct));

				if (_Callback) {
					if (_Type == Max)
						_Iter.second = std::max_element(std::begin(_Ct), std::end(_Ct), callback());		
					else _Iter = std::minmax_element(std::begin(_Ct), std::end(_Ct), callback());
				}
				else {
					if (_Type == Max)
						_Iter.second = std::max_element(std::begin(_Ct), std::end(_Ct));
					else _Iter = std::minmax_element(std::begin(_Ct), std::end(_Ct));
				}				
			}

			~MinMaxAlgoTest()
			{
				if (_TypeValue == Min)
				{
					Assert::IsTrue(_Result == _Iter.first);
					Assert::IsTrue(*_Result == *_Iter.first);
				}
				else if (_TypeValue == Max) {
					Assert::IsTrue(_Result == _Iter.second);
					Assert::IsTrue(*_Result == *_Iter.second);
				}
				else {
					Assert::IsTrue(_Res.first == _Iter.first);
					Assert::IsTrue(*_Res.first == *_Iter.first);

					Assert::IsTrue(_Res.second == _Iter.second);
					Assert::IsTrue(*_Res.second == *_Iter.second);
				}
			}

			void set_pair_result(const std::pair<in_iterator, in_iterator>& _R)
			{
				_Res = _R;
			}

			std::function<bool(size_t, size_t)> callback()
			{
				return std::function<bool(size_t, size_t)>([](size_t _Val, size_t _Val2) {
					return (_Val & 0xfff) < (_Val2 & 0xfff);
				});
			}
		private:
			std::pair<std::vector<size_t>::iterator, std::vector<size_t>::iterator> _Iter;
			std::pair<in_iterator, in_iterator> _Res;
			MinMaxType _TypeValue;
		};

		template<typename _IterCat>
		void RunMinElement()
		{
			{  // seq
				MinMaxAlgoTest<_IterCat> _Alg(Min);
				_Alg.set_result(min_element(seq, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par
				MinMaxAlgoTest<_IterCat> _Alg(Min);
				_Alg.set_result(min_element(par, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par_vec
				MinMaxAlgoTest<_IterCat> _Alg(Min);
				_Alg.set_result(min_element(par_vec, _Alg.begin_in(), _Alg.end_in()));
			}
		}

		TEST_METHOD(MinElement)
		{
			RunMinElement<random_access_iterator_tag>();
			RunMinElement<forward_iterator_tag>();
		}

		template<typename _IterCat>
		void RunMinElementCallback()
		{
			{  // seq
				MinMaxAlgoTest<_IterCat> _Alg(Min, true);
				_Alg.set_result(min_element(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par
				MinMaxAlgoTest<_IterCat> _Alg(Min, true);
				_Alg.set_result(min_element(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par_vec
				MinMaxAlgoTest<_IterCat> _Alg(Min, true);
				_Alg.set_result(min_element(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(MinElementCallback)
		{
			RunMinElementCallback<random_access_iterator_tag>();
			RunMinElementCallback<forward_iterator_tag>();
		}

		TEST_METHOD(MinElementEdgeCases)
		{
			std::vector<int> vec_empty;
			Assert::IsTrue(min_element(par, std::begin(vec_empty), std::end(vec_empty)) == std::end(vec_empty));

			std::vector<int> vec_one = { 1 };
			Assert::IsTrue(min_element(par, std::begin(vec_one), std::end(vec_one)) == std::begin(vec_one));

			std::vector<int> vec_same = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			Assert::IsTrue(min_element(par, std::begin(vec_same), std::end(vec_same)) == std::begin(vec_same));

			// Check if the correct element is taken if it's on two different ranges
			const int COLLECTION_SIZE = 2048;
			const int MARKER_VALUE = -1;
			debug_static_execution_policy dbg(COLLECTION_SIZE);
			auto _Pos = dbg.get_chunk_size();

			// Test machine has one core only
			if (dbg.get_chunk_count() < 2)
				return;

			std::vector<int> vec(COLLECTION_SIZE);
			vec[_Pos] = MARKER_VALUE;

			// Only one minimal value
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), min_element(dbg, std::begin(vec), std::end(vec)))) == _Pos);

			// Two the same minimal values in the different chunk
			vec[_Pos - 1] = MARKER_VALUE;
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), min_element(dbg, std::begin(vec), std::end(vec)))) == (_Pos - 1));

			//  Second min in different range
			vec[_Pos] = MARKER_VALUE - 1;
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), min_element(dbg, std::begin(vec), std::end(vec)))) == (_Pos));
		}

		template<typename _IterCat>
		void RunMaxElement()
		{
			{  // seq
				MinMaxAlgoTest<random_access_iterator_tag> _Alg(Max);
				_Alg.set_result(max_element(seq, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par
				MinMaxAlgoTest<random_access_iterator_tag> _Alg(Max);
				_Alg.set_result(max_element(par, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par_vec
				MinMaxAlgoTest<random_access_iterator_tag> _Alg(Max);
				_Alg.set_result(max_element(par_vec, _Alg.begin_in(), _Alg.end_in()));
			}
		}

		TEST_METHOD(MaxElement)
		{
			RunMaxElement<random_access_iterator_tag>();
			RunMaxElement<forward_iterator_tag>();
		}

		template<typename _IterCat>
		void RunMaxElementCallback()
		{
			{  // seq
				MinMaxAlgoTest<random_access_iterator_tag> _Alg(Max, true);
				_Alg.set_result(max_element(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par
				MinMaxAlgoTest<random_access_iterator_tag> _Alg(Max, true);
				_Alg.set_result(max_element(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par_vec
				MinMaxAlgoTest<random_access_iterator_tag> _Alg(Max, true);
				_Alg.set_result(max_element(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(MaxElementCallback)
		{	
			RunMaxElementCallback<random_access_iterator_tag>();
			RunMaxElementCallback<forward_iterator_tag>();
		}

		TEST_METHOD(MaxElementEdgeCases)
		{
			std::vector<int> vec_empty;
			Assert::IsTrue(max_element(par, std::begin(vec_empty), std::end(vec_empty)) == std::end(vec_empty));

			std::vector<int> vec_one = { 1 };
			Assert::IsTrue(max_element(par, std::begin(vec_one), std::end(vec_one)) == std::begin(vec_one));

			std::vector<int> vec_same = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			Assert::IsTrue(max_element(par, std::begin(vec_same), std::end(vec_same)) == std::begin(vec_same));

			// Check if the correct element is taken if it's on two different ranges
			const int COLLECTION_SIZE = 2048;
			const int MARKER_VALUE = 1;
			debug_static_execution_policy dbg(COLLECTION_SIZE);
			auto _Pos = dbg.get_chunk_size();

			// Test machine has one core only
			if (dbg.get_chunk_count() < 2)
				return;

			std::vector<int> vec(COLLECTION_SIZE);
			vec[_Pos] = MARKER_VALUE;

			// Only one minimal value
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), max_element(dbg, std::begin(vec), std::end(vec)))) == _Pos);

			// Two the same minimal values in the different chunk
			vec[_Pos - 1] = MARKER_VALUE;
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), max_element(dbg, std::begin(vec), std::end(vec)))) == (_Pos - 1));

			//  Second min in different range
			vec[_Pos] = MARKER_VALUE + 1;
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), max_element(dbg, std::begin(vec), std::end(vec)))) == (_Pos));
		}

		template<typename _IterCat>
		void RunMinMaxElement()
		{
			{  // seq
				MinMaxAlgoTest<_IterCat> _Alg(MinMax);
				_Alg.set_pair_result(minmax_element(seq, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par
				MinMaxAlgoTest<_IterCat> _Alg(MinMax);
				_Alg.set_pair_result(minmax_element(par, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par_vec
				MinMaxAlgoTest<_IterCat> _Alg(MinMax);
				_Alg.set_pair_result(minmax_element(par_vec, _Alg.begin_in(), _Alg.end_in()));
			}
		}

		TEST_METHOD(MinMaxElement)
		{
			RunMinMaxElement<random_access_iterator_tag>();
			RunMinMaxElement<forward_iterator_tag>();		
		}

		template<typename _IterCat>
		void RunMinMaxElementCallback()
		{
			{  // seq
				MinMaxAlgoTest<_IterCat> _Alg(MinMax, true);
				_Alg.set_pair_result(minmax_element(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par
				MinMaxAlgoTest<_IterCat> _Alg(MinMax, true);
				_Alg.set_pair_result(minmax_element(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par_vec
				MinMaxAlgoTest<_IterCat> _Alg(MinMax, true);
				_Alg.set_pair_result(minmax_element(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(MinMaxElementCallback)
		{	
			RunMinMaxElementCallback<random_access_iterator_tag>();
			RunMinMaxElementCallback<forward_iterator_tag>();
		}

		TEST_METHOD(MinMaxElementEdgeCases)
		{
			std::vector<int> vec_empty;
			auto _Pair = minmax_element(par, std::begin(vec_empty), std::end(vec_empty));
			Assert::IsTrue(_Pair.first == std::end(vec_empty));
			Assert::IsTrue(_Pair.second == std::end(vec_empty));

			std::vector<int> vec_one = { 1 };
			_Pair = minmax_element(par, std::begin(vec_one), std::end(vec_one));
			Assert::IsTrue(_Pair.first == std::begin(vec_one));
			Assert::IsTrue(_Pair.second == std::begin(vec_one));

			std::vector<int> vec_same = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
			_Pair = minmax_element(par, std::begin(vec_same), std::end(vec_same));
			Assert::IsTrue(_Pair.first == std::begin(vec_same));

			auto _End = std::begin(vec_same);
			std::advance(_End, vec_same.size() - 1);
			Assert::IsTrue(_Pair.second == _End);

			// Check if the correct element is taken if it's on two different ranges
			const int COLLECTION_SIZE = 2048;
			const int MARKER_MIN_VALUE = -1;
			const int MARKER_MAX_VALUE = 2;

			debug_static_execution_policy dbg(COLLECTION_SIZE);
			auto _Pos = dbg.get_chunk_size();

			// Test machine has one core only
			if (dbg.get_chunk_count() < 2)
				return;

			std::vector<int> vec(COLLECTION_SIZE);

			vec[_Pos - 1] = MARKER_MIN_VALUE;
			vec[_Pos - 2] = MARKER_MAX_VALUE;

			_Pair = minmax_element(dbg, std::begin(vec), std::end(vec));
			// Only one value
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _Pair.first)) == (_Pos - 1));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _Pair.second)) == (_Pos - 2));
			
			vec[_Pos] = MARKER_MIN_VALUE;
			vec[_Pos + 1] = MARKER_MAX_VALUE;

			_Pair = minmax_element(dbg, std::begin(vec), std::end(vec));

			// Two the same minimal values in the different chunk
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _Pair.first)) == (_Pos - 1));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _Pair.second)) == (_Pos + 1));

			vec[_Pos] = MARKER_MIN_VALUE - 1;
			vec[_Pos + 1] = MARKER_MAX_VALUE + 1;
		
			_Pair = minmax_element(dbg, std::begin(vec), std::end(vec));

			//  Second min in different range
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _Pair.first)) == _Pos);
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _Pair.second)) == (_Pos + 1));
		}
	};
} // ParallelSTL_Tests
