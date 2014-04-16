#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(CopyTest)
	{
		template<typename _IterCat, typename _OutIterCat = _IterCat>
		struct CopyAlgoTest :
			public AlgoTest<CopyAlgoTest<_IterCat, _OutIterCat>, _IterCat, size_t, void, _OutIterCat>
		{
			CopyAlgoTest()
			{
				std::iota(std::begin(_Ct), std::end(_Ct), 0);
				_Ct_copy.resize(_Ct.size());
			}

			~CopyAlgoTest()
			{
				Assert::IsTrue(_Ct_copy == _Ct);
				Assert::AreEqual(_Ct_copy.size(), _Ct.size());

				size_t _Idx = 0;
				for (auto _It = std::begin(_Ct); _It != std::end(_Ct); ++_It, ++_Idx) {
					if (*_It != _Idx) {
						Assert::Fail();
						break;
					}
				}
			}
		};

		TEST_METHOD(CopyEdgeCases)
		{
			std::vector<int> vec, dest;
			auto _Iter = copy(par, std::begin(vec), std::end(vec), std::begin(dest));
			Assert::IsTrue(_Iter == std::end(dest));

			std::vector<int> vec_one(1, 1), dest_one(1, 2);

			_Iter = copy(par, std::begin(vec_one), std::end(vec_one), std::begin(dest_one));
			Assert::IsTrue(_Iter == std::end(dest_one));
			Assert::AreEqual(dest_one[0], int{ 1 });

			// Compile time check only
			std::vector<MovableOnly> vec_movable, vec_moveable_dest;
			move(par, std::begin(vec_movable), std::end(vec_movable), std::begin(vec_moveable_dest));
			
			std::vector<int> vec_copy = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			std::vector<int>::iterator _End = std::begin(vec_copy);			
			std::advance(_End, 4);

			copy(par, std::begin(vec_copy), _End, _End);

			for (size_t _I = 0; _I < 4; ++_I) {
				vec_copy[0] = *_End;
				++_End;
			}

			// Check the copy counts
			std::vector<CopyableOnly> vec_copyable(10), vec_copyable2(10);
			copy(par, std::begin(vec_copyable), std::end(vec_copyable), std::begin(vec_copyable2));

			std::for_each(std::begin(vec_copyable2), std::end(vec_copyable2), [](CopyableOnly& _Val){
				Assert::AreEqual(_Val._Copy_count, 1); // Only one copy
			});
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunCopy()
		{
			// random_access_iterator_tag
			{  // seq
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(copy(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}

			{  //par
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(copy(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}

			{  //vec
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(copy(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}
		}

		TEST_METHOD(Copy)
		{
			RunCopy<random_access_iterator_tag>();
			RunCopy<forward_iterator_tag>();
			RunCopy<input_iterator_tag, output_iterator_tag>();
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunCopyN()
		{
			// random_access_iterator_tag
			{  // seq
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(copy_n(seq, _Alg.begin_in(), _Alg.size_in(), _Alg.begin_dest()));
			}

			{  //par
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(copy_n(par, _Alg.begin_in(), _Alg.size_in(), _Alg.begin_dest()));
			}

			{  //vec
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(copy_n(vec, _Alg.begin_in(), _Alg.size_in(), _Alg.begin_dest()));
			}
		}

		TEST_METHOD(CopyN)
		{
			RunCopyN<random_access_iterator_tag>();
			RunCopyN<forward_iterator_tag>();
			RunCopyN<input_iterator_tag, output_iterator_tag>();
		}

		template<typename _IterCat>
		void RunUninitializedCopyN()
		{
			{  // seq
				CopyAlgoTest<_IterCat> _Alg;
				_Alg.set_result(uninitialized_copy_n(seq, _Alg.begin_in(), _Alg.size_in(), _Alg.begin_dest()));
			}

			{  //par
				CopyAlgoTest<_IterCat> _Alg;
				_Alg.set_result(uninitialized_copy_n(par, _Alg.begin_in(), _Alg.size_in(), _Alg.begin_dest()));
			}

			{  //vec
				CopyAlgoTest<_IterCat> _Alg;
				_Alg.set_result(uninitialized_copy_n(vec, _Alg.begin_in(), _Alg.size_in(), _Alg.begin_dest()));
			}
		}

		TEST_METHOD(UninitializedCopyN)
		{
			RunUninitializedCopyN<random_access_iterator_tag>();
			RunUninitializedCopyN<forward_iterator_tag>();		
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunUninitializedCopy()
		{
			{  // seq
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(uninitialized_copy(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}

			{  //par
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(uninitialized_copy(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}

			{  //vec
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(uninitialized_copy(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest()));
			}
		}

		TEST_METHOD(UninitializedCopy)
		{
			RunUninitializedCopy<random_access_iterator_tag>();
			RunUninitializedCopy<forward_iterator_tag>();
			RunUninitializedCopy<input_iterator_tag, forward_iterator_tag>();	
		}

		TEST_METHOD(UninitializedCopyEdgeCases)
		{
			std::vector<int> vec_empty;
			uninitialized_copy(std::begin(vec_empty), std::end(vec_empty), std::begin(vec_empty));
			uninitialized_copy_n(std::begin(vec_empty), 0, std::begin(vec_empty));

			const static int MAX_DATA_SIZE = 32 + (rand() % 1024);

			UnintializeHelper::set_no_exception(); // Make sure that no exception will be thrown from copy constructor

			std::unique_ptr<UnintializeHelper[]> vec(new UnintializeHelper[MAX_DATA_SIZE]);
			
			for (int i = 0; i < MAX_DATA_SIZE; ++i)
				vec.get()[i] = UnintializeHelper();
			
			std::unique_ptr<UnintializeHelper[]> dest(new UnintializeHelper[MAX_DATA_SIZE]);

			{
				UnintializeHelper::reset();

				try {
					uninitialized_copy_n(par, vec.get(), MAX_DATA_SIZE, dest.get());
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

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunMove()
		{
			{  // seq
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				move(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest());
			}

			{  //par
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				move(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest());
			}

			{  //vec
				CopyAlgoTest<_IterCat, _IterCat2> _Alg;
				move(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest());
			}
		}

		TEST_METHOD(Move)
		{
			RunMove<random_access_iterator_tag>();
			RunMove<forward_iterator_tag>();
			RunMove<input_iterator_tag, output_iterator_tag>();	
		}
	};

	TEST_CLASS(CopyIfTest)
	{
		template<typename _IterCat, typename _OutIterCat = _IterCat>
		struct CopyIfAlgoTest :
			public AlgoTest<CopyIfAlgoTest<_IterCat, _OutIterCat>, _IterCat, size_t, void, _OutIterCat> //resuls as output iterator
		{
			CopyIfAlgoTest()
			{
				std::iota(std::begin(_Ct), std::end(_Ct), 0);
				_Ct_copy.resize(_Ct.size());
			}

			~CopyIfAlgoTest()
			{
				Assert::AreEqual(static_cast<size_t>(std::distance(std::begin(_Ct_copy), _Result.get_inner())), _Ct.size() / 2);

				_Ct_copy.resize(std::distance(begin_dest().get_inner(), _Result.get_inner()));

				Assert::IsTrue(std::find_if_not(std::begin(_Ct_copy), std::end(_Ct_copy), [](size_t _Val) {
					return _Val % 2 == 1;
				}) == std::end(_Ct_copy));

				Assert::IsTrue(std::is_sorted(std::begin(_Ct_copy), std::end(_Ct_copy)));
				Assert::AreNotEqual(_Ct_copy.size(), container::size_type{ 0 });
				Assert::AreNotEqual(_Ct_copy.size(), _Ct.size());
				Assert::AreEqual(_Ct_copy.size(), _Ct.size() / 2);
			}

			std::function<bool(size_t)> callback() {
				return std::function<bool(size_t)>([](size_t _Val){
					return _Val % 2 == 1 ? true : false;
				});
			}
		};

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunCopyIf()
		{
			{  // seq
				CopyIfAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(copy_if(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}

			{  //par

				CopyIfAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(copy_if(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}

			{  //vec
				CopyIfAlgoTest<_IterCat, _IterCat2> _Alg;
				_Alg.set_result(copy_if(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}
		}

		TEST_METHOD(CopyIf)
		{
			RunCopyIf<random_access_iterator_tag>();
			RunCopyIf<forward_iterator_tag>();
			RunCopyIf<input_iterator_tag, output_iterator_tag>();
		}

		TEST_METHOD(CopyIfEdgeCases)
		{
			// Empty destination & source
			std::vector<int> vec_empty, dest_empty;
			auto _Iter = copy_if(par, std::begin(vec_empty), std::end(vec_empty), std::begin(dest_empty), [](int){ return true; });
			Assert::IsTrue(_Iter == std::end(dest_empty));

			std::vector<int> vec_one = { 1 }, dest_one = { 2 };

			// Copy one element only
			_Iter = copy_if(par, std::begin(vec_one), std::end(vec_one), std::begin(dest_one), [](int){ return true; });
			Assert::IsTrue(_Iter == std::end(dest_one));
			Assert::AreEqual(dest_one[0], int{ 1 });

			std::vector<int> vec_copy = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			std::vector<int>::iterator _End = std::begin(vec_copy);
			std::advance(_End, 4);

			copy_if(par, std::begin(vec_copy), _End, _End, [](int){ return true; });

			for (size_t _I = 0; _I < 4; ++_I) {
				vec_copy[0] = *_End;
				++_End;
			}

			// Check the copy counts
			std::vector<CopyableOnly> vec_copyable(10), vec_copyable2(10);
			copy_if(par, std::begin(vec_copyable), std::end(vec_copyable), std::begin(vec_copyable2), [](CopyableOnly&){ return true; });

			std::for_each(std::begin(vec_copyable2), std::end(vec_copyable2), [](CopyableOnly& _Val){
				Assert::AreEqual(_Val._Copy_count, 1); // Only one copy
			});

			std::vector<int> vec_data = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
			std::vector<int> vec_out(vec_data.size());

			_Iter = copy_if(par, std::begin(vec_data), std::end(vec_data), std::begin(vec_out), [](int _Val){ return _Val < 4 || _Val > 7; });
			
			Assert::AreEqual(static_cast<size_t>(std::distance(std::begin(vec_out), _Iter)), size_t{ 5 });
		}
	};
} // namespace ParallelSTL_Tests




