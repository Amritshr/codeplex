#include "stdafx.h"
namespace ParallelSTL_Tests
{
	TEST_CLASS(NthElementTest)
	{
		struct NthElementAlgoTest :
			public AlgoTest<NthElementAlgoTest, std::random_access_iterator_tag, size_t>
		{
			size_t _Nth_pos;
			container::iterator _Nth;
			bool _Callback;

			NthElementAlgoTest(bool _C = false) : _Callback(_C)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), 0);
				std::random_shuffle(std::begin(_Ct), std::end(_Ct));

				_Nth_pos = rand() % _Ct.size();
				_Nth = std::begin(_Ct);
				std::advance(_Nth, _Nth_pos);
			}

			~NthElementAlgoTest()
			{
				std::function<bool(size_t, size_t)> _Pred = callback();
				if (!_Callback) {
					_Pred = std::function<bool(size_t, size_t)>([](size_t _Val, size_t _Val2) {
						return _Val < _Val2;
					});

					Assert::AreEqual(_Nth_pos, *_Nth);
				}

				Assert::IsTrue(std::all_of(std::begin(_Ct), _Nth, [&](size_t _Val){
					return _Pred(_Val, *this->_Nth) || !_Pred(*this->_Nth, _Val);
				}));

				Assert::IsTrue(std::all_of(_Nth, std::end(_Ct), [&](size_t _Val){
					return _Pred(*this->_Nth, _Val) || !_Pred(_Val, *this->_Nth);
				}));
			}

			in_iterator nth_in()
			{				
				return make_test_iterator<in_iterator::iterator_category>(_Nth);
			}

			std::function<bool(size_t, size_t)> callback() {
				return std::function<bool(size_t, size_t)>([](size_t _Val, size_t _Val2){
					return (_Val & 0xfff) < (_Val2 & 0xfff);
				});
			}
		};

		TEST_METHOD(NthElement)
		{
			{  // seq
				NthElementAlgoTest _Alg;
				nth_element(seq, _Alg.begin_in(), _Alg.nth_in(), _Alg.end_in());
			}

			{  //par
				NthElementAlgoTest _Alg;
				nth_element(par, _Alg.begin_in(), _Alg.nth_in(), _Alg.end_in());
			}

			{  //vec
				NthElementAlgoTest _Alg;
				nth_element(vec, _Alg.begin_in(), _Alg.nth_in(), _Alg.end_in());
			}
		}

		TEST_METHOD(NthElementCallback)
		{
			{  // seq
				NthElementAlgoTest _Alg(true);
				nth_element(seq, _Alg.begin_in(), _Alg.nth_in(), _Alg.end_in(), _Alg.callback());
			}

			{  //par
				NthElementAlgoTest _Alg(true);
				nth_element(par, _Alg.begin_in(), _Alg.nth_in(), _Alg.end_in(), _Alg.callback());
			}

			{  //vec
				NthElementAlgoTest _Alg(true);
				nth_element(vec, _Alg.begin_in(), _Alg.nth_in(), _Alg.end_in(), _Alg.callback());
			}
		}

		TEST_METHOD(NthElementEdgeCases)
		{
			std::vector<int> vec_empty;
			nth_element(std::begin(vec_empty), std::begin(vec_empty), std::end(vec_empty));

			std::vector<int> vec_one = { 1 };
			nth_element(std::begin(vec_one), std::begin(vec_one), std::end(vec_one));
			Assert::AreEqual(1, vec_one[0]);

			std::vector<int> vec = { 2, 5, 1, 3, 7, 9, 8, 1, 7, 3, 12 };

			std::vector<int> vec_tmp(vec.size());

			std::copy(std::begin(vec), std::end(vec), std::begin(vec_tmp)); // Begin
			nth_element(std::begin(vec_tmp), std::begin(vec_tmp), std::end(vec_tmp));
			Assert::AreEqual(1, vec_tmp[0]);
			Assert::IsTrue(std::all_of(std::begin(vec_tmp), std::end(vec_tmp), [](int _Val){
				return 1 <= _Val;
			}));

			std::copy(std::begin(vec), std::end(vec), std::begin(vec_tmp)); // Last
			nth_element(std::begin(vec_tmp), std::end(vec_tmp), std::end(vec_tmp));
			Assert::IsTrue(std::all_of(std::begin(vec_tmp), std::end(vec_tmp), [](int _Val){
				return 12 >= _Val;
			}));

			std::copy(std::begin(vec), std::end(vec), std::begin(vec_tmp)); // Last - 1
			nth_element(std::begin(vec_tmp), std::begin(vec_tmp), std::end(vec_tmp));
			Assert::IsTrue(std::all_of(std::begin(vec_tmp), std::end(vec_tmp), [](int _Val){
				return 12 >= _Val;
			}));
		}
	};
}

