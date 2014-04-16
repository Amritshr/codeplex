#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(ReduceTest)
	{
		const static size_t INITIAL_VALUE = 100;
		const static size_t MARKER_VALUE = 1;

		template<typename _IterCat>
		struct ReduceAlgoTest :
			public AlgoTest<ReduceAlgoTest<_IterCat>, _IterCat, size_t, size_t>
		{

			ReduceAlgoTest(size_t _Init = 0, bool _Callback = false) : _CallbackEnabled(_Callback), _InitValue(_Init)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), rand());
			}

			~ReduceAlgoTest()
			{
				if (_CallbackEnabled)
					Assert::AreEqual(std::accumulate(std::begin(_Ct), std::end(_Ct), _InitValue, callback()), _Result);
				else Assert::AreEqual(std::accumulate(std::begin(_Ct), std::end(_Ct), _InitValue), _Result);
			}

			std::function<size_t(size_t, size_t)> callback()
			{
				return std::function<size_t(size_t, size_t)>([](size_t _Val, size_t _Val2){
					return _Val + _Val2 + MARKER_VALUE;
				});
			}

		private:
			bool _CallbackEnabled;
			size_t _InitValue;
		};

		template<typename _IterCat>
		void RunReduce()
		{
			{  // seq
				ReduceAlgoTest<_IterCat> _Alg;
				_Alg.set_result(reduce(seq, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par
				ReduceAlgoTest<_IterCat> _Alg;
				_Alg.set_result(reduce(par, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //vec
				ReduceAlgoTest<_IterCat> _Alg;
				_Alg.set_result(reduce(vec, _Alg.begin_in(), _Alg.end_in()));
			}
		}

		TEST_METHOD(Reduce)
		{
			RunReduce<random_access_iterator_tag>();
			RunReduce<forward_iterator_tag>();
			RunReduce<input_iterator_tag>();
		}

		template<typename _IterCat>
		void RunReduceWithInit()
		{
			{  // seq
				ReduceAlgoTest<_IterCat> _Alg(INITIAL_VALUE);
				_Alg.set_result(reduce(seq, _Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE));
			}

			{  //par
				ReduceAlgoTest<_IterCat> _Alg(INITIAL_VALUE);
				_Alg.set_result(reduce(par, _Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE));
			}

			{  //vec
				ReduceAlgoTest<_IterCat> _Alg(INITIAL_VALUE);
				_Alg.set_result(reduce(vec, _Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE));
			}
		}

		TEST_METHOD(ReduceWithInit)
		{
			RunReduceWithInit<random_access_iterator_tag>();
			RunReduceWithInit<forward_iterator_tag>();
			RunReduceWithInit<input_iterator_tag>();
		}

		template<typename _IterCat>
		void RunReduceWithCallback()
		{
			{  // seq
				ReduceAlgoTest<_IterCat> _Alg(INITIAL_VALUE, true);
				_Alg.set_result(reduce(seq, _Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE, _Alg.callback()));
			}

			{  //par
				ReduceAlgoTest<_IterCat> _Alg(INITIAL_VALUE, true);
				_Alg.set_result(reduce(par, _Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE, _Alg.callback()));
			}

			{  //vec
				ReduceAlgoTest<_IterCat> _Alg(INITIAL_VALUE, true);
				_Alg.set_result(reduce(vec, _Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE, _Alg.callback()));
			}
		}

		TEST_METHOD(ReduceWithCallback)
		{
			RunReduceWithCallback<random_access_iterator_tag>();
			RunReduceWithCallback<forward_iterator_tag>();
			RunReduceWithCallback<input_iterator_tag>();
		}

		struct Element {
			int _Val;
			explicit Element(int _V) : _Val(_V) {}

			Element operator+(const Element& rhs)
			{
				return Element(_Val + rhs._Val);
			}
		};

		TEST_METHOD(ReduceEdgeCases)
		{
			std::vector<int> vec_empty;

			std::vector<Element> vec_el;
			vec_el.emplace_back(Element(1));

			std::vector<Element> vec_el2;
			vec_el2.emplace_back(Element(1));
			vec_el2.emplace_back(Element(1));
			vec_el2.emplace_back(Element(1));
			vec_el2.emplace_back(Element(1));
			vec_el2.emplace_back(Element(1));

			std::vector<int> vec_one = { 1 };
			std::vector<int> vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

			auto _Val = reduce(par, std::begin(vec_empty), std::end(vec_empty), 0);
			Assert::AreEqual(_Val, int{ 0 });

			Element _El = reduce(par, std::begin(vec_el), std::end(vec_el), Element(0), [](Element _Val, Element _Val2) -> Element { return Element(_Val._Val + _Val2._Val); });
			Assert::AreEqual(_El._Val, int{ 1 });

			_El = reduce(par, std::begin(vec_el), std::end(vec_el), Element(0));
			Assert::AreEqual(_El._Val, int{ 1 });

			_El = reduce(par, std::begin(vec_el2), std::end(vec_el2), Element(0), std::plus<>());
			Assert::AreEqual(_El._Val, int{ 5 });

			_Val = reduce(par, std::begin(vec_one), std::end(vec_one), 0, std::plus<>());
			Assert::AreEqual(_Val, int{ 1 });

			_Val = reduce(par, std::begin(vec_one), std::end(vec_one));
			Assert::AreEqual(_Val, int{ 1 });

			_Val = reduce(par, std::begin(vec), std::end(vec));
			Assert::AreEqual(_Val, int{ 55 });
		}

		TEST_METHOD(ReduceSequential)
		{
			{
				ReduceAlgoTest<random_access_iterator_tag> _Alg;
				_Alg.set_result(reduce(_Alg.begin_in(), _Alg.end_in()));
			}

			{
				ReduceAlgoTest<forward_iterator_tag> _Alg;
				_Alg.set_result(reduce(_Alg.begin_in(), _Alg.end_in()));
			}

			{
				ReduceAlgoTest<input_iterator_tag> _Alg;
				_Alg.set_result(reduce(_Alg.begin_in(), _Alg.end_in()));
			}
		}

		TEST_METHOD(ReduceWithInitSequential)
		{
			{
				ReduceAlgoTest<random_access_iterator_tag> _Alg(INITIAL_VALUE);
				_Alg.set_result(reduce(_Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE));
			}

			{
				ReduceAlgoTest<forward_iterator_tag> _Alg(INITIAL_VALUE);
				_Alg.set_result(reduce(_Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE));
			}

			{
				ReduceAlgoTest<input_iterator_tag> _Alg(INITIAL_VALUE);
				_Alg.set_result(reduce(_Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE));
			}
		}

		TEST_METHOD(ReduceWithCallbackSequential)
		{
			{
				ReduceAlgoTest<random_access_iterator_tag> _Alg(INITIAL_VALUE, true);
				_Alg.set_result(reduce(_Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE, _Alg.callback()));
			}

			{
				ReduceAlgoTest<forward_iterator_tag> _Alg(INITIAL_VALUE, true);
				_Alg.set_result(reduce(_Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE, _Alg.callback()));
			}

			{
				ReduceAlgoTest<input_iterator_tag> _Alg(INITIAL_VALUE, true);
				_Alg.set_result(reduce(_Alg.begin_in(), _Alg.end_in(), INITIAL_VALUE, _Alg.callback()));
			}
		}
	};
} // namespace ParallelSTL_Tests
