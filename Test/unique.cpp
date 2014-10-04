#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(UniqueTest)
	{
		template<typename _IterCat, typename _OutIterCat = _IterCat>
		struct UniqueAlgoTest :
			public AlgoTest<UniqueAlgoTest<_IterCat, _OutIterCat>, _IterCat, size_t, void, _OutIterCat>
		{
		private:
			bool _Copy;
		public:
			UniqueAlgoTest(bool _C = true) : _Copy(_C)
			{
				size_t _Start = 0;
				int _Update = false;

				for_each(std::begin(_Ct), std::end(_Ct), [&_Start, &_Update](size_t& _Val) {
					_Val = _Start;
					if (_Update)
						++_Start;

					_Update = !_Update;
				});

				_Ct_copy.resize(_Ct.size());
				std::copy(std::begin(_Ct), std::end(_Ct), std::begin(_Ct_copy));
			}

			~UniqueAlgoTest()
			{
				if (_Copy) {
					_Ct_copy.resize(std::distance(begin_dest().get_inner(), _Result.get_inner()));
					_Ct.resize(static_cast<size_t>(std::distance(std::begin(_Ct), unique(std::begin(_Ct), std::end(_Ct)))));
				}
				else {
					_Ct_copy.resize(static_cast<size_t>(std::distance(std::begin(_Ct_copy), unique(std::begin(_Ct_copy), std::end(_Ct_copy)))));
					_Ct.resize(std::distance(std::begin(_Ct), _Result.get_inner()));
				}

				Assert::IsTrue(_Ct == _Ct_copy);
				Assert::AreEqual(_Ct.size(), _Ct_copy.size());
			}

			std::function<bool(size_t, size_t)> callback() {
				return std::function<bool(size_t, size_t)>([](size_t _Val, size_t _Val2){
					return _Val == _Val2;
				});
			}
		};

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunUniqueCopy()
		{
			{  // seq
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(unique_copy(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}

			{  //par
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(unique_copy(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}

			{  //par_vec
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(unique_copy(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}
		}

		TEST_METHOD(UniqueCopy)
		{
			RunUniqueCopy<random_access_iterator_tag>();
			RunUniqueCopy<forward_iterator_tag>();
			RunUniqueCopy<input_iterator_tag, output_iterator_tag>();
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunUniqueCopyCallback()
		{
			{  // seq
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(unique_copy(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}

			{  //par
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(unique_copy(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}

			{  //par_vec
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(unique_copy(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}
		}

		TEST_METHOD(UniqueCopyCallback)
		{
			RunUniqueCopyCallback<random_access_iterator_tag>();
			RunUniqueCopyCallback<forward_iterator_tag>();
			RunUniqueCopyCallback<input_iterator_tag, output_iterator_tag>();
		}

		TEST_METHOD(UniqueCopyEdgeCases)
		{
			std::vector<int> vec_empty, dest_empty;
			Assert::IsTrue(unique_copy(par, std::begin(vec_empty), std::end(vec_empty), std::begin(dest_empty)) == std::begin(dest_empty));

			std::vector<int> vec_one = { 1 }, dest_one = { 2 };
			Assert::IsTrue(unique_copy(par, std::begin(vec_one), std::end(vec_one), std::begin(dest_one)) != std::begin(dest_one));
			Assert::AreEqual(dest_one[0], 1);

			std::vector<int> vec = { 1, 3, 3, 10, 10, 7, 6, 7, 7, 12, 13, 24, 24, 24, 24 };
			std::vector<int> vec_dest(vec.size());

			auto _Iter = unique_copy(par, std::begin(vec), std::end(vec), std::begin(vec_dest));

			Assert::AreEqual(static_cast<size_t>(std::distance(std::begin(vec_dest), _Iter)), size_t{ 9 });
			vec_dest.resize(std::distance(std::begin(vec_dest), _Iter));
			
			vec.resize(static_cast<size_t>(std::distance(std::begin(vec), unique(std::begin(vec), std::end(vec)))));
			Assert::IsTrue(vec_dest == vec);
			Assert::AreEqual(vec_dest.size(), vec.size());
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunUnique()
		{
			{  // seq
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(unique(seq, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(unique(par, _Alg.begin_in(), _Alg.end_in()));
			}

			{  //par_vec
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(unique(par_vec, _Alg.begin_in(), _Alg.end_in()));
			}
		}

		TEST_METHOD(Unique)
		{
			RunUnique<random_access_iterator_tag>();
			RunUnique<forward_iterator_tag>();
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunUniqueCallback()
		{
			{  // seq
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(unique(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(unique(par, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}

			{  //par_vec
				UniqueAlgoTest<_IterCat, _IterCat2> _Alg(false);
				_Alg.set_result(unique(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.callback()));
			}
		}

		TEST_METHOD(UniqueCallback)
		{
			RunUniqueCallback<random_access_iterator_tag>();
			RunUniqueCallback<forward_iterator_tag>();
		}

		TEST_METHOD(UniqueEdgeCases)
		{
			std::vector<int> vec_empty;
			Assert::IsTrue(unique(par, std::begin(vec_empty), std::end(vec_empty)) == std::begin(vec_empty));

			std::vector<int> vec_one = { 1 };
			Assert::IsTrue(unique(par, std::begin(vec_one), std::end(vec_one)) != std::begin(vec_one));
			Assert::AreEqual(vec_one[0], 1);

			std::vector<int> vec = { 1, 3, 3, 10, 10, 7, 6, 7, 7, 12, 13, 24, 24, 24, 24 };
			
			auto _Iter = unique(par, std::begin(vec), std::end(vec));

			Assert::AreEqual(static_cast<size_t>(std::distance(std::begin(vec), _Iter)), size_t{ 9 });
		}
	};
}
