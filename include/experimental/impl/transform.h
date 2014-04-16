#pragma once

#ifndef _IMPL_TRANSFORM_H_
#define _IMPL_TRANSFORM_H_ 1

#include "algorithm_impl.h"

namespace std {
	namespace experimental {
		namespace parallel {
			namespace details {
				//
				// transform
				//
				template <class _InIt, class _OutIt, class _Fn, class _IterCat>
				_OutIt _Transform_impl(const sequential_execution_policy&, _InIt _First, _InIt _Last, _OutIt _Dest, _Fn _Func, _IterCat)
				{
					_EXP_TRY
						return std::transform(_First, _Last, _Dest, _Func);
					_EXP_RETHROW
				}

				template <class _ExPolicy, class _InIt, class _OutIt, class _Fn, class _IterCat>
				_OutIt _Transform_impl(const _ExPolicy& _Policy, _InIt _First, _InIt _Last, _OutIt _Dest, _Fn _Func, _IterCat _Cat)
				{
					return std::get<1>(*_For_each_n_impl(_Policy, make_composable_iterator(_First, _Dest), std::distance(_First, _Last),
						[_Func](typename composable_iterator<_InIt, _OutIt>::reference _It){
						*std::get<1>(_It) = _Func(*std::get<0>(_It));
					}, _Cat));
				}

				template <class _ExPolicy, class _InIt, class _OutIt, class _Fn>
				typename _enable_if_parallel<_ExPolicy, _OutIt>::type _Transform_impl(const _ExPolicy&, _InIt _First, _InIt _Last, _OutIt _Dest, _Fn _Func, std::input_iterator_tag _Cat)
				{
					return _Transform_impl(seq, _First, _Last, _Dest, _Func, _Cat);
				}

				template <class _InIt, class _OutIt, class _Fn, class _IterCat>
				_OutIt _Transform_impl(const execution_policy& _Policy, _InIt _First, _InIt _Last, _OutIt _Dest, _Fn _Func, _IterCat _Cat)
				{
					_EXP_GENERIC_EXECUTION_POLICY(_Transform_impl, _Policy, _First, _Last, _Dest, _Func, _Cat);
				}

				//
				// transform binary predicate
				// 
				template <class _InIt, class _InIt2, class _OutIt, class _Fn, class _IterCat>
				_OutIt _Transform_impl_binary(const sequential_execution_policy&, _InIt _First, _InIt _Last, _InIt2 _First2, _OutIt _Dest, _Fn _Func, _IterCat)
				{
					_EXP_TRY
						return std::transform(_First, _Last, _First2, _Dest, _Func);
					_EXP_RETHROW
				}

				template <class _ExPolicy, class _InIt, class _InIt2, class _OutIt, class _Fn, class _IterCat>
				_OutIt _Transform_impl_binary(const _ExPolicy& _Policy, _InIt _First, _InIt _Last, _InIt2 _First2, _OutIt _Dest, _Fn _Func, _IterCat _Cat)
				{
					return std::get<2>(*_For_each_n_impl(_Policy, make_composable_iterator(_First, _First2, _Dest), std::distance(_First, _Last),
						[_Func](typename composable_iterator<_InIt, _InIt2, _OutIt>::reference _It){
						*std::get<2>(_It) = _Func(*std::get<0>(_It), *std::get<1>(_It));
					}, _Cat));
				}

				template <class _ExPolicy, class _InIt, class _InIt2, class _OutIt, class _Fn>
				typename _enable_if_parallel<_ExPolicy, _OutIt>::type _Transform_impl_binary(const _ExPolicy&, _InIt _First, _InIt _Last, _InIt2 _First2, _OutIt _Dest, _Fn _Func, std::input_iterator_tag _Cat)
				{
					return _Transform_impl_binary(seq, _First, _Last, _First2, _Dest, _Func, _Cat);
				}

				template <class _ExPolicy, class _InIt, class _InIt2, class _OutIt, class _Fn, class _IterCat>
				_OutIt _Transform_impl_binary(const execution_policy& _Policy, _InIt _First, _InIt _Last, _InIt2 _First2, _OutIt _Dest, _Fn _Func, _IterCat _Cat)
				{
					_EXP_GENERIC_EXECUTION_POLICY(_Transform_impl_binary, _Policy, _First, _Last, _First2, _Dest, _Func, _Cat);
				}
			} // details

			template <class _ExPolicy, class _InIt, class _OutIt, class _Fn>
			inline typename details::_enable_if_policy<_ExPolicy, _OutIt>::type transform(_ExPolicy&& _Policy, _InIt _First, _InIt _Last, _OutIt _Dest, _Fn _Func)
			{
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt>::iterator_category>::value, "Required input iterator or stronger.");
				static_assert(std::is_base_of<std::_Mutable_iterator_tag, typename std::iterator_traits<_OutIt>::iterator_category>::value, "Required output iterator or stronger.");

				details::common_iterator<_InIt, _OutIt>::iterator_category _Cat;
				return details::_Transform_impl(_Policy, _First, _Last, _Dest, _Func, _Cat);
			}

			template <class _ExPolicy, class _InIt, class _InIt2, class _OutIt, class _Fn>
			inline typename details::_enable_if_policy<_ExPolicy, _OutIt>::type transform(_ExPolicy&& _Policy, _InIt _First, _InIt _Last, _InIt2 _First2, _OutIt _Dest, _Fn _Func)
			{
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt>::iterator_category>::value, "Required input iterator or stronger.");
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt2>::iterator_category>::value, "Required input iterator or stronger.");
				static_assert(std::is_base_of<std::_Mutable_iterator_tag, typename std::iterator_traits<_OutIt>::iterator_category>::value, "Required output iterator or stronger.");

				details::common_iterator<_InIt, _InIt2, _OutIt>::iterator_category _Cat;
				return details::_Transform_impl_binary(_Policy, _First, _Last, _First2, _Dest, _Func, _Cat);
			}
		}
	}
} // std::experimental::parallel

#endif // _IMPL_TRANSFORM_H_
