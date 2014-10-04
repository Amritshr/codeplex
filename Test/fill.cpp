#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(FillTest)
	{
		const static size_t FILL_VALUE = 10;

		template<typename _IterCat>
		struct FillAlgoTest :
			public AlgoTest<FillAlgoTest<_IterCat>, _IterCat, size_t>
		{
			bool _HasResult;

			FillAlgoTest(bool _Res = false) : _HasResult(_Res)
			{
				std::fill(std::begin(_Ct), std::end(_Ct), 0);
			}

			~FillAlgoTest()
			{
				Assert::IsTrue(std::find_if_not(std::begin(_Ct), std::end(_Ct), [&](size_t _Val){
					return _Val == FILL_VALUE;
				}) == std::end(_Ct));

				if (_HasResult)
					Assert::IsTrue(_Result.get_inner() == std::end(_Ct));
			}
		};

		template<typename _IterCat>
		void RunFill()
		{
			{  // seq
				FillAlgoTest<_IterCat> _Alg;
				fill(seq, _Alg.begin_in(), _Alg.end_in(), FILL_VALUE);
			}

			{  //par
				FillAlgoTest<_IterCat> _Alg;
				fill(par, _Alg.begin_in(), _Alg.end_in(), FILL_VALUE);
			}

			{  //par_vec
				FillAlgoTest<_IterCat> _Alg;
				fill(par_vec, _Alg.begin_in(), _Alg.end_in(), FILL_VALUE);
			}
		}

		TEST_METHOD(Fill)
		{
			RunFill<random_access_iterator_tag>();
			RunFill<forward_iterator_tag>();	
		}

		template<typename _IterCat>
		void RunFillN()
		{
			{  // seq
				FillAlgoTest<_IterCat> _Alg(true);
				_Alg.set_result(fill_n(seq, _Alg.begin_in(), _Alg.size_in(), FILL_VALUE));
			}

			{  //par
				FillAlgoTest<_IterCat> _Alg(true);
				_Alg.set_result(fill_n(par, _Alg.begin_in(), _Alg.size_in(), FILL_VALUE));
			}

			{  //par_vec
				FillAlgoTest<_IterCat> _Alg(true);
				_Alg.set_result(fill_n(par_vec, _Alg.begin_in(), _Alg.size_in(), FILL_VALUE));
			}
		}

		TEST_METHOD(FillN)
		{
			RunFillN<random_access_iterator_tag>();
			RunFillN<forward_iterator_tag>();
			RunFillN<output_iterator_tag>();
		}

		template<typename _IterCat>
		void RunUninitializedFill()
		{
			{  // seq
				FillAlgoTest<_IterCat> _Alg;
				uninitialized_fill(seq, _Alg.begin_in(), _Alg.end_in(), FILL_VALUE);
			}

			{  //par
				FillAlgoTest<_IterCat> _Alg;
				uninitialized_fill(par, _Alg.begin_in(), _Alg.end_in(), FILL_VALUE);
			}

			{  //par_vec
				FillAlgoTest<_IterCat> _Alg;
				uninitialized_fill(par_vec, _Alg.begin_in(), _Alg.end_in(), FILL_VALUE);
			}
		}

		TEST_METHOD(UninitializedFill)
		{
			RunUninitializedFill<random_access_iterator_tag>();
			RunUninitializedFill<forward_iterator_tag>();
		}

		TEST_METHOD(UninitializedFillEdgeCases)
		{
			const int _Init_value = 3;
			std::vector<int> vec_empty;
			uninitialized_fill(std::begin(vec_empty), std::end(vec_empty), _Init_value);
			uninitialized_fill_n(std::begin(vec_empty), 0, _Init_value);

			UnintializeHelper::set_no_exception(); // Make sure that no exception will be thrown from copy constructor
			const static int MAX_DATA_SIZE = 32 + (rand() % 1024);
			std::unique_ptr<UnintializeHelper[]> vec(new UnintializeHelper[MAX_DATA_SIZE]);

			{
				UnintializeHelper::reset();

				try {
					uninitialized_fill_n(par, vec.get(), MAX_DATA_SIZE, UnintializeHelper());
				}
				catch (exception_list& e) {
					Assert::IsTrue(e.size() > 0);

					for (auto& _E : e) {
						try {
							std::rethrow_exception(_E);
						}
						catch (CustomException&) {}
						catch (...) {
							Assert::Fail();
						}
					}
				}
				catch (...) {
					Assert::Fail();
				}

				Assert::AreEqual(0, UnintializeHelper::get_count());
			}
		}

		template<typename _IterCat>
		void RunUninitializedFillN()
		{
			{  // seq
				FillAlgoTest<_IterCat> _Alg(true);
				_Alg.set_result(uninitialized_fill_n(seq, _Alg.begin_in(), _Alg.size_in(), FILL_VALUE));
			}

			{  //par
				FillAlgoTest<_IterCat> _Alg(true);
				_Alg.set_result(uninitialized_fill_n(par, _Alg.begin_in(), _Alg.size_in(), FILL_VALUE));
			}

			{  //par_vec
				FillAlgoTest<_IterCat> _Alg(true);
				_Alg.set_result(uninitialized_fill_n(par_vec, _Alg.begin_in(), _Alg.size_in(), FILL_VALUE));
			}
		}

		TEST_METHOD(UninitializedFillN)
		{
			RunUninitializedFillN<random_access_iterator_tag>();
			RunUninitializedFillN<forward_iterator_tag>();		
		}
	};
} // ParallelSTL_Tests
