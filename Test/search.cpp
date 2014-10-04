#include "stdafx.h"

#include <execution_policy_utils.h>

namespace ParallelSTL_Tests
{
	TEST_CLASS(SearchTest)
	{
		const static size_t MARKER_VALUE = 1;

		template<typename _IterCat>
		struct SearchNAlgoTest :
			public AlgoTest<SearchNAlgoTest<_IterCat>, _IterCat, size_t>
		{
			bool _ValueExits;
			size_t _ValuePos;
		public:
			typename container::difference_type _ValueCount;

			SearchNAlgoTest(bool _Exists) : _ValueExits(_Exists)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), rand() + MARKER_VALUE * 2);
				const size_t _Size = _Ct.size() - 2;

				_ValuePos = rand() % _Size;
				_ValueCount = rand() % (_Size - _ValuePos);
				if (_ValueCount < 2)
					_ValueCount = 1;

				if (_ValueExits) {
					for (typename container::difference_type _I = 0; _I < _ValueCount; ++_I)
						_Ct[_ValuePos + _I] = MARKER_VALUE;
				}
			}

			~SearchNAlgoTest()
			{
				if (_ValueExits) {
					Assert::IsTrue(_Result != end_in());

					container::size_type _Pos = std::distance(begin_in(), _Result);
					Assert::IsTrue(_Pos == _ValuePos);
				}
				else Assert::IsTrue(_Result == end_in());
			}

			std::function<bool(size_t, size_t)> callback()
			{
				return std::function<bool(size_t, size_t)>([](size_t _First, size_t){
					return _First < MARKER_VALUE * 2;
				});
			}
		};

		template<typename _IterCat>
		struct SearchAlgoTest :
			public AlgoTest<SearchAlgoTest<_IterCat>, _IterCat, size_t>
		{
			bool _ValueExits;
			size_t _ValuePos;
			typename container::difference_type _ValueCount;
		public:
			std::vector<size_t> _Needle;

			SearchAlgoTest(bool _Exists) : _ValueExits(_Exists)
			{
				const size_t _Size = _Ct.size() - 2;

				_ValuePos = rand() % _Size;
				_ValueCount = rand() % (_Size - _ValuePos);
				if (_ValueCount < 2)
					_ValueCount = 1;

				_Needle.resize(_ValueCount);
				std::iota(std::begin(_Needle), std::end(_Needle), MARKER_VALUE);
				std::iota(std::begin(_Ct), std::end(_Ct), rand() + _Needle.size() + MARKER_VALUE);

				if (_ValueExits) {
					for (size_t _Idx = 0; _Idx < _Needle.size(); ++_Idx)
						_Ct[_ValuePos + _Idx] = _Needle[_Idx];
				}
			}

			~SearchAlgoTest()
			{
				if (_ValueExits) {
					Assert::IsTrue(_Result != end_in());

					container::size_type _Pos = std::distance(begin_in(), _Result);
					Assert::IsTrue(_Pos == _ValuePos);
				}
				else Assert::IsTrue(_Result == end_in());
			}

			std::function<bool(size_t, size_t)> callback()
			{
				return std::function<bool(size_t, size_t)>([&](size_t _First, size_t){
					return _First < this->_Needle.size() + 1;
				});
			}
		};

		TEST_METHOD(SearchEdgeCases)
		{
			std::vector<int> empty_vec;
			std::vector<int> vec = { 2, 1 };
			std::vector<int> needle_empty_vec;
			std::vector<int> needle_vec = { 1, 2, 3 };
			std::vector<int> needle_one_vec = { 1 };

			Assert::IsTrue(search(par, std::begin(empty_vec), std::end(empty_vec), std::begin(needle_empty_vec), std::end(needle_empty_vec)) == std::end(empty_vec)); // Empty collection
			Assert::IsTrue(search(par, std::begin(empty_vec), std::end(empty_vec), std::begin(needle_vec), std::end(needle_vec)) == std::end(empty_vec)); // Search needle is bigger than collection
			Assert::IsTrue(search(par, std::begin(vec), std::end(vec), std::begin(needle_empty_vec), std::end(needle_empty_vec)) == std::begin(vec)); // Nothing to search	
			Assert::IsTrue(std::distance(std::begin(vec), search(par, std::begin(vec), std::end(vec), std::begin(needle_one_vec), std::end(needle_one_vec))) == 1); // Needle one element

			Assert::IsTrue(search(par, std::begin(vec), std::end(vec), std::begin(needle_vec), std::end(needle_vec)) == std::end(vec));// Search needle is bigger than collection

			std::vector<int> double_vec = { 0, 1, 2, 3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 2, 3, 1, 1, 2 }; // Last values should be match
			std::vector<int> double_needle = { 1, 2, 3 };
			// Double match, making sure that find the closest match
			Assert::IsTrue(std::distance(std::begin(double_vec), search(par, std::begin(double_vec), std::end(double_vec), std::begin(double_needle), std::end(double_needle))) == 1);
		}

		TEST_METHOD(SearchNEdgeCases)
		{
			std::vector<int> empty_vec, sec_vec = { 1, 2, 3 };

			Assert::IsTrue(search_n(par, std::begin(empty_vec), std::end(empty_vec), 1, 1) == std::end(empty_vec)); // Empty collection
			Assert::IsTrue(search_n(par, std::begin(sec_vec), std::end(sec_vec), 1, 20) == std::end(sec_vec)); // Search needle is bigger than collection
			Assert::IsTrue(search_n(par, std::begin(empty_vec), std::end(empty_vec), 1, 0) == std::begin(empty_vec)); // Nothing to search

			std::vector<int> double_vec = { 0, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 2, 1 }; // Last values should be match
			// Double match, making sure that find the closest match
			auto _Iter = search_n(par, std::begin(double_vec), std::end(double_vec), 2, 1);
			Assert::IsTrue(std::distance(std::begin(double_vec), _Iter) == 1);
		}
	
		TEST_METHOD(SearchOnChunkBorders)
		{
			const int COLLECTION_SIZE = 1024;
			const int MATCH_ELEMENTS = 4;

			_ASSERTE(MATCH_ELEMENTS < (COLLECTION_SIZE / 2));

			debug_static_execution_policy dbg(COLLECTION_SIZE - MATCH_ELEMENTS + 1); // The algorithm implementation sub the elements count thus adjastment here
			auto _Pos = dbg.get_chunk_size();
			
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
			_It = search(par, std::begin(vec), std::end(vec), std::begin(needle), std::end(needle));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == (_Pos - (MATCH_ELEMENTS/ 2)));

			// The match is at the end of the chunk
			std::iota(std::begin(vec), std::end(vec), 5);

			for (size_t i = 0; i < needle.size(); ++i)
				vec[_Pos - MATCH_ELEMENTS + i] = needle[i];

			_It = search(par, std::begin(vec), std::end(vec), std::begin(needle), std::end(needle));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == (_Pos - MATCH_ELEMENTS));

			// The match is at the beginning of the chunk
			std::iota(std::begin(vec), std::end(vec), 5);

			for (size_t i = 0; i < needle.size(); ++i)
				vec[_Pos + i] = needle[i];

			_It = search(par, std::begin(vec), std::end(vec), std::begin(needle), std::end(needle));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == _Pos);		
		}

		TEST_METHOD(SearchNOnChunkBorders)
		{
			const int COLLECTION_SIZE = 1024;
			const int MATCH_ELEMENTS = 4;
			const int MATCH_VALUE = 1;

			_ASSERTE(MATCH_ELEMENTS < (COLLECTION_SIZE / 2));

			debug_static_execution_policy dbg(COLLECTION_SIZE - MATCH_ELEMENTS + 1); // The algorithm implementation sub the elements count thus adjastment here
			auto _Pos = dbg.get_chunk_size();

			// Test machine has one core only
			if (dbg.get_chunk_count() < 2)
				return;

			if (_Pos < MATCH_ELEMENTS)
				_Pos *= 2;

			std::vector<int>::iterator _It;
			std::vector<int> vec(COLLECTION_SIZE);
			std::iota(std::begin(vec), std::end(vec), MATCH_ELEMENTS);

			_It = std::begin(vec);
			std::advance(_It, _Pos - (MATCH_ELEMENTS/2));
			std::fill_n(_It, MATCH_ELEMENTS, MATCH_VALUE);

			// The match is between two chunks
			_It = search_n(par, std::begin(vec), std::end(vec), MATCH_ELEMENTS, MATCH_VALUE);
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == (_Pos - (MATCH_ELEMENTS/2)));

			// The match is at the end of the chunk
			std::iota(std::begin(vec), std::end(vec), MATCH_ELEMENTS + 1);
			_It = std::begin(vec);
			std::advance(_It, _Pos - MATCH_ELEMENTS);
			std::fill_n(_It, MATCH_ELEMENTS, MATCH_VALUE);

			_It = search_n(par, std::begin(vec), std::end(vec), MATCH_ELEMENTS, MATCH_VALUE);
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == (_Pos - MATCH_ELEMENTS));

			// The match is at the beginning of the chunk
			std::iota(std::begin(vec), std::end(vec), MATCH_ELEMENTS + 1);
			_It = std::begin(vec);
			std::advance(_It, _Pos);
			std::fill_n(_It, MATCH_ELEMENTS, MATCH_VALUE);

			_It = search_n(par, std::begin(vec), std::end(vec), MATCH_ELEMENTS, MATCH_VALUE);
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == _Pos);
		}
		
		template<typename _IterCat>
		void RunSearch(bool _Exists)
		{
			{  // seq
				SearchAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search(seq, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle)));
			}

			{  //par
				SearchAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search(par, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle)));
			}

			{  //par_vec
				SearchAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search(par_vec, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle)));
			}
		}

		template<typename _IterCat>
		void RunSearchPredicate(bool _Exists)
		{
			{  // seq
				SearchAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search(seq, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle), _Alg.callback()));
			}

			{  //par
				SearchAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search(par, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle), _Alg.callback()));
			}

			{  //par_vec
				SearchAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search(par_vec, _Alg.begin_in(), _Alg.end_in(), std::begin(_Alg._Needle), std::end(_Alg._Needle), _Alg.callback()));
			}
		}

		TEST_METHOD(Search)
		{
			RunSearch<random_access_iterator_tag>(true);
			RunSearch<random_access_iterator_tag>(false);

			RunSearch<forward_iterator_tag>(true);
			RunSearch<forward_iterator_tag>(false);

			// Overload with predicate
			RunSearchPredicate<random_access_iterator_tag>(true);
			RunSearchPredicate<random_access_iterator_tag>(false);

			RunSearchPredicate<forward_iterator_tag>(true);
			RunSearchPredicate<forward_iterator_tag>(false);
		}

		template<typename _IterCat>
		void RunSearchN(bool _Exists)
		{
			{  // seq
				SearchNAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search_n(seq, _Alg.begin_in(), _Alg.end_in(), _Alg._ValueCount, MARKER_VALUE));
			}

			{  //par
				SearchNAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search_n(par, _Alg.begin_in(), _Alg.end_in(), _Alg._ValueCount, MARKER_VALUE));
			}

			{  //par_vec
				SearchNAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search_n(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg._ValueCount, MARKER_VALUE));
			}
		}

		template<typename _IterCat>
		void RunSearchNPredicate(bool _Exists)
		{
			{  // seq
				SearchNAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search_n(seq, _Alg.begin_in(), _Alg.end_in(), _Alg._ValueCount, MARKER_VALUE, _Alg.callback()));
			}

			{  //par
				SearchNAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search_n(par, _Alg.begin_in(), _Alg.end_in(), _Alg._ValueCount, MARKER_VALUE, _Alg.callback()));
			}

			{  //par_vec
				SearchNAlgoTest<_IterCat> _Alg(_Exists);
				_Alg.set_result(search_n(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg._ValueCount, MARKER_VALUE, _Alg.callback()));
			}
		}

		TEST_METHOD(SearchN)
		{
			RunSearchN<random_access_iterator_tag>(true);
			RunSearchN<random_access_iterator_tag>(false);

			RunSearchN<forward_iterator_tag>(true);
			RunSearchN<forward_iterator_tag>(false);

			RunSearchNPredicate<random_access_iterator_tag>(true);
			RunSearchNPredicate<random_access_iterator_tag>(false);

			RunSearchNPredicate<forward_iterator_tag>(true);
			RunSearchNPredicate<forward_iterator_tag>(false);
		}
	};
} // ParallelSTL_Tests
