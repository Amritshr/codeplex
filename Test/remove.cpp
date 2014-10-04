#include "stdafx.h"

namespace ParallelSTL_Tests
{
	const static size_t MARKER_VAL = 1;
	const static size_t MARKER_COUNT = 30; // 30% of collections are markers

	template<typename _IterCat, typename _OutIterCat = _IterCat>
	struct RemoveAlgoTest :
		public AlgoTest<RemoveAlgoTest<_IterCat, _OutIterCat>, _IterCat, size_t, void, _OutIterCat>
	{
		size_t _Marker_count;
		bool _Copy;

		RemoveAlgoTest(bool _Cpy) : _Copy(_Cpy)
		{
			std::iota(std::begin(_Ct), std::end(_Ct), rand() + MARKER_VAL);
			if (_Cpy)
				_Ct_copy.resize(_Ct.size());

			_Marker_count = (_Ct.size() * MARKER_COUNT) / 100;

			for (size_t _I = 0; _I < _Marker_count; ++_I)
				_Ct[_I] = MARKER_VAL;

			std::random_shuffle(std::begin(_Ct), std::end(_Ct));
		}

		~RemoveAlgoTest()
		{
			auto _Size = _Ct.size();
			container *_Val = &_Ct;
			if (_Copy)
				_Val = &_Ct_copy;
			
			_Val->resize(std::distance(std::begin(*_Val), _Result.get_inner()));

			Assert::AreEqual(_Size - _Marker_count, _Val->size());

			Assert::IsTrue(std::find_if_not(std::begin(*_Val), std::end(*_Val), [](size_t _Val) {
				return MARKER_VAL < _Val;
			}) == std::end(*_Val));

			Assert::AreNotEqual(container::size_type{ 0 }, _Val->size());
			Assert::AreNotEqual(_Size, _Val->size());
		}

		std::function<bool(size_t)> callback() {
			return std::function<bool(size_t)>([](size_t _Val){
				return _Val < MARKER_VAL * 2;
			});
		}
	};

	TEST_CLASS(RemoveTest)
	{
		template<typename _IterCat>
		void RunRemove()
		{
			{  // seq
				RemoveAlgoTest<_IterCat> _Alg(false);
				_Alg.set_result(remove(seq, _Alg.begin_in(), _Alg.end_in(), MARKER_VAL));
			}

			{  //par
				RemoveAlgoTest<_IterCat> _Alg(false);
				_Alg.set_result(remove(par, _Alg.begin_in(), _Alg.end_in(), MARKER_VAL));
			}

			{  //par_vec
				RemoveAlgoTest<_IterCat> _Alg(false);
				_Alg.set_result(remove(par_vec, _Alg.begin_in(), _Alg.end_in(), MARKER_VAL));
			}
		}

		TEST_METHOD(Remove)
		{
			RunRemove<random_access_iterator_tag>();
			RunRemove<forward_iterator_tag>();
		}
	};

	TEST_CLASS(RemoveCopyTest)
	{
		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunRemoveCopy()
		{
			{  // seq
				RemoveAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(remove_copy(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VAL));
			}

			{  //par

				RemoveAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(remove_copy(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VAL));
			}

			{  //par_vec
				RemoveAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(remove_copy(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), MARKER_VAL));
			}
		}

		TEST_METHOD(RemoveCopy)
		{
			RunRemoveCopy<random_access_iterator_tag>();
			RunRemoveCopy<forward_iterator_tag>();
			RunRemoveCopy<input_iterator_tag, output_iterator_tag>();
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunRemoveCopyIf()
		{
			{  // seq
				RemoveAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(remove_copy_if(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}

			{  //par
				RemoveAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(remove_copy_if(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}

			{  //par_vec
				RemoveAlgoTest<_IterCat, _IterCat2> _Alg(true);
				_Alg.set_result(remove_copy_if(par_vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.callback()));
			}
		}

		TEST_METHOD(RemoveCopyIf)
		{
			RunRemoveCopyIf<random_access_iterator_tag>();
			RunRemoveCopyIf<forward_iterator_tag>();
			RunRemoveCopyIf<input_iterator_tag, output_iterator_tag>();
		}

		TEST_METHOD(RemoveCopyEdgeCases)
		{
			// Empty destination & source
			std::vector<int> vec_empty, dest_empty;
			auto _Iter = remove_copy(par, std::begin(vec_empty), std::end(vec_empty), std::begin(dest_empty), 1);
			Assert::IsTrue(_Iter == std::end(dest_empty));

			std::vector<int> vec_one = { 1 }, dest_one = { 2 };

			// Copy one element only
			_Iter = remove_copy(par, std::begin(vec_one), std::end(vec_one), std::begin(dest_one), 0);
			Assert::IsTrue(_Iter == std::end(dest_one));
			Assert::AreEqual(dest_one[0], int{ 1 });

			std::vector<int> vec_copy = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };
			std::vector<int>::iterator _End = std::begin(vec_copy);
			std::advance(_End, 4);

			remove_copy_if(par, std::begin(vec_copy), _End, _End, [](int){ return false; });

			for (size_t _I = 0; _I < 4; ++_I) {
				vec_copy[0] = *_End;
				++_End;
			}

			std::vector<int> vec_data = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
			std::vector<int> vec_out(vec_data.size());
			_Iter = remove_copy_if(par, std::begin(vec_data), std::end(vec_data), std::begin(vec_out), [](int _Val){ return _Val < 4 || _Val > 7; });
			Assert::AreEqual(static_cast<size_t>(std::distance(std::begin(vec_out), _Iter)), size_t{ 4 });
		}
	};
}
