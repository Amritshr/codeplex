// An early experimental version of remove_if implemented in C++ AMP. Draws from the rather neat approach to partition propsed
// in a different context in Cederman, D. & Tsigas, P. (2009) "GPU-Quicksort: A Practical Quicksort Algorithm for Graphics Processors"
#pragma once
#ifndef _AMP_REMOVE_IF_HPP_BUMPTZI
#define _AMP_REMOVE_IF_HPP_BUMPTZI

#include "AMP_scan.hpp"

#include <algorithm>
#include <amp.h>
#include <amp_short_vectors.h>
#include <iterator>

namespace experimental
{
	// Note that this implementation is O(N) storage, because the in-place version that we provide elsewhere cannot be guaranteed
	// to execute correctly under current API specifications (be it C++ AMP or OpenCL).
	template<typename I, typename O, typename Idx, typename P>
	unsigned int compact_SIMD(I i_f, I i_l, O o_f, const Idx& tidx, P p) restrict(cpu, amp)
	{
		const auto p_val = ((i_f + tidx.local[0]) < i_l) ? (!p(*(i_f + tidx.local[0]))) : 0u; 

		tile_static unsigned int out_idx[tidx.tile_dim0 + 1];
		fill(out_idx + tidx.local[0], out_idx + (i_l - i_f), !p(*(i_f + tidx.local[0])), tidx.tile_dim0);
		fill(out_idx + (i_l - i_f) + tidx.local[0], out_idx + tidx.tile_dim0 + 1, 0u, tidx.tile_dim0);

		tidx.barrier.wait_with_tile_static_memory_fence();
		
		experimental::scan_block(out_idx, out_idx + tidx.tile_dim0 + 1, tidx);

		tidx.barrier.wait_with_tile_static_memory_fence();

		if (p_val) {
			*(o_f + out_idx[tidx.local[0]]) = *(i_f + tidx.local[0]);
		}

		return (out_idx[tidx.tile_dim0]);
	}


	template<typename AV1, typename AV2, typename Ext, typename P>
	void compute_offsets(const AV1& in, const AV2& offsets, const Ext& t_ext, unsigned int block_cnt, unsigned int el_per_block, P p)
	{
		using namespace concurrency;
		
	//	const auto block_cnt = in.extent.size() / el_per_block + ((in.extent.size() % el_per_block) ? 1 : 0); 

		parallel_for_each(t_ext, [=](const tiled_index<t_ext.tile_dim0>& tidx) restrict(amp) {
			auto block = tidx.tile[0];
			auto cnt = 0u;

			while (block < block_cnt) {	
				auto f = cbegin(in) + block * el_per_block + tidx.local[0];
				const auto l = f + min<unsigned int>(el_per_block, max(0, cend(in) - f));
				while (f < l) {
					cnt += !p(*f);

					f += tidx.tile_dim0;
				}

				tile_static unsigned int t_l_m[tidx.tile_dim0];
				t_l_m[tidx.local[0]] = cnt;

				tidx.barrier.wait_with_tile_static_memory_fence();

				experimental::reduce(t_l_m, t_l_m + tidx.tile_dim0, tidx);

				if (tidx.tile_origin == tidx.global) offsets[block] = t_l_m[0];

				block += t_ext.size() / tidx.tile_dim0;
			}

			if (tidx.global[0] == 0) offsets[block_cnt] = 0u;
		});
	}

