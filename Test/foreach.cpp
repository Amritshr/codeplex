#include "stdafx.h"

namespace ParallelSTL_Tests
{
	// Test for internal implementation of partitioners bypassing the algorithm interface
	template<class _ExPolicy, class _InIt, class _Diff, class _Fn1>
	inline _InIt for_each_impl(_InIt _First, _Diff _Count, _Fn1 _Func)
	{
		if (_Count > 0) {
			return details::_Partitioner<_ExPolicy>::_For_Each(_First, _Count, _Func, [](_InIt _Begin, size_t _Count, _Fn1 _UserFunc){

				details::LoopHelper<_ExPolicy, _InIt>::Loop(_Begin, _Count, [&_UserFunc](const _InIt::reference _It){
					_UserFunc(_It);
				});
			});
		}

		return _First;
	}

	TEST_CLASS(ForEachTest)
	{
		template<typename _IterCat>
		struct ForEachAlgoTest :
			public AlgoTest<ForEachAlgoTest<_IterCat>, _IterCat, size_t>
		{
			bool _ReturnValue;

			ForEachAlgoTest(bool _Ret = true) : _ReturnValue(_Ret)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), 0);
				_Ct_copy.resize(_Ct.size());
			}

			~ForEachAlgoTest()
			{
				container _Ref(_Ct.size());
				std::iota(begin(_Ref), end(_Ref), size_t{ 1 });
				Assert::IsTrue(std::equal(begin(_Ct), end(_Ct), begin(_Ref)));

				if (_ReturnValue)
					Assert::IsTrue(end_in() == _Result);
			}

			std::function<void(size_t&)> callback()
			{
				return std::function<void(size_t&)>([](size_t& _Val){
					_Val = _Val + 1;
				});
			}
		};

		template<typename _IterCat>
		struct ForEachExAlgoTest :
			public AlgoTest<ForEachExAlgoTest<_IterCat>, _IterCat, size_t>
		{
			ForEachExAlgoTest() : _Exception_count(0)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), 0);
			}

			~ForEachExAlgoTest()
			{
				Assert::IsTrue(_Exception != nullptr);
				if (_Exception != nullptr) {
					try {
						std::rethrow_exception(_Exception);
					}
					catch (const exception_list& e) {
						Assert::AreEqual(static_cast<container::size_type>(_Exception_count), e.size());

						for (auto const& ex : e) {
							try {
								std::rethrow_exception(ex);
							}
							catch (const std::invalid_argument&) {}
							catch (...) {
								Assert::Fail(L"Wrong exception type being rethrown");
							}
						}
					}
				}
			}

			std::function<void(size_t)> callback()
			{
				return std::function<void(size_t)>([&](size_t){
					_Exception_count++;
					throw std::invalid_argument("test");
				});
			}
		private:
			std::atomic<int> _Exception_count;
		};

		TEST_METHOD(PassLambdaForEach)
		{
			std::vector<size_t> _Ct(10);

			for_each(par, std::begin(_Ct), std::end(_Ct), [](size_t& _Val) {
				_Val = 10;
			});

			for_each(std::begin(_Ct), std::end(_Ct), [](size_t _Val){
				Assert::AreEqual(_Val, size_t{ 10 });
			});
		}

		TEST_METHOD(ForEachNSeq)
		{
			std::vector<size_t> _Ct(10);

			auto _Iter = for_each_n(std::begin(_Ct), _Ct.size(), [](size_t& _Val) {
				_Val = 10;
			});

			// Verify the values with for_each from <algorithm>
			for_each(std::begin(_Ct), std::end(_Ct), [](size_t _Val){
				Assert::AreEqual(_Val, size_t{ 10 });
			});

			Assert::IsTrue(_Iter == std::end(_Ct));
		}

		TEST_METHOD(ForEachImpl)
		{
			{ // auto_partitioner_tag
				ForEachAlgoTest<random_access_iterator_tag> _Alg;
				_Alg.set_result(for_each_impl<details::auto_partitioner_tag>(_Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
			}

			{ // static_partitioner_tag
				ForEachAlgoTest<random_access_iterator_tag> _Alg;
				_Alg.set_result(for_each_impl<details::static_partitioner_tag>(_Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
			}

			// The implementation of dynamic partitioner is broken thus commented out
			//{ // dynamic_partitioner_tag
			//	ForEachAlgoTest<random_access_iterator_tag> _Alg;
			//	_Alg.set_result(for_each_impl<details::dynamic_partitioner_tag>(_Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
			//}

			{ // auto_partitioner_tag
				ForEachExAlgoTest<random_access_iterator_tag> _Alg;
				_Alg.Catch([&](){
					_Alg.set_result(for_each_impl<details::auto_partitioner_tag>(_Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
				});				
			}

			{ // static_partitioner_tag
				ForEachExAlgoTest<random_access_iterator_tag> _Alg;
				_Alg.Catch([&](){
					_Alg.set_result(for_each_impl<details::static_partitioner_tag>(_Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
				});
			}

			// The implementation of dynamic partitioner is broken thus commented out	
			//{ // dynamic_partitioner_tag
			//	ForEachExAlgoTest<random_access_iterator_tag> _Alg;
			//	_Alg.Catch([&](){
			//		_Alg.set_result(for_each_impl<details::dynamic_partitioner_tag>(_Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
			//	});
			//}
		}

		template<typename _IterCat>
		void RunForEach()
		{
			{  // seq
				ForEachAlgoTest<_IterCat> _Alg(false);
				for_each(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback());
			}

			{  //par
				ForEachAlgoTest<_IterCat>  _Alg(false);
				for_each(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback());
			}

			{  //par_vec
				ForEachAlgoTest<_IterCat> _Alg(false);
				for_each(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback());
			}
		}

		TEST_METHOD(ForEach)
		{
			RunForEach<random_access_iterator_tag>();
			RunForEach<forward_iterator_tag>();
			RunForEach<input_iterator_tag>();
		}

		template<typename _IterCat>
		void RunForEachN()
		{
			{  // seq
				ForEachAlgoTest<_IterCat> _Alg;
				_Alg.set_result(for_each_n(seq, _Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
			}

			{  //par
				ForEachAlgoTest<_IterCat> _Alg;
				_Alg.set_result(for_each_n(par, _Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
			}

			{  //par_vec
				ForEachAlgoTest<_IterCat> _Alg;
				_Alg.set_result(for_each_n(par_vec, _Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(ForEachN)
		{
			RunForEachN<random_access_iterator_tag>();
			RunForEachN<forward_iterator_tag>();
			RunForEachN<input_iterator_tag>();		
		}

		template<typename _IterCat>
		void RunForEachThrow()
		{
			{  // seq
				ForEachExAlgoTest<_IterCat> _Alg;
				_Alg.Catch([&](){
					_Alg.set_result(for_each_n(seq, _Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
				});
			}

			{  //par
				ForEachExAlgoTest<_IterCat> _Alg;
				_Alg.Catch([&](){
					_Alg.set_result(for_each_n(par, _Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
				});
			}

			/* std::par_vec doesn't catch exceptions thus it can lead to undefined behaviour
			{  //par_vec
				ForEachExAlgoTest<_IterCat> _Alg;
				_Alg.Catch([&](){
					_Alg.set_result(for_each_n(par_vec, _Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
				});
			}
			*/
		}

		TEST_METHOD(ForEachThrow)
		{
			RunForEachThrow<random_access_iterator_tag>();
			RunForEachThrow<forward_iterator_tag>();
			RunForEachThrow<input_iterator_tag>();		
		}

		TEST_METHOD(ForEachPerfTest)
		{
			Logger::WriteMessage("-----------Begin performance tests for foreach----------");
			using namespace std::experimental::D4087;
			auto nestSerial = [] (int n)
			{
				bounds<1> bnd{ n };
				int sum = 0;
				// Using serial implementation
				for_each(std::begin(bnd), std::end(bnd), [&](index<1> idx) {
					for_each(std::begin(bnd), std::end(bnd), [&](index<1> idx2) {
						sum = (sum + idx[0] * idx2[0]) % 1000000009;
					});
				});
				return sum;
			};

			auto nestParallel = [] (int n)
			{
				bounds<1> bnd{ n };
				int sum = 0;
				// Using serial implementation
				for_each(par, std::begin(bnd), std::end(bnd), [&](index<1> idx) {
					for_each(par, std::begin(bnd), std::end(bnd), [&](index<1> idx2) {
						sum = (sum + idx[0] * idx2[0]) % 1000000009;
					});
				});
				return sum;
			};

			auto nestVec = [] (int n)
			{
				bounds<1> bnd{ n };
				int sum = 0;
				// Using serial implementation
				for_each(par, std::begin(bnd), std::end(bnd), [&](index<1> idx) {
					for_each(par_vec, std::begin(bnd), std::end(bnd), [&](index<1> idx2) {
						sum = (sum + idx[0] * idx2[0]) % 1000000009;
					});
				});
				return sum;
			};


			auto unevenNestSerial = [](int n)
			{
				bounds<1> bnd1{ 2 };
				bounds<1> bnd2{ n };
				int sum = 0;
				// Using par_vec implementation
				for_each(par, std::begin(bnd1), std::end(bnd1), [&](index<1> idx) {
					for_each(seq, std::begin(bnd2), std::end(bnd2), [&](index<1> idx2) {
						sum = (sum + idx[0] * idx2[0]) % 1000000009;
					});
				});
				return sum;
			};

			auto unevenNestVec = [] (int n)
			{
				bounds<1> bnd1{ 2 };
				bounds<1> bnd2{ n };
				int sum = 0;
				// Using par_vec implementation
				for_each(par, std::begin(bnd1), std::end(bnd1), [&](index<1> idx) {
					for_each(par_vec, std::begin(bnd2), std::end(bnd2), [&](index<1> idx2) {
						sum = (sum + idx[0] * idx2[0]) % 1000000009;
					});
				});
				return sum;
			};

			auto unevenNestParallel = [] (int n)
			{
				bounds<1> bnd1{ 2 };
				bounds<1> bnd2{ n };
				int sum = 0;
				// Using serial implementation
				for_each(par, std::begin(bnd1), std::end(bnd1), [&](index<1> idx) {
					for_each(par, std::begin(bnd2), std::end(bnd2), [&](index<1> idx2) {
						sum = (sum + idx[0] * idx2[0]) % 1000000009;
					});
				});
				return sum;
			};
			measure_time([&] { nestSerial(10000); }, "nested serial loop");
			measure_time([&] { nestParallel(10000); }, "nested parallel loop");
			measure_time([&] { nestVec(10000); }, "nested vectorized loops");

			measure_time([&] { unevenNestSerial(100000000); }, "uneven nested serial loops");
			measure_time([&] { unevenNestParallel(100000000); }, "uneven nested parallel loops");
			measure_time([&] { unevenNestVec(100000000); }, "uneven nested vectorized loops");
			Logger::WriteMessage("-----------End performance tests----------");
		}
	}; //  TEST_CLASS(foreach_tests)
}
