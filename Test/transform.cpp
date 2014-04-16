#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(TransformTest)
	{
		template<typename _IterCat, typename _IterCatOut = _IterCat>
		struct TransformAlgoTest :
			public AlgoTest<TransformAlgoTest<_IterCat, _IterCatOut>, _IterCat, size_t, void, _IterCatOut>
		{
			TransformAlgoTest(bool _Params = false) : _Two_params_pred(_Params)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), rand());
				_Ct2.resize(_Ct.size());
				std::iota(std::begin(_Ct2), std::end(_Ct2), rand());
				_Ct_copy.resize(_Ct.size());
			}

			std::function<size_t(size_t)> binary()
			{
				return std::function<size_t(size_t)>([](size_t _Val){
					return _Val + 1;
				});
			}

			std::function<size_t(size_t, size_t)> binary_op()
			{
				return std::function<size_t(size_t, size_t)>([](size_t _Val, size_t _Val2){
					return _Val + _Val2;
				});
			}

			~TransformAlgoTest()
			{
				auto _Res = std::begin(_Ct_copy);

				if (_Two_params_pred) {
					auto _It_ct2 = std::begin(_Ct2);

					for (auto _It = std::begin(_Ct); _It != std::end(_Ct); ++_It, ++_It_ct2, ++_Res) {
						if ((*_It + *_It_ct2) != *_Res) {
							Assert::Fail();
							break;
						}
					}
				}
				else {
					for (auto _It = std::begin(_Ct); _It != std::end(_Ct); ++_It, ++_Res) {
						if ((*_It + 1) != *_Res) {
							Assert::Fail();
							break;
						}
					}
				}
			}

			in_iterator begin_in2()
			{
				return std::begin(_Ct2);
			}

		private:
			std::vector<size_t> _Ct2;
			bool _Two_params_pred;
		};

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunTransform()
		{
			{  // seq
				TransformAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(transform(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.binary()));
			}

			{  //par
				TransformAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(transform(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.binary()));
			}

			{  //vec
				TransformAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(transform(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.binary()));
			}
		}

		TEST_METHOD(Transform)
		{
			RunTransform<random_access_iterator_tag>();
			RunTransform<forward_iterator_tag>();
			RunTransform<input_iterator_tag, output_iterator_tag>();		
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunTransformTwoParamsPred()
		{
			{  // seq
				TransformAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(transform(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_in2(), _Alg.begin_dest(), _Alg.binary_op()));
			}

			{  //par
				TransformAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(transform(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_in2(), _Alg.begin_dest(), _Alg.binary_op()));
			}

			{  //vec
				TransformAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(transform(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_in2(), _Alg.begin_dest(), _Alg.binary_op()));
			}
		}

		TEST_METHOD(TransformTwoParamsPred)
		{
			RunTransformTwoParamsPred<random_access_iterator_tag>();
			RunTransformTwoParamsPred<forward_iterator_tag>();
			RunTransformTwoParamsPred<input_iterator_tag, output_iterator_tag>();
		}
	};
} // ParallelSTL_Tests
