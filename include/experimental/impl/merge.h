#pragma once

#ifndef _IMPL_MERGE_H_
#define _IMPL_MERGE_H_ 1

#include "algorithm_impl.h"
#include "taskgroup.h"

namespace std {
	namespace experimental {
		namespace parallel {
			namespace details {

				// "move" operation is applied between buffers
				template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Random_output_iterator, typename _Function>
				void _Merge_chunks(_Random_iterator _Begin1, const _Random_iterator &_End1, _Random_buffer_iterator _Begin2, const _Random_buffer_iterator &_End2,
					_Random_output_iterator _Output, const _Function &_Func)
				{
					while (_Begin1 != _End1 && _Begin2 != _End2)
					{
						// be aware of stability of the merge algo
						if (_Func(*_Begin2, *_Begin1))
						{
							*_Output = std::move(*_Begin2);
							++_Begin2;
						}
						else
						{
							*_Output = std::move(*_Begin1);
							++_Begin1;
						}
						++_Output;
					}

					if (_Begin1 != _End1)
					{
						std::move(_Begin1, _End1, _Output);
					}
					else if (_Begin2 != _End2)
					{
						std::move(_Begin2, _End2, _Output);
					}
				}

				// Find out two middle points for two sorted arrays by binary search so that the number of total elements on the left part of two middle points is equal
				// to the number of total elements on the right part of two sorted arrays and all elements on the left part is smaller than right part. 
				template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Function>
				size_t _Search_mid_point(const _Random_iterator &_Begin1, size_t &_Len1, const _Random_buffer_iterator &_Begin2, size_t &_Len2, const _Function &_Func)
				{
					size_t _Len = (_Len1 + _Len2) / 2, _Lower1 = 0, lower2 = 0;

					size_t _Upper1 = _Len1, _Upper2 = _Len2;
					while (_Lower1 < _Upper1 && lower2 < _Upper2)
					{
						size_t _Mid1 = (_Lower1 + _Upper1) / 2, _Mid2 = (lower2 + _Upper2) / 2;
						if (_Func(_Begin1[_Mid1], _Begin2[_Mid2]))
						{
							if (_Mid1 + _Mid2 < _Len)
								_Lower1 = _Mid1 + 1;
							else
								_Upper2 = _Mid2;
						}
						else
						{
							if (_Mid1 + _Mid2 < _Len)
								lower2 = _Mid2 + 1;
							else
								_Upper1 = _Mid1;
						}
					}

					if (_Lower1 == _Upper1)
						_Upper2 = _Len - _Upper1;
					else
						_Upper1 = _Len - _Upper2;

					// If the middle elements in 2 ranges are identical, we need to move the split point to avoid unstable merge result
					if (_Upper1 != _Len1 && _Upper2 != 0 && !_Func(_Begin1[_Upper1], _Begin2[_Upper2 - 1]) && !_Func(_Begin2[_Upper2 - 1], _Begin1[_Upper1]))
					{
						size_t _Offset = (std::min)(std::upper_bound(_Begin1 + _Upper1, _Begin1 + _Len1, _Begin1[_Upper1], _Func) - _Begin1 - _Upper1,
							_Upper2 - (std::lower_bound(_Begin2, _Begin2 + _Upper2, _Begin2[_Upper2 - 1], _Func) - _Begin2));
						_Upper1 += _Offset;
						_Upper2 -= _Offset;
					}
					_Len1 = _Upper1;
					_Len2 = _Upper2;

					return _Len;
				}

