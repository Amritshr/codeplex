#include "stdafx.h"

namespace ParallelSTL_Tests
{
	const static size_t MARKER_VALUE = 1;

	template<typename _IterCat, typename _OutIterCat = _IterCat>
	struct ScanAlgoTest :
		public AlgoTest<ScanAlgoTest<_IterCat, _OutIterCat>, _IterCat, size_t, void, _OutIterCat>
	{
		ScanAlgoTest(bool _Ex) : _Exclusive(_Ex)
		{
			size_t _El = 0;
			std::for_each(std::begin(_Ct), std::end(_Ct), [&_El](size_t& _Val){
				_Val = _El;

				if (_El)
					_El = 0;
				else _El = 1;
			});

			_Ct_copy.resize(_Ct.size());
		}

		~ScanAlgoTest()
		{
			size_t _Sum = MARKER_VALUE;
			auto _Res = std::begin(_Ct);
			for (auto _It = std::begin(_Ct_copy); _It != std::end(_Ct_copy); ++_It) {
				if (_Exclusive == true && *_It != _Sum) {
					Assert::Fail();
					break;
				}

				_Sum += *_Res;
				++_Res;

				if (_Exclusive == false && *_It != _Sum) {
					Assert::Fail();
					break;
				}
			}

			Assert::IsTrue(_Result == std::end(_Ct_copy));
		}

		std::function<size_t(size_t, size_t)> callback() {
			return std::function<size_t(size_t, size_t)>([](size_t _Val, size_t _Val2) {
				return _Val + _Val2;
			});
		}
	private:
		bool _Exclusive;
	};

	TEST_CLASS(ExclusiveScanTest)
	{
		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunExclusiveScan()
		{
			{  // seq
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(exclusive_scan(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE));
			}

			{  //par
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(exclusive_scan(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE));
			}

			{  //vec
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(exclusive_scan(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE));
			}
		}

		TEST_METHOD(ExclusiveScan)
		{
			RunExclusiveScan<random_access_iterator_tag>();
			RunExclusiveScan<forward_iterator_tag>();
			RunExclusiveScan<input_iterator_tag, output_iterator_tag>();
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunExclusiveScanCallback()
		{
			{  // seq
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(exclusive_scan(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE, _Alg.callback()));
			}

			{  //par
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(exclusive_scan(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE, _Alg.callback()));
			}

			{  //vec
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(exclusive_scan(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE, _Alg.callback()));
			}
		}

		TEST_METHOD(ExclusiveScanCallback)
		{
			RunExclusiveScanCallback<random_access_iterator_tag>();
			RunExclusiveScanCallback<forward_iterator_tag>();
			RunExclusiveScanCallback<input_iterator_tag, output_iterator_tag>();
		}

		TEST_METHOD(ExclusiveScanEdgeCases)
		{
			std::vector<size_t> vec_empty, vec_out_empty;

			Assert::IsTrue(exclusive_scan(par, std::begin(vec_empty), std::end(vec_empty), std::begin(vec_out_empty), size_t{ 0 }) == std::begin(vec_out_empty));

			std::vector<size_t> vec_one = { 2 }, vec_out_one = { 1 };

			auto _It = exclusive_scan(par, std::begin(vec_one), std::end(vec_one), std::begin(vec_out_one), MARKER_VALUE);
			Assert::AreEqual(size_t{ 1 }, vec_out_one[0]);
			Assert::AreEqual(size_t{ 2 }, vec_one[0]);
			Assert::IsTrue(_It == std::end(vec_out_one));

			std::vector<size_t> vec = { 2, 3, 4, 5 }, vec_out = { 1, 1, 1, 1 };
			std::vector<size_t> expected = { 1, 3, 6, 10 };

			_It = exclusive_scan(par, std::begin(vec), std::end(vec), std::begin(vec_out), MARKER_VALUE);
			Assert::IsTrue(vec_out == expected);
			Assert::IsTrue(_It == std::end(vec_out));
		}
	};

	TEST_CLASS(InclusiveScanTest)
	{
		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunInclusiveScan()
		{
			{  // seq
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(inclusive_scan(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE));
			}

			{  //par
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(inclusive_scan(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE));
			}

			{  //vec
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(inclusive_scan(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE));
			}
		}

		TEST_METHOD(InclusiveScan)
		{
			RunInclusiveScan<random_access_iterator_tag>();
			RunInclusiveScan<forward_iterator_tag>();
			RunInclusiveScan<input_iterator_tag, output_iterator_tag>();
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunInclusiveScanCallback()
		{
			{  // seq
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(inclusive_scan(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE, _Alg.callback()));
			}

			{  //par
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(inclusive_scan(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE, _Alg.callback()));
			}

			{  //vec
				ScanAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(inclusive_scan(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VALUE, _Alg.callback()));
			}
		}

		TEST_METHOD(InclusiveScanCallback)
		{
			RunInclusiveScanCallback<random_access_iterator_tag>();
			RunInclusiveScanCallback<forward_iterator_tag>();
			RunInclusiveScanCallback<input_iterator_tag, output_iterator_tag>();
		}

		TEST_METHOD(InclusiveScanEdgeCases)
		{
			std::vector<size_t> vec_empty, vec_out_empty;

			Assert::IsTrue(inclusive_scan(par, std::begin(vec_empty), std::end(vec_empty), std::begin(vec_out_empty), size_t{ 0 }) == std::begin(vec_out_empty));

			std::vector<size_t> vec_one = { 2 }, vec_out_one = { 1 };

			auto _It = inclusive_scan(par, std::begin(vec_one), std::end(vec_one), std::begin(vec_out_one), MARKER_VALUE);
			Assert::AreEqual(size_t{ 3 }, vec_out_one[0]);
			Assert::AreEqual(size_t{ 2 }, vec_one[0]);
			Assert::IsTrue(_It == std::end(vec_out_one));

			std::vector<size_t> vec = { 2, 3, 4, 5 }, vec_out = { 1, 1, 1, 1 };
			std::vector<size_t> expected = { 3, 6, 10, 15 };

			_It = inclusive_scan(par, std::begin(vec), std::end(vec), std::begin(vec_out), MARKER_VALUE);
			Assert::IsTrue(vec_out == expected);
			Assert::IsTrue(_It == std::end(vec_out));
		}
	};
} // namespace ParallelSTL_Tests
