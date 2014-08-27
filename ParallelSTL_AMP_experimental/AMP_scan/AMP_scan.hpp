// Fast, thread_local parallel scan as introduced in Dotsenko, Y., Govindaraju, N. K., Sloan, P., Boyd, C.
// & Manferdelli, J. (2008) "Fast Scan Algorithms on Graphics Processors". 
#pragma once
#ifndef _AMP_SCAN_HPP_BUMPTZI
#define _AMP_SCAN_HPP_BUMPTZI

#include "AMP_helpers.hpp"

#include <amp.h>
#include <array>
#include <iterator>

namespace experimental
{
	// Auxiliary function for reducing the rows in the Matrix (Step 1 in the paper). Not intended for general use.
	template<typename I, typename O, typename Idx>
	O reduce_rows(I i_f, I i_l, O o_f, unsigned int col_cnt, const Idx& tidx) restrict(amp)
	{
		using T = typename std::iterator_traits<I>::value_type;

		const auto f = i_f + tidx.local[0] * col_cnt;
		const auto l = f + min<unsigned int>(col_cnt, max(0, i_l - f));

		*(o_f + tidx.local[0]) = serial_accumulate(f, l, T());
	
		return (o_f);
	}
	// Auxiliary function for doing the final scan of the rows (step III in the paper). Not intended for general use.
	template<typename I1, typename I2, typename Idx>
	void scan_rows(I1 f, I1 l, I2 row_sums, unsigned int col_cnt, const Idx& tidx) restrict(amp)
	{
		auto row_f = f + tidx.local[0] * col_cnt;
		auto row_l = row_f + min<unsigned int>(col_cnt, max(0, l - row_f));
		auto res = *(row_sums + tidx.local[0]);

		while (row_f != row_l) {
			auto tmp = *row_f;
			*row_f++ = res;
			res += tmp;
		}
		
	}
	// And finally the scan entry-point. Note that this is for per-tile work and we have not extended it (yet) to work on a wide
	// arbitrary range residing in RAM.
	template<typename I, typename Idx>
	I scan_block(I f, I l, const Idx& tidx) restrict(amp)
	{
		if (f >= l) return (l); // Empty range passed, probably an error.

		const auto col_cnt = ((l - f) / tidx.tile_dim0 + (((l - f) % tidx.tile_dim0) ? 1u : 0u)) | 1u; // The last OR pads to an odd count thus ensuring co-primality with the count of LDS banks.

		using T = typename std::iterator_traits<I>::value_type;
		
		tile_static T row_sums[tidx.tile_dim0];
		reduce_rows(f, l, row_sums, col_cnt, tidx);

		tidx.barrier.wait_with_tile_static_memory_fence();

		if (tidx.tile_origin == tidx.global) serial_scan(row_sums, row_sums + tidx.tile_dim0);//scan_sums(row_sums, row_sums + tidx.tile_dim0, tidx);

		tidx.barrier.wait_with_tile_static_memory_fence();

		scan_rows(f, l, row_sums, col_cnt, tidx);

		return (--l);
	}

	template<typename AV1, typename AV2, typename Ext>
	void reduce_blocks(const AV1& R, const AV2& reduce_res, const Ext& t_ext)
	{
		using namespace concurrency;
		using T = typename AV2::value_type;

		const auto thread_cnt = t_ext.size() / t_ext.tile_dim0;
		const auto work_per_thread = R.extent.size() / thread_cnt + ((R.extent.size() % thread_cnt) ? 1 : 0);

		parallel_for_each(t_ext, [=](const tiled_index<t_ext.tile_dim0>& tidx) restrict(amp) {
			auto t_f = cbegin(R) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + min<int>(work_per_thread, max(0, cend(R) - t_f));

			T res = T();
			while ((t_f + tidx.local[0]) < t_l) {
				res += *(t_f + tidx.local[0]);
				t_f += tidx.tile_dim0;
			}

			tile_static T t_l_m[tidx.tile_dim0];
			t_l_m[tidx.local[0]] = res;

			tidx.barrier.wait_with_tile_static_memory_fence();

			reduce(t_l_m, t_l_m + tidx.tile_dim0, tidx);

			if (tidx.tile_origin == tidx.global) reduce_res[tidx.tile] = t_l_m[0];
		});
	}