				// _Div_num of threads(tasks) merge two chunks in parallel, _Div_num should be power of 2, if not, the largest power of 2 that is
				// smaller than _Div_num will be used
				template<typename _Random_iterator, typename _Random_buffer_iterator, typename _Random_output_iterator, typename _Function>
				void _Parallel_merge(_Random_iterator _Begin1, size_t _Len1, _Random_buffer_iterator _Begin2, size_t _Len2, _Random_output_iterator _Output,
					const _Function &_Func, size_t _Div_num)
				{
					// Turn to serial merge or continue splitting chunks base on "_Div_num"
					if (_Div_num <= 1 || (_Len1 <= 1 && _Len2 <= 1))
					{
						_Merge_chunks(_Begin1, _Begin1 + _Len1, _Begin2, _Begin2 + _Len2, _Output, _Func);
					}
					else
					{
						size_t _Mid_len1 = _Len1, _Mid_len2 = _Len2;
						size_t _Mid = _Search_mid_point(_Begin1, _Mid_len1, _Begin2, _Mid_len2, _Func);

						TaskGroup _Tg;
						auto _Handle = make_task([&]
						{
							_Parallel_merge(_Begin1, _Mid_len1, _Begin2, _Mid_len2, _Output, _Func, _Div_num / 2);
						});
						_Tg.run(_Handle);

						_Parallel_merge(_Begin1 + _Mid_len1, _Len1 - _Mid_len1, _Begin2 + _Mid_len2, _Len2 - _Mid_len2, _Output + _Mid, _Func, _Div_num / 2);

						_Tg.wait();
					}
				}

				//
				// merge
				//
				template<class _InIt, class _InIt2, class OutIt, class _Pr, class _IterCat>
				inline OutIt _Merge_impl(const sequential_execution_policy&, _InIt _First, _InIt _Last, _InIt2 _First2, _InIt2 _Last2, OutIt _Dest, _Pr _Pred, _IterCat)
				{
					_EXP_TRY
						return std::merge(_First, _Last, _First2, _Last2, _Dest, _Pred);
					_EXP_RETHROW
				}

				template<class _ExPolicy, class _InIt, class _InIt2, class OutIt, class _Pr>
				inline typename _enable_if_parallel<_ExPolicy, OutIt>::type _Merge_impl(const _ExPolicy&, _InIt _First, _InIt _Last, _InIt2 _First2, _InIt2 _Last2, OutIt _Dest, _Pr _Pred, std::random_access_iterator_tag)
				{
					size_t _Size1 = std::distance(_First, _Last);
					size_t _Size2 = std::distance(_First2, _Last2);
					_Parallel_merge(_First, _Size1, _First2, _Size2, _Dest, _Pred, std::thread::hardware_concurrency());
					std::advance(_Dest, _Size1 + _Size2);
					return _Dest;
				}

				template<class _ExPolicy, class _InIt, class _InIt2, class OutIt, class _Pr, class _IterCat>
				inline typename _enable_if_parallel<_ExPolicy, OutIt>::type _Merge_impl(const _ExPolicy&, _InIt _First, _InIt _Last, _InIt2 _First2, _InIt2 _Last2, OutIt _Dest, _Pr _Pred, _IterCat _Cat)
				{
					return _Merge_impl(seq, _First, _Last, _First2, _Last2, _Dest, _Pred, _Cat);
				}

				template<class _InIt, class _InIt2, class OutIt, class _Pr, class _IterCat>
				inline OutIt _Merge_impl(const execution_policy& _Policy, _InIt _First, _InIt _Last, _InIt2 _First2, _InIt2 _Last2, OutIt _Dest, _Pr _Pred, _IterCat _Cat)
				{
					_EXP_GENERIC_EXECUTION_POLICY(_Merge_impl, _Policy, _First, _Last, _First2, _Last2, _Dest, _Pred, _Cat);
				}

				//
				// inplace_merge
				//
				template<class _BidIt, class _Pr, class _IterCat>
				inline void _Inplace_merge_impl(const sequential_execution_policy&, _BidIt _First, _BidIt _Mid, _BidIt _Last, _Pr _Pred, _IterCat)
				{
					_EXP_TRY
						std::inplace_merge(_First, _Mid, _Last, _Pred);
					_EXP_RETHROW
				}

