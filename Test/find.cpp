#include "stdafx.h"

#include <execution_policy_utils.h>

namespace ParallelSTL_Tests
{
	TEST_CLASS(FindTest)
	{
		const static size_t MARKER_VALUE = 1;
		const static size_t RAND_NUM_COUNT = 2;

		template<typename _IterCat>
		struct FindEndAlgoTest :
			public AlgoTest<FindEndAlgoTest<_IterCat>, _IterCat, size_t>
		{
			bool _ValueExists;
			size_t _ValuePos;
			typename container::difference_type _ValueCount;
		public:
			std::vector<size_t> _Needle;

			FindEndAlgoTest(bool _Exists) : _ValueExists(_Exists)
			{
				const size_t _Size = _Ct.size() - 2;

				_ValuePos = rand() % _Size;
				_ValueCount = rand() % (_Size - _ValuePos);
				if (_ValueCount < 2)
					_ValueCount = 1;

				_Needle.resize(_ValueCount);
				std::iota(std::begin(_Needle), std::end(_Needle), MARKER_VALUE);
				std::iota(std::begin(_Ct), std::end(_Ct), rand() + _Needle.size());

				if (_ValueExists) {
					std::copy(std::begin(_Needle), std::end(_Needle), std::begin(_Ct) + _ValuePos);
				}
			}

			~FindEndAlgoTest()
			{
				if (_ValueExists) {
					Assert::IsTrue(_Result != end_in());

					container::size_type _Pos = std::distance(begin_in(), _Result);
					Assert::IsTrue(_Pos == _ValuePos);
				}
				else Assert::IsTrue(_Result == end_in());
			}

			std::function<bool(size_t, size_t)> callback()
			{
				return std::function<bool(size_t, size_t)>([&](size_t _First, size_t _Second){
					return _First == _Second;
				});
			}
		};

		template<typename _IterCat>
		struct FindAlgoTest :
			public AlgoTest<FindAlgoTest<_IterCat>, _IterCat, size_t>
		{
			in_iterator _Iter_pos;
			std::vector<size_t> _Find_items;
			bool _ValueExists;
		public:
			typedef typename test_iterator<
				typename std::conditional<std::is_same<_IterCat, std::input_iterator_tag>::value, std::forward_iterator_tag, _IterCat>::type,
				typename std::vector<size_t>::iterator
			> find_iterator;

			FindAlgoTest(bool _Exists) : _ValueExists(_Exists)
			{
				auto _Init = rand() + MARKER_VALUE;
				std::iota(std::begin(_Ct), std::end(_Ct), _Init);
				
				if (_ValueExists) {
					for (size_t _I = 0; _I < RAND_NUM_COUNT; ++_I)
						_Ct[_I] = MARKER_VALUE;

					std::random_shuffle(std::begin(_Ct), std::end(_Ct));

					_Find_items.resize((_Ct.size() % 100) + 2);  // Make sure that there are at least two elements in the collection
					_Find_items[0] = MARKER_VALUE;
					std::random_shuffle(std::begin(_Find_items), std::end(_Find_items));

					_Iter_pos = std::find(std::begin(_Ct), std::end(_Ct), MARKER_VALUE);
				}
			}

			~FindAlgoTest()
			{
				if (_ValueExists) {
					Assert::IsTrue(_Result == _Iter_pos);
					Assert::IsTrue(*_Result == *_Iter_pos);
				}
				else Assert::IsTrue(_Result == end_in());
			}

			std::function<bool(size_t)> callback()
			{
				return std::function<bool(size_t)>([](size_t _Val){
					return _Val == MARKER_VALUE;
				});
			}

			std::function<bool(size_t)> not_callback()
			{
				return std::function<bool(size_t)>([](size_t _Val){
					return _Val != MARKER_VALUE;
				});
			}

			std::function<bool(size_t, size_t)> first_of_callback()
			{
				return std::function<bool(size_t, size_t)>([](size_t _Val, size_t _Val2){
					return _Val == _Val2;
				});
			}

			find_iterator begin_find()
			{
				return make_test_iterator<find_iterator::iterator_category>(std::begin(_Find_items));
			}

			find_iterator end_find()
			{
				return make_test_iterator<find_iterator::iterator_category>(std::end(_Find_items));
			}
		};

		TEST_METHOD(FindEdgeCases)
		{
			std::vector<size_t> empty_vec;
			std::vector<size_t> one_vec = { MARKER_VALUE };

			Assert::IsTrue(find(par, std::begin(empty_vec), std::end(empty_vec), MARKER_VALUE) == std::end(empty_vec));
			auto _Iter = find(par, std::begin(one_vec), std::end(one_vec), MARKER_VALUE);
			Assert::IsTrue(std::distance(std::begin(one_vec), _Iter) == 0);

			Assert::IsTrue(find_if(par, std::begin(empty_vec), std::end(empty_vec), [](size_t _Val){ return _Val == MARKER_VALUE;  }) == std::end(empty_vec));
			_Iter = find_if(par, std::begin(one_vec), std::end(one_vec), [](size_t _Val){ return _Val == MARKER_VALUE;  });
			Assert::IsTrue(std::distance(std::begin(one_vec), _Iter) == 0);

			Assert::IsTrue(find_if_not(par, std::begin(empty_vec), std::end(empty_vec), [](size_t _Val){ return _Val != MARKER_VALUE; }) == std::end(empty_vec));
			_Iter = find_if_not(par, std::begin(one_vec), std::end(one_vec), [](size_t _Val){ return _Val != MARKER_VALUE; });
			Assert::IsTrue(std::distance(std::begin(one_vec), _Iter) == 0);

			Assert::IsTrue(find_first_of(par, std::begin(empty_vec), std::end(empty_vec), std::begin(one_vec), std::end(one_vec)) == std::end(empty_vec));
			Assert::IsTrue(find_first_of(par, std::begin(one_vec), std::end(one_vec), std::begin(empty_vec), std::end(empty_vec)) == std::end(one_vec));
			_Iter = find_first_of(par, std::begin(one_vec), std::end(one_vec), std::begin(one_vec), std::end(one_vec));
			Assert::IsTrue(std::distance(std::begin(one_vec), _Iter) == 0);
		}

		template<typename _IterCat>
		void RunFind(bool _Exist)
		{
			{  // seq
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find(seq, _Alg.begin_in(), _Alg.end_in(), MARKER_VALUE));
			}

			{  //par
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find(par, _Alg.begin_in(), _Alg.end_in(), MARKER_VALUE));
			}

