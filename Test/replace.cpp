#include "stdafx.h"

namespace ParallelSTL_Tests
{
	TEST_CLASS(ReplaceTest)
	{
		enum AlgoType {
			replace_algo,
			replace_if_algo,
			replace_copy_algo,
			replace_copy_if_algo
		};

		template<typename _IterCat, typename _IterCatOut = _IterCat>
		struct ReplaceAlgoTest :
			public AlgoTest<ReplaceAlgoTest<_IterCat>, _IterCat, size_t, void, _IterCatOut>
		{
			static const size_t OLD_VALUE_MARKER = 1;
			static const size_t NEW_VALUE_MARKER = 2;
			static const size_t MAX_REPLACEMENTS = 10; // 10% replacements in the collection

			std::vector<size_t> _Org;

			AlgoType _AlgoType;
		public:
			ReplaceAlgoTest(AlgoType _Algo) : _AlgoType(_Algo)
			{
				_ASSERTE(OLD_VALUE_MARKER < NEW_VALUE_MARKER);

				std::iota(std::begin(_Ct), std::end(_Ct), NEW_VALUE_MARKER + rand());
				_Ct_copy.resize(_Ct.size());
				std::iota(std::begin(_Ct_copy), std::end(_Ct_copy), rand());

				size_t _Replacemnts = (_Ct.size() / MAX_REPLACEMENTS) + 1; // Make sure that there is at least one replacement
				_ASSERTE(_Ct.size() > _Replacemnts);

				for (size_t _Idx = 0; _Idx < _Replacemnts; ++_Idx)
					_Ct[_Idx] = OLD_VALUE_MARKER;

				std::random_shuffle(std::begin(_Ct), std::end(_Ct));

				_Org = _Ct; // Copy the input collection
			}

			size_t old_value()
			{
				return OLD_VALUE_MARKER;
			}

			size_t new_value()
			{
				return NEW_VALUE_MARKER;
			}

			std::function<bool(size_t)> predicate()
			{
				return std::function<bool(size_t)>([](size_t _Val) {
					return _Val % 2 == 0;
				});
			}

			~ReplaceAlgoTest()
			{
				container _ExpectedDest(_Ct.size());

				switch (_AlgoType) {
				case replace_algo:
					std::replace(std::begin(_Org), std::end(_Org), old_value(), new_value());
					Assert::IsTrue(_Org == _Ct);
					break;
				case replace_if_algo:
					std::replace_if(std::begin(_Org), std::end(_Org), predicate(), new_value());
					Assert::IsTrue(_Org == _Ct);
					break;
				case replace_copy_algo:
					std::replace_copy(std::begin(_Org), std::end(_Org), std::begin(_ExpectedDest), old_value(), new_value());
					Assert::IsTrue(_ExpectedDest == _Ct_copy);
					break;
				case replace_copy_if_algo:
					std::replace_copy_if(std::begin(_Org), std::end(_Org), std::begin(_ExpectedDest), predicate(), new_value());
					Assert::IsTrue(_ExpectedDest == _Ct_copy);
					break;
				}
			}
		};

		template<typename _IterCat>
		void RunReplace()
		{
			{  // seq				
				ReplaceAlgoTest<_IterCat> _Alg(replace_algo);
				replace(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.old_value(), _Alg.new_value());
			}

			{  //par
			ReplaceAlgoTest<_IterCat> _Alg(replace_algo);
				replace(par, _Alg.begin_in(), _Alg.end_in(), _Alg.old_value(), _Alg.new_value());
			}

			{  //vec
				ReplaceAlgoTest<_IterCat> _Alg(replace_algo);
				replace(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.old_value(), _Alg.new_value());
			}
		}

		TEST_METHOD(Replace)
		{
			RunReplace<random_access_iterator_tag>();
			RunReplace<forward_iterator_tag>();
		}

		template<typename _IterCat>
		void RunReplaceIf()
		{
			{  // seq
				ReplaceAlgoTest<_IterCat> _Alg(replace_if_algo);
				replace_if(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate(), _Alg.new_value());
			}

			{  //par
				ReplaceAlgoTest<_IterCat> _Alg(replace_if_algo);
				replace_if(par, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate(), _Alg.new_value());
			}

			{  //vec
				ReplaceAlgoTest<_IterCat> _Alg(replace_if_algo);
				replace_if(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.predicate(), _Alg.new_value());
			}
		}

		TEST_METHOD(ReplaceIf)
		{
			RunReplaceIf<random_access_iterator_tag>();
			RunReplaceIf<forward_iterator_tag>();		
		}

		template<typename _IterCat>
		void RunReplaceCopy()
		{
			{  // seq
				ReplaceAlgoTest<_IterCat> _Alg(replace_copy_algo);
				replace_copy(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.old_value(), _Alg.new_value());
			}

			{  //par
				ReplaceAlgoTest<_IterCat> _Alg(replace_copy_algo);
				replace_copy(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.old_value(), _Alg.new_value());
			}

			{  //vec
				ReplaceAlgoTest<_IterCat> _Alg(replace_copy_algo);
				replace_copy(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.old_value(), _Alg.new_value());
			}
		}

		TEST_METHOD(ReplaceCopy)
		{
			RunReplaceCopy<random_access_iterator_tag>();
			RunReplaceCopy<forward_iterator_tag>();			
		}

		template<typename _IterCat, typename _IterCat2 = _IterCat>
		void RunReplaceCopyIf()
		{
			{  // seq
				ReplaceAlgoTest<_IterCat, _IterCat2> _Alg(replace_copy_if_algo);
				replace_copy_if(seq, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.predicate(), _Alg.new_value());
			}

			{  //par
				ReplaceAlgoTest<_IterCat, _IterCat2> _Alg(replace_copy_if_algo);
				replace_copy_if(par, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.predicate(), _Alg.new_value());
			}

			{  //vec
				ReplaceAlgoTest<_IterCat, _IterCat2> _Alg(replace_copy_if_algo);
				replace_copy_if(vec, _Alg.begin_in(), _Alg.end_in(), _Alg.begin_dest(), _Alg.predicate(), _Alg.new_value());
			}
		}

		TEST_METHOD(ReplaceCopyIf)
		{
			RunReplaceCopyIf<random_access_iterator_tag>();
			RunReplaceCopyIf<forward_iterator_tag>();
			RunReplaceCopyIf<input_iterator_tag, output_iterator_tag>();
		}
	};
} // ParallelSTL_Tests

