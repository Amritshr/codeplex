// This implements p-ary search and hides it under the binary_search interface (perhaps we should fix the strange bool return though).
// In spite of discussions being ample around how it is evil to use anything but the binary search algorithm for binary_search, usage
// is frequent enough in practice to warrant having a proper implementation. More information about p-ary search can be retrieved from
// https://www.usenix.org/legacy/event/hotpar09/tech/full_papers/kaldeway/kaldeway_html/. We also provide a(n overloaded) version for
// restrict(amp) contexts.
#pragma once
#ifndef _AMP_BINARY_SEARCH_HPP_BUMPTZI
#define _AMP_BINARY_SEARCH_HPP_BUMPTZI

#include "AMP_helpers.hpp"

#include <amp.h>
#include <iterator>
#include <utility>

namespace experimental
{
	template<typename I, typename T>
	bool binary_search(I f, I l, const T& val) restrict(amp)
	{
		while (f < l) {
			auto m = f + (l - f) / 2u;

			if (*m == val) return (true);

			if (*m < val) f = ++m;
			else l = m;
		}

		return (false);
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
	bool binary_search(const AV& r, const T& val)
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, R.extent.size() / SIMD_w);
		const auto work_per_thread = R.extent.size() / thread_cnt;

		array_view<unsigned int, 1> found(1);
		found[{0}] = 0u;

		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			const auto t_f = cbegin(r) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + ((tidx.tile[0] != (thread_cnt - 1)) ? work_per_thread : (cend(r) - t_f));

			tile_static bool early_out;
			if (tidx.tile_origin == tidx.global) early_out = (found[{0}] != 0u) || (val < *t_f) || (*(t_l - 1) < val);

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (early_out) return;

			const auto el = p_ary_search_subset(t_f, t_l, val, tidx) + tidx.local[0]; 
			
			if ((el < t_l) && (*el == val)) found[{0}] = 1u;
		});

		return (found[{0}] != 0u);
	}

	template<typename I, typename T>
	bool binary_search(I f, I l, const T& val)
	{
		if (f >= l) return (false); // Empty range passed, probably an error.
	
		return (binary_search(concurrency::array_view<const Iterator_value_type<I>, 1>(std::distance(f, l), &*f), val));
	}

	template<typename I, typename T, typename Idx, typename C>
	I p_ary_search_subset(I f, I l, const T& val, const Idx& tidx, C cmp) restrict(amp)
	{
		tile_static unsigned int subset;
		if (tidx.tile_origin == tidx.global) subset = 0u;

		tidx.barrier.wait_with_tile_static_memory_fence();

		int work_per_lane = (l - f) / tidx.tile_dim0;
		while (work_per_lane) {
			const auto l_f = f + subset + tidx.local[0] * work_per_lane;

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (cmp(*l_f, val)) concurrency::atomic_fetch_max(&subset, l_f - f);

			tidx.barrier.wait_with_tile_static_memory_fence();

			work_per_lane /= tidx.tile_dim0;
		}

		return (f + subset);
	}

	template<typename AV, typename T, typename C>
	bool binary_search(const AV& r, const T& val, C cmp)
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, R.extent.size() / SIMD_w);
		const auto work_per_thread = R.extent.size() / thread_cnt;

		array_view<unsigned int, 1> found(1);
		found[{0}] = 0u;

		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			const auto t_f = cbegin(r) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + ((tidx.tile[0] != (thread_cnt - 1)) ? work_per_thread : (cend(r) - t_f));

			tile_static bool early_out;
			if (tidx.tile_origin == tidx.global) early_out = (found[{0}] != 0u) || cmp(val, *t_f) || cmp(*(t_l - 1), val);

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (early_out) return;

			const auto el = p_ary_search_subset(t_f, t_l, val, tidx, cmp) + tidx.local[0];

			if ((el < t_l) && (*el == val)) found[{0}] = 1u;
		});

		return (found[{0}] != 0u);
	}
	
	template<typename I, typename T, typename C>
	bool binary_search(I f, I l, const T& val, C cmp)
	{
		if (f >= l) return (false); // Empty range passed, probably an error.

		return (binary_search(concurrency::array_view<const Iterator_value_type<I>, 1>(std::distance(f, l), &*f), val, cmp));
	}
}	   // Namespace experimental.
#endif // _AMP_BINARY_SEARCH_HPP_BUMPTZI