#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(RotateCopyTest)
	{
		template<typename _IterCat, typename _IterCatOut = _IterCat>
		struct RotateCopyAlgoTest :
			public AlgoTest<RotateCopyAlgoTest<_IterCat, _IterCatOut>, _IterCat, size_t, void, _IterCatOut>
		{
			size_t _Mid_pos;
			bool _Copy;
		public:
			RotateCopyAlgoTest(bool _RotateEnd, bool _IsCopy) : _Copy(_IsCopy)
			{
				std::iota(std::begin(_Ct), std::end(_Ct), rand());
				_Ct_copy.resize(_Ct.size());

				if (_RotateEnd)
					_Mid_pos = _Ct.size();
				else _Mid_pos = rand() % _Ct.size();

				if (!_Copy) 
					std::copy(std::begin(_Ct), std::end(_Ct), std::begin(_Ct_copy));
			}

			in_iterator mid_in()
			{
				auto _Mid_iter = std::begin(_Ct);
				if (_Mid_pos == _Ct.size())
					return std::end(_Ct);

				std::advance(_Mid_iter, _Mid_pos);
				return _Mid_iter;
			}

			~RotateCopyAlgoTest()
			{
				if (_Copy) {
					container _ExpectedResult(_Ct.size());
					std::rotate_copy(begin_in(), mid_in(), end_in(), std::begin(_ExpectedResult));

					Assert::IsTrue(_ExpectedResult == _Ct_copy);
				}
				else {
					auto _Mid = std::begin(_Ct_copy);
					std::advance(_Mid, _Mid_pos);

					std::rotate(std::begin(_Ct_copy), _Mid, std::end(_Ct_copy));			
					Assert::IsTrue(_Ct_copy == _Ct);
				}
			}
		};

		TEST_METHOD(RotateCopyEdgeCases)
		{
			std::vector<int> empty, empty_dest;
			Assert::IsTrue(rotate_copy(par, std::begin(empty), std::end(empty), std::end(empty), std::begin(empty_dest)) == std::end(empty_dest));

			std::vector<int> vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			std::vector<int> dest = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };

			Assert::IsTrue(rotate_copy(par, std::begin(vec), std::end(vec), std::end(vec), std::begin(dest)) == std::end(dest));
			Assert::IsTrue(vec == dest);

			std::vector<int> vec2 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			std::vector<int> dest2 = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };

			Assert::IsTrue(rotate_copy(par, std::begin(vec2), std::begin(vec2), std::end(vec2), std::begin(dest2)) == std::end(dest2));
			Assert::IsTrue(vec2 == dest2);

			// Rotate for std::end() - 1
			std::vector<int> vec3 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			std::vector<int> dest3 = { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20 };
			std::vector<int>::iterator _Mid = std::begin(vec3);
			std::advance(_Mid, vec3.size() / 2);
			std::vector<int> _Expected(vec3.size());

			auto _Iter = rotate_copy(par, std::begin(vec3), _Mid, std::end(vec3), std::begin(dest3));			
			auto _IterExpected = std::rotate_copy(std::begin(vec3), _Mid, std::end(vec3), std::begin(_Expected));

			Assert::IsTrue(dest3 == _Expected);
			Assert::IsTrue(std::distance(std::begin(dest3), _Iter) == std::distance(std::begin(_Expected), _IterExpected));
		}

		template<typename _IterCat>
		void RunRotateCopy(bool _RotateEnd)
		{
			{  // seq
				RotateCopyAlgoTest<_IterCat> _Alg(_RotateEnd, true);
				rotate_copy(seq, _Alg.begin_in(), _Alg.mid_in(), _Alg.end_in(), _Alg.begin_dest());
			}

			{  //par
				RotateCopyAlgoTest<_IterCat> _Alg(_RotateEnd, true);
				rotate_copy(par, _Alg.begin_in(), _Alg.mid_in(), _Alg.end_in(), _Alg.begin_dest());
			}

			{  //par_vec
				RotateCopyAlgoTest<_IterCat> _Alg(_RotateEnd, true);
				rotate_copy(par_vec, _Alg.begin_in(), _Alg.mid_in(), _Alg.end_in(), _Alg.begin_dest());
			}
		}

		TEST_METHOD(RotateCopy)
		{
			RunRotateCopy<random_access_iterator_tag>(false);		
			RunRotateCopy<forward_iterator_tag>(false);

			RunRotateCopy<random_access_iterator_tag>(true);
			RunRotateCopy<forward_iterator_tag>(true);
		}

		TEST_METHOD(RotateEdgeCases)
		{
			std::vector<int> empty;
			Assert::IsTrue(rotate(par, std::begin(empty), std::end(empty), std::end(empty)) == std::end(empty));

			std::vector<int> vec_expeted_results = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

			std::vector<int> vec = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			Assert::IsTrue(rotate(par, std::begin(vec), std::end(vec), std::end(vec)) == std::begin(vec));
			Assert::IsTrue(vec_expeted_results == vec);

			std::vector<int> vec2 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			Assert::IsTrue(rotate(par, std::begin(vec2), std::begin(vec2), std::end(vec2)) == std::end(vec2));
			Assert::IsTrue(vec_expeted_results == vec2);

			// Rotate for std::end() - 1
			std::vector<int> vec3 = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			std::vector<int>::iterator _Mid = std::begin(vec3);
			std::advance(_Mid, vec3.size() / 2);

			Assert::IsTrue(rotate(par, std::begin(vec3), _Mid, std::end(vec3)) == _Mid);
		}

		template<typename _IterCat>
		void RunRotate(bool _RotateEnd)
		{
			{  // seq
				RotateCopyAlgoTest<_IterCat> _Alg(_RotateEnd, false);
				rotate(seq, _Alg.begin_in(), _Alg.mid_in(), _Alg.end_in());
			} 
			
			{  //par
				RotateCopyAlgoTest<_IterCat> _Alg(_RotateEnd, false);
				rotate(par, _Alg.begin_in(), _Alg.mid_in(), _Alg.end_in());
			}

			{  //par_vec
				RotateCopyAlgoTest<_IterCat> _Alg(_RotateEnd, false);
				rotate(par_vec, _Alg.begin_in(), _Alg.mid_in(), _Alg.end_in());
			}
		}

		TEST_METHOD(Rotate)
		{
			RunRotate<random_access_iterator_tag>(false);
			RunRotate<forward_iterator_tag>(false);

			RunRotate<random_access_iterator_tag>(true);
			RunRotate<forward_iterator_tag>(true);
		}
	};
} // ParallelSTL_Tests
