// This holds an implementation of accumulate that follows a rather simple 3-step
// reduction strategy: each lane serially reduces, a binary-tree reduction is done
// across each thread, and finally the partial results across all threads are 
// accumulated on the CPU.
#pragma once
#ifndef _AMP_ACCUMULATE_HPP_BUMPTZI
#define _AMP_ACCUMULATE_HPP_BUMPTZI

#include "AMP_helpers.hpp"

#include <amp.h>
#include <iterator>
#include <numeric>

namespace experimental
{
	template<typename I, typename T>
	T accumulate(I f, I l, T init, unsigned int offset = 1u) restrict(amp)
	{
		while (f < l) {
			init += *f;
			f += offset;
		}

		return (init);
	}

	template<typename AV1, typename AV2, typename Ext>
	void accumulate_blocks(const AV1& R, const AV2& accumulated_blocks, const Ext& t_ext)
	{
		using namespace concurrency;

		parallel_for_each(t_ext, [=](const tiled_index<t_ext.tile_dim0>& tidx) restrict(amp) {
			const auto b_f = cbegin(R) + tidx.tile[0] * tidx.tile_dim0;

			using T = typename AV2::value_type;

			tile_static T t_l_m[tidx.tile_dim0];
			t_l_m[tidx.local[0]] = accumulate(b_f + t_ext.size() + tidx.local[0], cend(R), *(b_f + tidx.local[0]), t_ext.size());

			tidx.barrier.wait_with_tile_static_memory_fence();

			reduce(t_l_m, t_l_m + tidx.tile_dim0, tidx);

			if (tidx.tile_origin == tidx.global) accumulated_blocks[tidx.tile] = t_l_m[0];
		});
	}


	template<typename AV, typename T>
	T accumulate(const AV& R, T init)
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(R.extent.size() / SIMD_w, max_thread_cnt);

		const array_view<T, 1> S(thread_cnt);

		accumulate_blocks(R, S, extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>());

		return (std::accumulate(cbegin(S), cend(S), init));
	}
	
	template<typename I, typename T>
	T accumulate(I f, I l, T init)
	{
		if (f >= l) return (init); // Empty range passed, probably an error.

		using namespace concurrency;
		using U = typename std::iterator_traits<I>::value_type;

		return (accumulate(array_view<const U, 1>(std::distance(f, l), &*f), init));
	}

	template<typename I, typename T, typename Op>
	T accumulate(I f, I l, T init, unsigned int offset = 1u, Op op = Op()) restrict(amp)
	{
		while (f < l) {
			init = op(init, *f);
			f += offset;
		}

		return (init);
	}

	template<typename AV1, typename AV2, typename Ext, typename Op>
	void accumulate_blocks(const AV1& R, const AV2& accumulated_blocks, const Ext& t_ext, Op op)
	{
		using namespace concurrency;

		parallel_for_each(t_ext, [=](const tiled_index<t_ext.tile_dim0>& tidx) restrict(amp) {
			const auto b_f = cbegin(R) + tidx.tile[0] * tidx.tile_dim0;

			using T = typename AV2::value_type;

			tile_static T t_l_m[tidx.tile_dim0];
			t_l_m[tidx.local[0]] = accumulate(b_f + t_ext.size() + tidx.local[0], cend(R), *(b_f + tidx.local[0]), t_ext.size(), op);

			tidx.barrier.wait_with_tile_static_memory_fence();

			reduce(t_l_m, t_l_m + tidx.tile_dim0, tidx, op);

			if (tidx.tile_origin == tidx.global) accumulated_blocks[tidx.tile] = t_l_m[0];
		});
	}

	template<typename AV, typename T, typename Op>
	T accumulate(const AV& R, T init, Op op)
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 128u;

		const auto thread_cnt = std::min((R.extent.size() + ((R.extent.size() % SIMD_w) ? 1u : 0u)) / SIMD_w, max_thread_cnt);

		const array_view<T, 1> S(thread_cnt);

		accumulate_blocks(R, S, extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), op);

		return (std::accumulate(cbegin(S), cend(S), init));
	}

	template<typename I, typename T, typename Op>
	T accumulate(I f, I l, T init, Op op)
	{
		if (f >= l) return (init); // Empty range passed, probably an error.	

		using U = typename std::iterator_traits<I>::value_type;
		
		return (accumulate(concurrency::array_view<const U, 1>(std::distance(f, l), &*f), init, op));
	}
}	   // Namespace experimental.
#endif // _AMP_ACCUMULATE_HPP_BUMPTZI
