#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(ParitionTest)
	{
		template<typename _IterCat>
		struct PartitionAlgoTest :
			public AlgoTest<PartitionAlgoTest<_IterCat>, _IterCat, size_t>
		{
		private:
			bool _Stable;
		public:
			PartitionAlgoTest(bool _S = false) : _Stable(_S)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), rand());
			}

			~PartitionAlgoTest()
			{
				Assert::IsTrue(std::all_of(std::begin(_Ct), _Result.get_inner(), callback()));
				Assert::IsTrue(std::all_of(_Result.get_inner(), std::end(_Ct), [](size_t _Val){ return _Val % 2 == 0; }));

				if (_Stable) {
					Assert::IsTrue(std::is_sorted(std::begin(_Ct), _Result.get_inner()));
					Assert::IsTrue(std::is_sorted(_Result.get_inner(), std::end(_Ct)));
				}
			}

			std::function<bool(size_t)> callback() {
				return std::function<bool(size_t)>([](size_t _Val){
					return _Val % 2 == 1;
				});
			}
		};

		template<typename _IterCat>
		void RunPartition()
		{
			{  // seq
				PartitionAlgoTest<_IterCat> _Alg;
				_Alg.set_result(partition(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par

				PartitionAlgoTest<_IterCat> _Alg;
				_Alg.set_result(partition(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //vec
				PartitionAlgoTest<_IterCat> _Alg;
				_Alg.set_result(partition(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(Partition)
		{
			RunPartition<random_access_iterator_tag>();
			RunPartition<forward_iterator_tag>();
		}

		TEST_METHOD(PartitionEdgeCases)
		{
			std::vector<int> vec_empty;

			Assert::IsTrue(partition(par, std::begin(vec_empty), std::end(vec_empty), [](int){ return true; }) == std::end(vec_empty));

			std::vector<int> vec_one = { 1 };

			Assert::IsTrue(partition(par, std::begin(vec_one), std::end(vec_one), [](int){ return true; }) == std::end(vec_one));
			Assert::IsTrue(partition(par, std::begin(vec_one), std::end(vec_one), [](int){ return false; }) == std::begin(vec_one));

			std::vector<int> vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

			Assert::IsTrue(partition(par, std::begin(vec), std::end(vec), [](int){ return true; }) == std::end(vec));
			Assert::IsTrue(partition(par, std::begin(vec), std::end(vec), [](int){ return false; }) == std::begin(vec));
		}

		template<typename _IterCat>
		void RunStablePartition()
		{
			{  // seq
				PartitionAlgoTest<_IterCat> _Alg(true);
				_Alg.set_result(stable_partition(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par
				PartitionAlgoTest<_IterCat> _Alg(true);
				_Alg.set_result(stable_partition(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //vec
				PartitionAlgoTest<_IterCat> _Alg(true);
				_Alg.set_result(stable_partition(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(StablePartition)
		{
			RunStablePartition<random_access_iterator_tag>();
			RunStablePartition<bidirectional_iterator_tag>();
		}

		TEST_METHOD(StablePartitionEdgeCases)
		{
			std::vector<int> vec_empty;
			
			Assert::IsTrue(stable_partition(par, std::begin(vec_empty), std::end(vec_empty), [](int){ return true; }) == std::end(vec_empty));

			std::vector<int> vec_one = { 1 };

			Assert::IsTrue(stable_partition(par, std::begin(vec_one), std::end(vec_one), [](int){ return true; }) == std::end(vec_one));
			Assert::IsTrue(stable_partition(par, std::begin(vec_one), std::end(vec_one), [](int){ return false; }) == std::begin(vec_one));

			std::vector<int> vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

			Assert::IsTrue(stable_partition(par, std::begin(vec), std::end(vec), [](int){ return true; }) == std::end(vec));
			Assert::IsTrue(stable_partition(par, std::begin(vec), std::end(vec), [](int){ return false; }) == std::begin(vec));
		}
	};

	TEST_CLASS(ParitionCopyTest)
	{
		template<typename _IterCat, typename _OutIterCat = _IterCat>
		struct ParitionCopyAlgoTest :
			public AlgoTest<ParitionCopyAlgoTest<_IterCat, _OutIterCat>, _IterCat, size_t, void, _OutIterCat>
		{
			container _Ct_false; // false results of predicate
			std::pair<out_iterator, out_iterator> _Res;

			ParitionCopyAlgoTest()
			{
				std::iota(std::begin(_Ct), std::end(_Ct), rand());
				_Ct_copy.resize(_Ct.size());
				_Ct_false.resize(_Ct.size());
			}

			~ParitionCopyAlgoTest()
			{				
				Assert::IsTrue(std::all_of(std::begin(_Ct_copy), _Res.first.get_inner(), this->callback()));
				Assert::IsTrue(std::none_of(std::begin(_Ct_false), _Res.second.get_inner(), this->callback()));
			}

			std::function<bool(size_t)> callback() {
				return std::function<bool(size_t)>([](size_t _Val){
					return _Val % 3 > 0 ? true : false;
				});
			}

			void set_pair_result(const std::pair<out_iterator, out_iterator>& _R)
			{
				_Res = _R;
			}

			typename out_iterator begin_dest2()
			{
				return make_test_iterator<out_iterator::iterator_category>(std::begin(_Ct_false));
			}
		};

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunPartitionCopy()
		{
			{  // seq
				ParitionCopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_pair_result(partition_copy(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.begin_dest2(), _Alg.callback()));
			}

			{  //par

				ParitionCopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_pair_result(partition_copy(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.begin_dest2(), _Alg.callback()));
			}

			{  //vec
				ParitionCopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_pair_result(partition_copy(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.begin_dest2(), _Alg.callback()));
			}
		}

		TEST_METHOD(PartitionCopy)
		{
			RunPartitionCopy<random_access_iterator_tag>();
			RunPartitionCopy<forward_iterator_tag>();
			RunPartitionCopy<input_iterator_tag, output_iterator_tag>();
		}

		TEST_METHOD(PartitionCopyEdgeCases)
		{
			std::vector<int> vec_empty, vec_true, vec_false;
	
			auto _Res = partition_copy(std::begin(vec_empty), std::end(vec_empty), std::begin(vec_true), std::begin(vec_false), [](int){ return true; });
			Assert::IsTrue(std::end(vec_true) ==_Res.first);
			Assert::IsTrue(std::end(vec_false) == _Res.second);

			std::vector<int> vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 , 11, 12, 13, 14, 15, 16 };

			vec_true.resize(vec.size());
			vec_false.resize(vec.size());

			_Res = partition_copy(std::begin(vec), std::end(vec), std::begin(vec_true), std::begin(vec_false), [](int){ return true; });
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec_true), _Res.first)) == vec.size());
			Assert::IsTrue(std::begin(vec_false) == _Res.second);

			_Res = partition_copy(std::begin(vec), std::end(vec), std::begin(vec_true), std::begin(vec_false), [](int){ return false; });
			Assert::IsTrue(std::begin(vec_true) == _Res.first);
			Assert::IsTrue(static_cast<size_t>(std::distance(std::begin(vec_false), _Res.second)) == vec.size());
		}
	};
} // namespace ParallelSTL_Tests
