#pragma once

#ifndef _IMPL_EQUAL_H_
#define _IMPL_EQUAL_H_ 1

#include "algorithm_impl.h"

namespace std {
	namespace experimental {
		namespace parallel {
			namespace details {
				//
				// equal
				//
				template <class _InIt, class _InIt2, class _Pr, class _IterCat>
				bool _Equal_impl(const sequential_execution_policy&, _InIt _First, _InIt _Last, _InIt2 _First2, _Pr _Pred, _IterCat)
				{
					_EXP_TRY
						return std::equal(_First, _Last, _First2, _Pred);
					_EXP_RETHROW
				}

				template <class _ExPolicy, class _InIt, class _InIt2, class _Pr, class _IterCat>
				bool _Equal_impl(const _ExPolicy&, _InIt _First, _InIt _Last, _InIt2 _First2, _Pr _Pred, _IterCat)
				{
					typedef typename std::decay<_ExPolicy>::type _ExecutionPolicy;

					if (_First != _Last)
					{
						cancellation_token _Token;

						_Partitioner<_ExecutionPolicy>::_For_Each(make_composable_iterator(_First, _First2), std::distance(_First, _Last),
							[_Pred, &_Token](typename details::composable_iterator<_InIt, _InIt2>& _Begin, size_t _Count){

							LoopHelper<_ExecutionPolicy, typename composable_iterator<_InIt, _InIt2> >::Loop(_Begin, _Count,
								[&_Token, &_Pred](typename composable_iterator<_InIt, _InIt2>::reference _It){
								if (!_Pred(*std::get<0>(_It), *std::get<1>(_It)))
									_Token.cancel();
							}, _Token);

						});

						return !_Token.is_cancelled();
					}

					return true;
				}

				template <class _ExPolicy, class _InIt, class _InIt2, class _Pr>
				inline typename _enable_if_parallel<_ExPolicy, bool>::type _Equal_impl(const _ExPolicy&, _InIt _First, _InIt _Last, _InIt2 _First2, _Pr _Pred, std::input_iterator_tag _Cat)
				{
					return _Equal_impl(seq, _First, _Last, _First2, _Pred, _Cat);
				}

				template <class _InIt, class _InIt2, class _Pr, class _IterCat>
				inline bool _Equal_impl(const execution_policy& _Policy, _InIt _First, _InIt _Last, _InIt2 _First2, _Pr _Pred, _IterCat _Cat)
				{
					_EXP_GENERIC_EXECUTION_POLICY(_Equal_impl, _Policy, _First, _Last, _First2, _Pred, _Cat);
				}
			} // details

			template <class _ExPolicy, class _InIt, class _InIt2, class _Pr>
			inline typename details::_enable_if_policy<_ExPolicy, bool>::type equal(_ExPolicy&& _Policy, _InIt _First, _InIt _Last, _InIt2 _First2, _Pr _Pred)
			{
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt>::iterator_category>::value, "Required input iterator or stronger.");
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt2>::iterator_category>::value, "Required input iterator or stronger.");

				return details::_Equal_impl(_Policy, _First, _Last, _First2, _Pred, std::_Iter_cat(_First));
			}

			template <class _ExPolicy, class _InIt, class _InIt2>
			inline typename details::_enable_if_policy<_ExPolicy, bool>::type equal(_ExPolicy&& _Policy, _InIt _First, _InIt _Last, _InIt2 _First2)
			{
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt>::iterator_category>::value, "Required input iterator or stronger.");
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt2>::iterator_category>::value, "Required input iterator or stronger.");

				return details::_Equal_impl(_Policy, _First, _Last, _First2, std::equal_to<>(), std::_Iter_cat(_First));
			}
		}
	}
} // std::experimental::parallel

#endif // _IMPL_EQUAL_H_
