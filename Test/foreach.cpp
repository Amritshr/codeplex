#include "stdafx.h"

namespace ParallelSTL_Tests
{
	// Test for internal implementation of partitioners bypassing the algorithm interface
	template<class _ExPolicy, class _InIt, class _Diff, class _Fn1>
	inline _InIt for_each_impl(_InIt _First, _Diff _Count, _Fn1 _Func)
	{
		if (_Count > 0) {
			return details::_Partitioner<_ExPolicy>::_For_Each(_First, _Count, [_Func](_InIt _Begin, size_t _Count){

				details::LoopHelper<_ExPolicy, _InIt>::Loop(_Begin, _Count, [&_Func](const _InIt::reference _It){
					_Func(_It);
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

			{  //vec
				ForEachAlgoTest<_IterCat> _Alg(false);
				for_each(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback());
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

			{  //vec
				ForEachAlgoTest<_IterCat> _Alg;
				_Alg.set_result(for_each_n(vec, _Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
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

			{  //vec
				ForEachExAlgoTest<_IterCat> _Alg;
				_Alg.Catch([&](){
					_Alg.set_result(for_each_n(vec, _Alg.begin_in(), _Alg.size_in(), _Alg.callback()));
				});
			}
		}

		TEST_METHOD(ForEachThrow)
		{
			RunForEachThrow<random_access_iterator_tag>();
			RunForEachThrow<forward_iterator_tag>();
			RunForEachThrow<input_iterator_tag>();		
		}
	}; //  TEST_CLASS(foreach_tests)
}
