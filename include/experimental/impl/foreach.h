#pragma once

#ifndef _IMPL_FOREACH_H_
#define _IMPL_FOREACH_H_ 1

#include <type_traits>
#include <iterator>

#include "algorithm_impl.h"

namespace std {
	namespace experimental {
		namespace parallel {
			namespace details {
				//
				//  for_each_n
				//
				template<class _InIt, class _Diff, class _Fn, class _IterTag>
				inline _InIt _For_each_n_impl(const sequential_execution_policy&, _InIt _First, _Diff _Count, _Fn _Func, _IterTag)
				{
					_EXP_TRY
						for (; 0 < _Count; --_Count, ++_First)
							_Func(*_First);

					_EXP_RETHROW

						return _First;
				}

				template<class _ExPolicy, class _InIt, class _Diff, class _Fn, class _IterTag>
				inline _InIt _For_each_n_impl(const _ExPolicy&, _InIt _First, _Diff _Count, _Fn _Func, _IterTag)
				{
					typedef typename std::decay<_ExPolicy>::type _ExecutionPolicy;
					static_assert(is_execution_policy<_ExecutionPolicy>::value, "Incorrect execution policy type");

					if (_Count > 0) {
						return _Partitioner<_ExecutionPolicy>::_For_Each(_First, _Count, [_Func](_InIt _Begin, size_t _Count){

							LoopHelper<_ExecutionPolicy, _InIt>::Loop(_Begin, _Count, [&_Func](const std::iterator_traits<_InIt>::reference _It) {
								_Func(_It);
							});
						});
					}

					return _First;
				}

				template<class _ExPolicy, class _InIt, class _Diff, class _Fn>
				inline typename _enable_if_parallel<_ExPolicy, _InIt>::type _For_each_n_impl(const _ExPolicy&, _InIt _First, _Diff _Count, _Fn _Func, std::input_iterator_tag _Cat)
				{
					return _For_each_n_impl(seq, _First, _Count, _Func, _Cat);
				}

				template <class _InIt, class _Diff, class _Fn, class _IterTag>
				inline _InIt _For_each_n_impl(const execution_policy& _Policy, _InIt _First, _Diff _Count, _Fn _Func, _IterTag _Cat)
				{
					_EXP_GENERIC_EXECUTION_POLICY(_For_each_n_impl, _Policy, _First, _Count, _Func, _Cat);
				}

				//
				//  for_each
				//
				template<class _InIt, class _Fn, class _IterTag>
				inline void _For_each_impl(const sequential_execution_policy&, _InIt _First, _InIt _Last, _Fn _Func, _IterTag)
				{
					_EXP_TRY
						std::for_each(_First, _Last, _Func);
					_EXP_RETHROW
				}

				template<class _ExPolicy, class _InIt, class _Fn, class _IterTag>
				inline void _For_each_impl(const _ExPolicy& _Policy, _InIt _First, _InIt _Last, _Fn _Func, _IterTag)
				{
					_For_each_n_impl(_Policy, _First, std::distance(_First, _Last), _Func, std::_Iter_cat(_First));
				}

				template<class _ExPolicy, class _InIt, class _Fn>
				inline typename _enable_if_parallel<_ExPolicy, void>::type _For_each_impl(const _ExPolicy&, _InIt _First, _InIt _Last, _Fn _Func, std::input_iterator_tag)
				{
					_For_each_impl(seq, _First, _Last, _Func, std::_Iter_cat(_First));
				}

				template<class _InIt, class _Fn, class _IterTag>
				inline void _For_each_impl(const execution_policy& _Policy, _InIt _First, _InIt _Last, _Fn _Func, _IterTag _Cat)
				{
					_EXP_GENERIC_EXECUTION_POLICY(_For_each_impl, _Policy, _First, _Last, _Func, _Cat);
				}
			} // details

			template <class _ExPolicy, class _InIt, class _Diff, class _Fn>
			inline typename details::_enable_if_policy<_ExPolicy, _InIt>::type for_each_n(_ExPolicy&& _Policy, _InIt _First, _Diff _Count, _Fn _Func)
			{
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt>::iterator_category>::value, "Required input iterator or stronger.");

				return details::_For_each_n_impl(_Policy, _First, _Count, _Func, std::_Iter_cat(_First));
			}

			template<class _ExPolicy, class _InIt, class _Fn>
			inline typename details::_enable_if_policy<_ExPolicy, void>::type for_each(_ExPolicy&& _Policy, _InIt _First, _InIt _Last, _Fn _Func)
			{
				static_assert(std::is_base_of<std::input_iterator_tag, typename std::iterator_traits<_InIt>::iterator_category>::value, "Required input iterator or stronger.");

				return details::_For_each_impl(_Policy, _First, _Last, _Func, std::_Iter_cat(_First));
			}
		}
	}
} // std::experimental::parallel

#endif // _IMPL_FOREACH_H_