				template<class _ExPolicy, class _BidIt, class _Pr>
				inline typename _enable_if_parallel<_ExPolicy, void>::type _Inplace_merge_impl(const _ExPolicy& _Policy, _BidIt _First, _BidIt _Mid, _BidIt _Last, _Pr _Pred, std::random_access_iterator_tag)
				{
					size_t _Size = std::distance(_First, _Last);

					auto _Holder = std::make_unique<typename std::iterator_traits<_BidIt>::value_type[]>(_Size);

					_Merge_impl(_Policy, _First, _Mid, _Mid, _Last, stdext::make_unchecked_array_iterator(_Holder.get()), _Pred, std::random_access_iterator_tag());

					move(_Policy, stdext::make_unchecked_array_iterator(_Holder.get()), stdext::make_unchecked_array_iterator(_Holder.get() + _Size), _First);
				}

				template<class _ExPolicy, class _BidIt, class _Pr, class _IterCat>
				inline typename _enable_if_parallel<_ExPolicy, void>::type _Inplace_merge_impl(const _ExPolicy&, _BidIt _First, _BidIt _Mid, _BidIt _Last, _Pr _Pred, _IterCat _Cat)
				{
					_Inplace_merge_impl(seq, _First, _Mid, _Last, _Pred, _Cat);
				}

				template<class _BidIt, class _Pr, class _IterCat>
				inline void _Inplace_merge_impl(const execution_policy& _Policy, _BidIt _First, _BidIt _Mid, _BidIt _Last, _Pr _Pred, _IterCat _Cat)
				{
					_EXP_GENERIC_EXECUTION_POLICY(_Inplace_merge_impl, _Policy, _First, _Mid, _Last, _Pred, _Cat);
				}
			} // details

			template<class _ExPolicy, class _InIt, class _InIt2, class _OutIt, class _Pr>
			inline typename details::_enable_if_policy<_ExPolicy, _OutIt>::type merge(_ExPolicy&& _Policy, _InIt _First, _InIt _Last, _InIt2 _First2, _InIt2 _Last2, _OutIt _Dest, _Pr _Pred)
			{
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt>::iterator_category>::value, "Required input iterator or stronger.");
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt2>::iterator_category>::value, "Required input iterator or stronger.");
				static_assert(std::is_base_of<std::_Mutable_iterator_tag, typename std::iterator_traits<_OutIt>::iterator_category>::value, "Required output iterator or stronger.");

				details::common_iterator<_InIt, _InIt2, _OutIt>::iterator_category _Cat;
				return details::_Merge_impl(_Policy, _First, _Last, _First2, _Last2, _Dest, _Pred, _Cat);
			}

			template<class _ExPolicy, class _InIt, class _InIt2, class _OutIt>
			inline typename details::_enable_if_policy<_ExPolicy, _OutIt>::type merge(_ExPolicy&& _Policy, _InIt _First, _InIt _Last, _InIt2 _First2, _InIt2 _Last2, _OutIt _Dest)
			{
				return merge(std::forward<_ExPolicy>(_Policy), _First, _Last, _First2, _Last2, _Dest, std::less<>());
			}

			template<class _ExPolicy, class _BidIt, class _Pr>
			inline typename details::_enable_if_policy<_ExPolicy, void>::type inplace_merge(_ExPolicy&& _Policy, _BidIt _First, _BidIt _Mid, _BidIt _Last, _Pr _Pred)
			{
				static_assert(std::is_base_of<std::bidirectional_iterator_tag, typename std::iterator_traits<_BidIt>::iterator_category>::value, "Required bidirectional iterator or stronger.");

				details::_Inplace_merge_impl(_Policy, _First, _Mid, _Last, _Pred, std::_Iter_cat(_First));
			}

			template<class _ExPolicy, class _BidIt>
			inline typename details::_enable_if_policy<_ExPolicy, void>::type inplace_merge(_ExPolicy&& _Policy, _BidIt _First, _BidIt _Mid, _BidIt _Last)
			{
				inplace_merge(std::forward<_ExPolicy>(_Policy), _First, _Mid, _Last, std::less<>());
			}
		}
	}
} // std::experimental::parallel

#endif // _IMPL_MERGE_H_
