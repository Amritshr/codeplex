#include "stdafx.h"

#include <execution_policy_utils.h>

namespace ParallelSTL_Tests
{
	TEST_CLASS(IsPartitionedTest)
	{
		const static size_t MARKER_VALUE = 1;

		template<typename _IterCat>
		struct IsPartitionedAlgoTest :
			public AlgoTest<IsPartitionedAlgoTest<_IterCat>, _IterCat, size_t, bool>
		{
			bool _Partitioned;
		public:
			IsPartitionedAlgoTest(bool _IsPartitioned) : _Partitioned(_IsPartitioned)
			{
				if (_IsPartitioned) {
					auto _Part_point = rand() % _Ct.size();
					std::fill_n(std::begin(_Ct), _Part_point, MARKER_VALUE + rand());
				}
				else {
					_ASSERTE(_Ct.size() > 3);
					_Ct[0] = MARKER_VALUE;
					_Ct[2] = MARKER_VALUE;
				}
			}

			~IsPartitionedAlgoTest()
			{
				Assert::AreEqual(_Result, _Partitioned);
			}

			std::function<bool(size_t)> callback()
			{
				return std::function<bool(size_t)>([](size_t _Val){
					return _Val >= MARKER_VALUE;
				});
			}
		};

		template<typename _IterCat>
		void RunIsPartitioned(bool _IsPartitioned)
		{
			{  // seq
				IsPartitionedAlgoTest<_IterCat> _Alg(_IsPartitioned);
				_Alg.set_result(is_partitioned(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
			
			{  //par
				IsPartitionedAlgoTest<_IterCat> _Alg(_IsPartitioned);
				_Alg.set_result(is_partitioned(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par_vec
				IsPartitionedAlgoTest<_IterCat> _Alg(_IsPartitioned);
				_Alg.set_result(is_partitioned(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(IsPartitionedEdgeCases)
		{
			std::vector<int> vec_empty;
			Assert::IsTrue(is_partitioned(par, std::begin(vec_empty), std::end(vec_empty), [](int _Val) { return _Val == MARKER_VALUE; }));

			std::vector<int> vec_one = { 1 };
			Assert::IsTrue(is_partitioned(par, std::begin(vec_one), std::end(vec_one), [](int _Val) { return _Val == MARKER_VALUE; }));

			std::vector<int> vec_two_10 = { 1, 0 };
			std::vector<int> vec_two_11 = { 1, 1 };
			std::vector<int> vec_two_01 = { 0, 1 };
			std::vector<int> vec_two_00 = { 0, 0 };

			Assert::IsTrue(is_partitioned(par, std::begin(vec_two_10), std::end(vec_two_10), [](int _Val) { return _Val == MARKER_VALUE; }));
			Assert::IsTrue(is_partitioned(par, std::begin(vec_two_11), std::end(vec_two_11), [](int _Val) { return _Val == MARKER_VALUE; }));
			Assert::IsFalse(is_partitioned(par, std::begin(vec_two_01), std::end(vec_two_01), [](int _Val) { return _Val == MARKER_VALUE; }));
			Assert::IsTrue(is_partitioned(par, std::begin(vec_two_00), std::end(vec_two_00), [](int _Val) { return _Val == MARKER_VALUE; }));
		}

		TEST_METHOD(IsPartitioned)
		{
			RunIsPartitioned<random_access_iterator_tag>(false);
			RunIsPartitioned<forward_iterator_tag>(false);
			RunIsPartitioned<input_iterator_tag>(false);

			RunIsPartitioned<random_access_iterator_tag>(true);
			RunIsPartitioned<forward_iterator_tag>(true);
			RunIsPartitioned<input_iterator_tag>(true);
		}

		TEST_METHOD(IsPartitionedOnChunkBorders)
		{
			const size_t COLLECTION_SIZE = 1024;
			debug_static_execution_policy dbg(COLLECTION_SIZE);
			auto _Pos = dbg.get_chunk_size();

			if (dbg.get_chunk_count() < 2)
				return;

			// Partition point on the chunk border
			std::vector<int> vec(COLLECTION_SIZE, MARKER_VALUE);
			std::fill_n(std::begin(vec), _Pos, 0);
			Assert::IsFalse(is_partitioned(dbg, std::begin(vec), std::end(vec), [](int _Val){ return _Val >= MARKER_VALUE; }));
			
			// Partition point before border
			std::vector<int> vec2(COLLECTION_SIZE, MARKER_VALUE);
			std::fill_n(std::begin(vec2), _Pos - 1, 0);
			Assert::IsFalse(is_partitioned(dbg, std::begin(vec2), std::end(vec2), [](int _Val){ return _Val >= MARKER_VALUE; }));

			// Partition point after border
			std::vector<int> vec3(COLLECTION_SIZE, MARKER_VALUE);
			std::fill_n(std::begin(vec3), _Pos + 1, 0);
			Assert::IsFalse(is_partitioned(dbg, std::begin(vec3), std::end(vec3), [](int _Val){ return _Val >= MARKER_VALUE; }));
		}
	};
} // ParallelSTL_Tests

