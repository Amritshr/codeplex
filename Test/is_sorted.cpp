#include "stdafx.h"

#include <execution_policy_utils.h>

namespace ParallelSTL_Tests
{
	TEST_CLASS(IsSortedTest)
	{
		template<typename _IterCat>
		struct SortAlgoTest :
			public AlgoTest<SortAlgoTest<_IterCat>, _IterCat, size_t, bool>
		{
			bool _Sorted;
		public:
			SortAlgoTest(bool _IsSorted) : _Sorted(_IsSorted)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), 0);

				auto _Pos = rand() % (_Ct.size() - 1);
				_Ct[_Pos] = _IsSorted ? _Ct[_Pos] : _Ct.size() + 1;
			}

			~SortAlgoTest()
			{
				Assert::AreEqual(_Result, _Sorted);
			}

			std::function<bool(size_t, size_t)> less()
			{
				return std::function<bool(size_t, size_t)>([](size_t _First, size_t _Next){
					return _First < _Next;
				});
			}
		};

		template<typename _IterCat>
		void RunIsSorted(bool _IsSorted)
		{
			{  // seq
				SortAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted(seq, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par
				SortAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted(par, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par_vec
				SortAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted(par_vec, _Alg.begin_in(), _Alg.end_in()));
			}
		}

		template<typename _IterCat>
		void RunIsSortedCallback(bool _IsSorted)
		{
			{  // seq
				SortAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.less()));
			}

			{  //par
				SortAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted(par, _Alg.begin_in(), _Alg.end_in(), _Alg.less()));
			}

			{  //par_vec
				SortAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.less()));
			}
		}

		TEST_METHOD(IsSortedEdgeCases)
		{
			std::vector<int> vec_empty;
			Assert::IsTrue(is_sorted(par, std::begin(vec_empty), std::end(vec_empty)));

			std::vector<int> vec_one = { 1 };
			Assert::IsTrue(is_sorted(par, std::begin(vec_one), std::end(vec_one)));

			// Sequence with duplicates
			std::vector<int> vec_dups = { 0, 1, 1, 1, 2 };
			Assert::IsTrue(is_sorted(par, std::begin(vec_dups), std::end(vec_dups)));

			// Compile time check only
			std::vector<MovableOnly> vec_movable, vec_moveable_dest;
			is_sorted(par, std::begin(vec_movable), std::end(vec_movable), [](MovableOnly&, MovableOnly&){ return true; });

			// Check the copy counts
			std::vector<CopyableOnly> vec_copyable(10);
			is_sorted(par, std::begin(vec_copyable), std::end(vec_copyable), [](CopyableOnly&, CopyableOnly&){ return true; });

			std::for_each(std::begin(vec_copyable), std::end(vec_copyable), [](CopyableOnly& _Val){
				Assert::AreEqual(_Val._Copy_count, 0); // No coppies
			});
		}

		TEST_METHOD(IsSortedOnChunkBorders)
		{
			const size_t COLLECTION_SIZE = 1024;
			debug_static_execution_policy dbg(COLLECTION_SIZE);
			auto _Pos = dbg.get_chunk_size();

			std::vector<int> vec(COLLECTION_SIZE);
			std::iota(std::begin(vec), std::end(vec), 0);			
			vec[_Pos] = COLLECTION_SIZE; // not sorted element at the beginning of the chunk
			Assert::IsFalse(is_sorted(dbg, std::begin(vec), std::end(vec)));

			std::vector<int> vec2(COLLECTION_SIZE);
			std::iota(std::begin(vec2), std::end(vec2), 0);
			vec2[_Pos - 1] = COLLECTION_SIZE; // not sorted element at the end of the chunk
			Assert::IsFalse(is_sorted(dbg, std::begin(vec2), std::end(vec2)));
	
			std::vector<int> vec3(COLLECTION_SIZE);
			std::iota(std::begin(vec3), std::end(vec3), 0);
			vec3[_Pos - 1] = COLLECTION_SIZE; // not sorted element at the beginning of the chunk
			vec3[_Pos] = COLLECTION_SIZE; // not sorted element at the end of the chunk
			Assert::IsFalse(is_sorted(dbg, std::begin(vec3), std::end(vec3)));
		}

		TEST_METHOD(IsSorted)
		{
			RunIsSorted<random_access_iterator_tag>(true);
			RunIsSorted<forward_iterator_tag>(true);

			RunIsSorted<random_access_iterator_tag>(false);
			RunIsSorted<forward_iterator_tag>(false);
		}

		TEST_METHOD(IsSortedCallback)
		{
			RunIsSortedCallback<random_access_iterator_tag>(true);
			RunIsSortedCallback<forward_iterator_tag>(true);
			
			RunIsSortedCallback<random_access_iterator_tag>(false);
			RunIsSortedCallback<forward_iterator_tag>(false);
		}
	};

	TEST_CLASS(IsSortedUntilTest)
	{
		template<typename _IterCat>
		struct SortUntilAlgoTest :
			public AlgoTest<SortUntilAlgoTest<_IterCat>, _IterCat, size_t>
		{
		public:
			SortUntilAlgoTest(bool _IsSorted)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), 0);
				_Last_pos = _Ct.size();

				if (!_IsSorted) {
					const size_t _Size = _Ct.size();
					size_t _Second, _First = rand() % (_Size - 1); // Pick 2 not sorted elements

					do {
						_Second = rand() % (_Size - 1);
					} while (_Second != _First);

					_Ct[_First] = _Size; // Assign value out of the range
					_Ct[_Second] = _Size;

					_Last_pos = _First < _Second ? _First : _Second;
					_Last_pos += 1;
				}
			}

			~SortUntilAlgoTest()
			{
				container::size_type _Diff = std::distance(begin_in(), _Result);
				Assert::AreEqual(_Diff, _Last_pos);
			}

			std::function<bool(size_t, size_t)> less()
			{
				return std::function<bool(size_t, size_t)>([](size_t _First, size_t _Next){
					return _First < _Next;
				});
			}
		private:
			size_t _Last_pos;
		};


		template<typename _IterCat>
		void RunIsSortedUntil(bool _IsSorted)
		{
			{  // seq
				SortUntilAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted_until(seq, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par
				SortUntilAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted_until(par, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par_vec
				SortUntilAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted_until(par_vec, _Alg.begin_in(), _Alg.end_in()));
			}
		}

		template<typename _IterCat>
		void RunIsSortedUntilCallback(bool _IsSorted)
		{
			{  // seq
				SortUntilAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted_until(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.less()));
			}

			{  //par
				SortUntilAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted_until(par, _Alg.begin_in(), _Alg.end_in(), _Alg.less()));
			}

			{  //par_vec
				SortUntilAlgoTest<_IterCat> _Alg(_IsSorted);
				_Alg.set_result(is_sorted_until(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.less()));
			}
		}

		TEST_METHOD(IsSortedUntilOnChunkBorders)
		{
			const size_t COLLECTION_SIZE = 1024;
			debug_static_execution_policy dbg(COLLECTION_SIZE);
			auto _Pos = dbg.get_chunk_size();

			std::vector<int>::iterator _It;

			std::vector<int> vec(COLLECTION_SIZE);
			std::iota(std::begin(vec), std::end(vec), 0);
			vec[_Pos] = COLLECTION_SIZE; // not sorted element at the beginning of the chunk
			_It = is_sorted_until(dbg, std::begin(vec), std::end(vec));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec), _It)) == (_Pos + 1));

			std::vector<int> vec2(COLLECTION_SIZE);
			std::iota(std::begin(vec2), std::end(vec2), 0);
			vec2[_Pos - 1] = COLLECTION_SIZE; // not sorted element at the end of the chunk
			_It = is_sorted_until(dbg, std::begin(vec2), std::end(vec2));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec2), _It)) == _Pos);

			std::vector<int> vec3(COLLECTION_SIZE);
			std::iota(std::begin(vec3), std::end(vec3), 0);
			vec3[_Pos - 1] = COLLECTION_SIZE; // not sorted element at the beginning of the chunk
			vec3[_Pos] = COLLECTION_SIZE - 1; // not sorted element at the end of the chunk
			_It = is_sorted_until(dbg, std::begin(vec3), std::end(vec3));
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec3), _It)) == _Pos);
		}

		TEST_METHOD(IsSortedUntilEdgeCases)
		{
			std::vector<int> vec_empty;
			Assert::IsTrue(is_sorted_until(par, std::begin(vec_empty), std::end(vec_empty)) == std::end(vec_empty));

			std::vector<int> vec_one = { 1 };
			Assert::IsTrue(is_sorted_until(par, std::begin(vec_one), std::end(vec_one)) == std::end(vec_one));

			// Compile time check only
			std::vector<MovableOnly> vec_movable, vec_moveable_dest;
			is_sorted_until(par, std::begin(vec_movable), std::end(vec_movable), [](MovableOnly&, MovableOnly&){ return true; });

			// Check the copy counts
			std::vector<CopyableOnly> vec_copyable(10);
			is_sorted_until(par, std::begin(vec_copyable), std::end(vec_copyable), [](CopyableOnly&, CopyableOnly&){ return true; });

			std::for_each(std::begin(vec_copyable), std::end(vec_copyable), [](CopyableOnly& _Val){
				Assert::AreEqual(_Val._Copy_count, 0); // No coppies
			});
		}

		TEST_METHOD(IsSortedUntil)
		{
			RunIsSortedUntil<random_access_iterator_tag>(true);
			RunIsSortedUntil<forward_iterator_tag>(true);		
			// Negative test when it's not sorted
			RunIsSortedUntil<random_access_iterator_tag>(false);
			RunIsSortedUntil<forward_iterator_tag>(false);
		}

		TEST_METHOD(IsSortedUntilCallback)
		{
			RunIsSortedUntilCallback<random_access_iterator_tag>(true);
			RunIsSortedUntilCallback<forward_iterator_tag>(true);
			// Negative test when it's not sorted
			RunIsSortedUntilCallback<random_access_iterator_tag>(false);
			RunIsSortedUntilCallback<forward_iterator_tag>(false);		
		}
	};

} // ParallelSTL_Tests