	template<unsigned int thread_cnt, typename AV1, typename Ext>
	void scan_reduced_blocks(const AV1& reduce_res, const Ext& t_ext)
	{
		using namespace concurrency;
		using T = typename AV1::value_type;
		
		parallel_for_each(t_ext, [=](const tiled_index<t_ext.tile_dim0>& tidx) restrict(amp) { // Single thread.
			tile_static T t_l_m[thread_cnt];
			copy(cbegin(reduce_res) + tidx.local[0], cend(reduce_res), t_l_m + tidx.local[0], tidx.tile_dim0);

			tidx.barrier.wait_with_tile_static_memory_fence();

			scan_block(t_l_m, t_l_m + thread_cnt, tidx);

			tidx.barrier.wait_with_tile_static_memory_fence();

			copy(t_l_m + tidx.local[0], t_l_m + thread_cnt, begin(reduce_res) + tidx.local[0], tidx.tile_dim0);
		});
	}

	template<typename AV1, typename AV2, typename Ext>
	void scan_range(const AV1& reduced_res, const AV2& R, const Ext& t_ext)
	{
		using namespace concurrency;
		using T = typename AV2::value_type;

		static const int max_block_sz = t_ext.tile_dim0 * 8; // 8 elems to be serially reduced per lane.

		const auto thread_cnt = t_ext.size() / t_ext.tile_dim0;
		const auto work_per_thread = R.extent.size() / thread_cnt + ((R.extent.size() % thread_cnt) ? 1 : 0);

		parallel_for_each(t_ext, [=](const tiled_index<t_ext.tile_dim0>& tidx) restrict(amp) {
			tile_static T t_l_m[max_block_sz + 2];
			if (tidx.tile_origin == tidx.global) t_l_m[0] = reduced_res[tidx.tile];

			auto t_f = begin(R) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + min<int>(work_per_thread, max(0, end(R) - t_f));
			while (t_f < t_l) {
				auto b_l = t_f + min(int (max_block_sz), max(0, t_l - t_f));
				const auto block_sz = b_l - t_f;

				copy(t_f + tidx.local[0], b_l, t_l_m + tidx.local[0] + 1, tidx.tile_dim0);
				fill(t_l_m + block_sz + tidx.local[0] + 1, t_l_m + block_sz + 2, T(), tidx.tile_dim0);

				tidx.barrier.wait_with_tile_static_memory_fence();

				scan_block(t_l_m, t_l_m + block_sz + 2, tidx);

				tidx.barrier.wait_with_tile_static_memory_fence();

				copy(t_l_m + tidx.local[0] + 1, t_l_m + block_sz + 1, t_f + tidx.local[0], tidx.tile_dim0);
				if (tidx.tile_origin == tidx.global) t_l_m[0] = t_l_m[(b_l - t_f) + 1];

				t_f += max_block_sz;
			}
		});
	}

	template<typename I>
	I scan(I f, I l) restrict(cpu)
	{
		if (f >= l) return (l); // Empty range passed, probably an error.

		static const unsigned int thread_cnt = 1792u;
		static const unsigned int SIMD_w = 256u;
		
		using namespace concurrency;
		using T = std::iterator_traits<I>::value_type;
		
		array_view<T, 1> R(std::distance(f, l), &*f);
		array_view<T, 1> reduce_results(thread_cnt);

		reduce_blocks(R.reinterpret_as<const T>(), reduce_results, extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>());

		scan_reduced_blocks<thread_cnt>(reduce_results, extent<1>(SIMD_w).tile<SIMD_w>());

		scan_range(reduce_results.reinterpret_as<const T>(), R, extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>());

		return (--l);
	}
}	   // Namespace experimental.
#endif // _AMP_SCAN_HPP_BUMPTZI
