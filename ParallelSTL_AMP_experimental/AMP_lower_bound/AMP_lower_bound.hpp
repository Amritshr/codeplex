// The lower_bound implementation, given its binary_search essence relies on p_ary
// search. More information about p-ary search can be retrieved from
// https://www.usenix.org/legacy/event/hotpar09/tech/full_papers/kaldeway/kaldeway_html/
// We also provide a(n overloaded) version for restrict(amp) contexts.
#pragma once
#ifndef _AMP_LOWER_BOUND_HPP_BUMPTZI
#define _AMP_LOWER_BOUND_HPP_BUMPTZI

#include "AMP_helpers.hpp"

#include <amp.h>
#include <iterator>

namespace experimental
{
	template<typename I, typename T>
	I lower_bound(I f, I l, const T& val) restrict(amp)
	{
		while (f < l) {
			auto m = f + (l - f) / 2u;
						
			if (*m < val) f = ++m;
			else l = m;
		}

		return (f);
	}

	template<typename I, typename T, typename Idx>
	I p_ary_search_subset(I f, I l, const T& val, const Idx& tidx) restrict(amp)
	{
		tile_static unsigned int subset;
		if (tidx.tile_origin == tidx.global) subset = 0u;

		tidx.barrier.wait_with_tile_static_memory_fence();

		int work_per_lane = (l - f) / tidx.tile_dim0;
		while (work_per_lane) {
			const auto l_f = f + subset + tidx.local[0] * work_per_lane;

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (*l_f < val) concurrency::atomic_fetch_max(&subset, l_f - f);

			tidx.barrier.wait_with_tile_static_memory_fence();

			work_per_lane /= tidx.tile_dim0;
		}

		return (f + subset);
	}

	template<typename AV, typename T>
	auto lower_bound(const AV& r, const T& val) -> decltype(cbegin(r))
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const int thread_cnt = std::min(max_thread_cnt, std::max(1u, r.extent.size() / SIMD_w));
		const int work_per_thread = r.extent.size() / thread_cnt;

		array_view<unsigned int, 1> l_b_idx(1);
		l_b_idx[{0}] = r.extent.size();

		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {	
			const auto t_f = cbegin(r) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + min(work_per_thread, cend(r) - t_f);

			tile_static bool early_out;
			if (tidx.tile_origin == tidx.global) early_out = (l_b_idx[{0}] < (t_f - cbegin(r))) || (*(t_l - 1) < val);

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (early_out) return;

			const auto el = p_ary_search_subset(t_f, t_l, val, tidx) + tidx.local[0]; 
			
			if ((el < t_l) && (*el >= val)) atomic_fetch_min(&l_b_idx[{0}], el - cbegin(r));
		});

		return (cbegin(r) + l_b_idx[{0}]);
	}

	template<typename I, typename T>
	I lower_bound(I f, I l, const T& val)
	{
		if (f >= l) return (l); // Empty range passed, probably an error.

		const concurrency::array_view<const Iterator_value_type<I>> r(std::distance(f, l), &*f)
		return (f + (lower_bound(r, val) - cbegin(r)));
	}

	template<typename I, typename T, typename C>
	I lower_bound(I f, I l, const T& val, C cmp)
	{

	}
}	   // Namespace experimental.
#endif // _AMP_LOWER_BOUND_HPP_BUMPTZI