			{  //par_vec
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find(par_vec, _Alg.begin_in(), _Alg.end_in(), MARKER_VALUE));
			}
		}

		TEST_METHOD(Find)
		{
			RunFind<random_access_iterator_tag>(true);
			RunFind<forward_iterator_tag>(true);
			RunFind<input_iterator_tag>(true);

			RunFind<random_access_iterator_tag>(false);
			RunFind<forward_iterator_tag>(false);
			RunFind<input_iterator_tag>(false);
		}

		template<typename _IterCat>
		void RunFindIf(bool _Exist)
		{
			{  // seq
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_if(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_if(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par_vec
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_if(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(FindIf)
		{
			RunFindIf<random_access_iterator_tag>(true);
			RunFindIf<forward_iterator_tag>(true);
			RunFindIf<input_iterator_tag>(true);

			RunFindIf<random_access_iterator_tag>(false);
			RunFindIf<forward_iterator_tag>(false);
			RunFindIf<input_iterator_tag>(false);
		}

		template<typename _IterCat>
		void RunFindIfNot(bool _Exist)
		{
			{  // seq
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_if_not(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.not_callback()));
			}

			{  //par
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_if_not(par, _Alg.begin_in(), _Alg.end_in(), _Alg.not_callback()));
			}

			{  //par_vec
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_if_not(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.not_callback()));
			}
		}

		TEST_METHOD(FindIfNot)
		{
			RunFindIfNot<random_access_iterator_tag>(true);
			RunFindIfNot<forward_iterator_tag>(true);
			RunFindIfNot<input_iterator_tag>(true);

			RunFindIfNot<random_access_iterator_tag>(false);
			RunFindIfNot<forward_iterator_tag>(false);
			RunFindIfNot<input_iterator_tag>(false);
		}

		template<typename _IterCat>
		void RunFindFirstOf(bool _Exist)
		{
			{  // seq
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_first_of(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_find(), _Alg.end_find()));
			}

			{  //par
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_first_of(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_find(), _Alg.end_find()));
			}

			{  //par_vec
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_first_of(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_find(), _Alg.end_find()));
			}
		}

		TEST_METHOD(FindFirstOf)
		{
			RunFindFirstOf<random_access_iterator_tag>(true);
			RunFindFirstOf<forward_iterator_tag>(true);
			RunFindFirstOf<input_iterator_tag>(true);

			RunFindFirstOf<random_access_iterator_tag>(false);
			RunFindFirstOf<forward_iterator_tag>(false);
			RunFindFirstOf<input_iterator_tag>(false);
		}

		template<typename _IterCat>
		void RunFindFirstOfPredicate(bool _Exist)
		{
			{  // seq
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_first_of(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_find(), _Alg.end_find(), _Alg.first_of_callback()));
			}

			{  //par
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_first_of(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_find(), _Alg.end_find(), _Alg.first_of_callback()));
			}

			{  //par_vec
				FindAlgoTest<_IterCat> _Alg(_Exist);
				_Alg.set_result(find_first_of(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_find(), _Alg.end_find(), _Alg.first_of_callback()));
			}
		}

		TEST_METHOD(FindFirstOfCallback)
		{
			RunFindFirstOfPredicate<random_access_iterator_tag>(true);
			RunFindFirstOfPredicate<forward_iterator_tag>(true);
			RunFindFirstOfPredicate<input_iterator_tag>(true);

			RunFindFirstOfPredicate<random_access_iterator_tag>(false);
			RunFindFirstOfPredicate<forward_iterator_tag>(false);
			RunFindFirstOfPredicate<input_iterator_tag>(false);
		}

		template<typename _IterCat>
		void RunFindEnd(bool _Exists)
		{
			{  // seq
				FindEndAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(find_end(seq, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle)));
			}

			{  //par
				FindEndAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(find_end(par, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle)));
			}

			{  //par_vec
				FindEndAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(find_end(par_vec, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle)));
			}
		}

		template<typename _IterCat>
		void RunFindEndPredicate(bool _Exists)
		{
			{  // seq
				FindEndAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(find_end(seq, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle), _Alg.callback()));
			}

			{  //par
				FindEndAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(find_end(par, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle), _Alg.callback()));
			}

			{  //par_vec
				FindEndAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(find_end(par_vec, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle), _Alg.callback()));
			}
		}

		TEST_METHOD(FindEnd)
		{
			RunFindEnd<random_access_iterator_tag>(true);
			RunFindEnd<random_access_iterator_tag>(false);

			RunFindEnd<forward_iterator_tag>(true);
			RunFindEnd<forward_iterator_tag>(false);

			// Overload with predicate
			RunFindEndPredicate<random_access_iterator_tag>(true);
			RunFindEndPredicate<random_access_iterator_tag>(false);

			RunFindEndPredicate<forward_iterator_tag>(true);
			RunFindEndPredicate<forward_iterator_tag>(false);
		}

		TEST_METHOD(FindEndEdgeCases)
		{
			std::vector<int> empty_vec;
			std::vector<int> vec = { 2, 1 };
			std::vector<int> needle_empty_vec;
			std::vector<int> needle_vec = { 1, 2, 3 };
			std::vector<int> needle_one_vec = { 1 };

			Assert::IsTrue(find_end(par, std::begin(empty_vec), std::end(empty_vec), std::begin(needle_empty_vec), std::end(needle_empty_vec)) == std::end(empty_vec)); // Empty collection
			Assert::IsTrue(find_end(par, std::begin(empty_vec), std::end(empty_vec), std::begin(needle_vec), std::end(needle_vec)) == std::end(empty_vec)); // Search needle is bigger than collection
			Assert::IsTrue(find_end(par, std::begin(vec), std::end(vec), std::begin(needle_empty_vec), std::end(needle_empty_vec)) == std::end(vec)); // Nothing to search	
			Assert::IsTrue(std::distance(std::begin(vec), find_end(par, std::begin(vec), std::end(vec), std::begin(needle_one_vec), std::end(needle_one_vec))) == 1); // Needle one elemen
			Assert::IsTrue(find_end(par, std::begin(vec), std::end(vec), std::begin(needle_vec), std::end(needle_vec)) == std::end(vec));// Search needle is bigger than collection

			std::vector<int> double_vec = { 0, 1, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 3, 1, 1, 2 }; // Last values should be match
			std::vector<int> double_needle = { 1, 2, 3 };
			// Double match, making sure that find the furthest match
			auto _Iter = find_end(par, std::begin(double_vec), std::end(double_vec), std::begin(double_needle), std::end(double_needle));
			Assert::IsTrue(std::distance(std::begin(double_vec), _Iter) == 13);
		}

		TEST_METHOD(FindEndOnChunkBorders)
		{
			const int COLLECTION_SIZE = 2048;
			const int MATCH_ELEMENTS = 4;

			_ASSERTE(MATCH_ELEMENTS < (COLLECTION_SIZE / 2));

			debug_static_execution_policy dbg(COLLECTION_SIZE - MATCH_ELEMENTS + 1); // The algorithm implementation sub the elements count thus adjastment here
			auto _Pos = dbg.get_chunk_size();

			_ASSERTE(_Pos >(MATCH_ELEMENTS * 3)); // If hit increase COLLECTION_SIZE
			
			// Test machine has one core only
			if (dbg.get_chunk_count() < 2)
				return;

			if (_Pos < MATCH_ELEMENTS)
				_Pos *= 2;

			std::vector<int> needle(MATCH_ELEMENTS);
			for (int i = 0; i < MATCH_ELEMENTS; ++i)
				needle[i] = i;

			std::vector<int>::iterator _It;
			std::vector<int> vec(COLLECTION_SIZE);
			std::iota(std::begin(vec), std::end(vec), MATCH_ELEMENTS);

			for (size_t i = 0; i < MATCH_ELEMENTS; ++i)
				vec[_Pos - (MATCH_ELEMENTS / 2) + i] = needle[i];

			// The match is between two chunks
			_It = find_end(par, std::begin(vec), std::end(vec), std::begin(needle), std::end(needle));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == (_Pos - (MATCH_ELEMENTS / 2)));

			// The match is at the end of the chunk
			std::iota(std::begin(vec), std::end(vec), 5);

			for (size_t i = 0; i < needle.size(); ++i)
				vec[_Pos - MATCH_ELEMENTS + i] = needle[i];

			_It = find_end(par, std::begin(vec), std::end(vec), std::begin(needle), std::end(needle));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == (_Pos - MATCH_ELEMENTS));

			// The match is at the beginning of the chunk
			std::iota(std::begin(vec), std::end(vec), 5);

			for (size_t i = 0; i < needle.size(); ++i)
				vec[_Pos + i] = needle[i];

			_It = find_end(par, std::begin(vec), std::end(vec), std::begin(needle), std::end(needle));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == _Pos);

			// 2 maching elements in the same chunk
			std::iota(std::begin(vec), std::end(vec), MATCH_ELEMENTS);

			for (size_t i = 0; i < needle.size(); ++i)
				vec[_Pos + i] = needle[i];
			
			// Add another needle in the same chunk on the further position
			for (size_t i = 0; i < needle.size(); ++i)
				vec[_Pos + (MATCH_ELEMENTS * 2) + i] = needle[i];

			_It = find_end(par, std::begin(vec), std::end(vec), std::begin(needle), std::end(needle));
			// Find the last element in the chunk
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == (_Pos + (MATCH_ELEMENTS * 2)));
		}
	};
} // ParallelSTL_Tests