	template<typename AV1, typename AV2, typename AV3, typename Ext, typename P>
	void write_output(const AV1& in, const AV2& offsets, const AV3& out, const Ext& t_ext, unsigned int block_cnt, int el_per_block, P p)
	{
		using namespace concurrency;

		parallel_for_each(t_ext, [=](const tiled_index<t_ext.tile_dim0>& tidx) restrict(amp) {
			using T = typename AV3::value_type;

			tile_static T in_t_l_m[tidx.tile_dim0];
			tile_static T out_t_l_m[tidx.tile_dim0];

			auto block = tidx.tile;
			while (block[0] < block_cnt) {
				auto off = offsets[block];
				auto b_off = 0;
				while (b_off < el_per_block) {
					const auto i_f = cbegin(in) + block[0] * el_per_block + b_off;
					const auto i_l = i_f + min(tidx.tile_dim0 + 0, max(0, el_per_block - b_off)); // Quirky add 0 in order to work around current C++ AMP limitation in dealing with static vars.

					copy(i_f + tidx.local[0], i_l, in_t_l_m + tidx.local[0], tidx.tile_dim0);

					tidx.barrier.wait_with_tile_static_memory_fence();

					auto num_true = compact_SIMD(in_t_l_m, in_t_l_m + (i_l - i_f), out_t_l_m, tidx, p);

					tidx.barrier.wait_with_tile_static_memory_fence();

					copy(out_t_l_m + tidx.local[0], out_t_l_m + num_true, begin(out) + off + tidx.local[0], tidx.tile_dim0);

					off += num_true;
					b_off += tidx.tile_dim0;
				}

				block += t_ext.size() / t_ext.tile_dim0;
			}
		});
	}
	template<typename I, typename T>
	I remove(I f, I l, const T& v)
	{

	}

	template<typename I, typename P>
	I remove_if(I f, I l, P p)
	{
		if (f >= l) return (l); // Empty range, nothing to do.

		using namespace concurrency;
		using T = typename std::iterator_traits<I>::value_type;

		array_view<const T, 1> in(std::distance(f, l), &*f);
		array_view<T, 1> out(in.extent);

		static constexpr unsigned int max_threads = 65535u;
		static constexpr unsigned int SIMD_w = 256u;
		static constexpr unsigned int el_per_lane = 16u;
		static constexpr unsigned int el_per_block = SIMD_w * el_per_lane;

		const auto block_cnt = in.extent.size() / el_per_block + ((in.extent.size() % el_per_block) ? 1u : 0u);
		const auto thread_cnt = std::min(max_threads, block_cnt);
		const auto exec_domain = extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>();

		array_view<unsigned int, 1> block_offsets(block_cnt + 1);

		compute_offsets(in, block_offsets, exec_domain, block_cnt, el_per_block, p);

		serial_scan(begin(block_offsets), end(block_offsets));//experimental::scan(block_offsets);

		write_output(in, block_offsets.reinterpret_as<const unsigned int>(), out, exec_domain, block_cnt, el_per_block, p);

		concurrency::copy(out.section(0, block_offsets[block_cnt]), f);
		
		return (f + block_offsets[block_cnt]);
	}

	template<typename AV, typename P>
	unsigned int remove_if(const AV& R, P p) // Overload taking an array_view, so as to save some copying.
	{
		using namespace concurrency;
		using T = typename AV::value_type;

		array_view<const T, 1> in(R);
		array_view<T, 1> out(R.extent);

		static constexpr unsigned int max_threads = 65535u;
		static constexpr unsigned int SIMD_w = 256u;
		static constexpr unsigned int el_per_lane = 16u;
		static constexpr unsigned int el_per_block = SIMD_w * el_per_lane;

		const auto block_cnt = R.extent.size() / el_per_block + ((R.extent.size() % el_per_block) ? 1u : 0u);
		const auto thread_cnt = std::min(max_threads, block_cnt);
		const auto exec_domain = extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>();

		array_view<unsigned int, 1> block_offsets(block_cnt + 1);

		compute_offsets(in, block_offsets, exec_domain, block_cnt, el_per_block, p);

	/*	serial_scan(begin(block_offsets), end(block_offsets));/*/experimental::scan(block_offsets);

		write_output(in, block_offsets.reinterpret_as<const unsigned int>(), out, exec_domain, block_cnt, el_per_block, p);
		
		parallel_for_each(extent<1>(block_offsets(block_cnt)), [=](const index<1>& idx) restrict(amp) { R[idx] = out[idx]; }); // Copy like this to stay on accelerator.

		return (block_offsets(block_cnt));
	}
}	   // Namespace experimental.
#endif // _AMP_REMOVE_IF_HPP_BUMPTZI