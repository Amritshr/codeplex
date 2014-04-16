#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(GenerateTest)
	{
		const static size_t FILL_VALUE = 10;

		template<typename _IterCat>
		struct GenerateAlgoTest :
			public AlgoTest<GenerateAlgoTest<_IterCat>, _IterCat, size_t>
		{
			GenerateAlgoTest()
			{
				std::fill(std::begin(_Ct), std::end(_Ct), 0);
			}

			~GenerateAlgoTest()
			{
				Assert::IsTrue(std::find_if_not(std::begin(_Ct), std::end(_Ct), [&](size_t _Val){
					return _Val == FILL_VALUE;
				}) == std::end(_Ct));
			}

			std::function<size_t(void)> callback()
			{
				return std::function<size_t(void)>([&](){
					return FILL_VALUE;
				});
			}
		};

		template<typename _IterCat>
		void RunGenerate()
		{
			{  // seq
				GenerateAlgoTest<_IterCat> _Alg;
				generate(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback());
			}

			{  //par
				GenerateAlgoTest<_IterCat> _Alg;
				generate(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback());
			}

			{  //vec
				GenerateAlgoTest<_IterCat> _Alg;
				generate(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback());
			}
		}

		TEST_METHOD(Generate)
		{
			RunGenerate<random_access_iterator_tag>();
			RunGenerate<forward_iterator_tag>();
		}

		template<typename _IterCat>
		void RunGenerateN()
		{
			{  // seq
				GenerateAlgoTest<_IterCat> _Alg;
				generate_n(seq, _Alg.begin_in(), _Alg.size_in(), _Alg.callback());
			}

			{  //par
				GenerateAlgoTest<_IterCat> _Alg;
				generate_n(par, _Alg.begin_in(), _Alg.size_in(), _Alg.callback());
			}

			{  //vec
				GenerateAlgoTest<_IterCat> _Alg;
				generate_n(vec, _Alg.begin_in(), _Alg.size_in(), _Alg.callback());
			}
		}

		TEST_METHOD(GenerateN)
		{
			RunGenerateN<random_access_iterator_tag>();
			RunGenerateN<forward_iterator_tag>();
			RunGenerateN<output_iterator_tag>();
		}
	};
} // ParallelSTL_Tests
