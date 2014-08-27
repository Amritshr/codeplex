// This contains two implementations of partition, one based on the work presented
// in Cederman, D. & Tsigas, P. (2009) "GPU-Quicksort: A practical Quicksort Algorithm 
// for graphics processors" http://dl.acm.org/citation.cfm?doid=1498698.1564500, and 
// another based on Billeter, M., Olsson, O. & Assarsson, U. (2009) "Efficient stream
// compaction on wide SIMD many-core architectures" http://dl.acm.org/citation.cfm?id=1572795
// Note that we could efficiently do partition in-place by a straightforward extension of the 
// algorithm, but we cannot guarantee deadlock-free execution with current APIs. We also 
// provide a(n overloaded) version for restrict(amp) contexts.
#pragma once
#ifndef _AMP_PARTITION_HPP_BUMPTZI
#define _AMP_PARTITION_HPP_BUMPTZI

#include "AMP_copy.hpp"
#include "AMP_fill.hpp"
#include "AMP_helpers.hpp"
#include "AMP_scan.hpp"

#include <algorithm>
#include <amp.h>
#include <utility>

namespace experimental
{
	template<typename I, typename P>
	unsigned int count_if(I f, I l, const P& p, unsigned int off = 1u) restrict(amp)
	{
		auto cnt = 0u;
		
		while (f < l) {
			if (p(*f)) ++cnt;
			f += off;
		}

		return (cnt);
	}

	template<typename I, typename O, typename P>
	O copy_if(I i_f, I i_l, O o_f, const P& p, unsigned int i_off = 1u, unsigned int o_off = 1u) restrict(amp)
	{
		while (i_f < i_l) {
			if (p(*i_f)) {
				*o_f = *i_f;
				o_f += o_off;
			}
			i_f += i_off;
			
		}

		return(o_f);
	}

/*	template<typename AV, typename P>
	concurrency::array_view<concurrency::index<2>> compute_offsets(const AV& r, P p)
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, r.extent.size() / SIMD_w);
		const auto work_per_thread = r.extent.size() / thread_cnt;

		const array_view<index<2>> offsets(thread_cnt + 1);

		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			const auto t_f = cbegin(r) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + ((tidx.tile[0] != (thread_cnt - 1u) ? work_per_thread : (cend(r) - t_f)));

			tile_static unsigned int t_l_m[tidx.tile_dim0];
			t_l_m[tidx.local[0]] = count_if(t_f + tidx.local[0], t_l, p, tidx.tile_dim0);

			tidx.barrier.wait_with_tile_static_memory_fence();

			reduce(t_l_m, t_l_m + tidx.tile_dim0, tidx);

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (tidx.tile_origin == tidx.global) offsets[tidx.tile] = index<2>(t_l_m[0], (t_l - t_f) - t_l_m[0]);
			if (tidx.tile[0] == (thread_cnt - 1) && tidx.tile_origin == tidx.global) offsets[thread_cnt] = index<2>(0, 0);
		});

		return (offsets);
	}*/

/*	template<typename AV0, typename AV1, typename P>
	concurrency::array_view<Container_value_type<AV0>> partition_into_temp(const AV0& r, const AV1& offsets, P p)
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, r.extent.size() / SIMD_w);
		const auto work_per_thread = r.extent.size() / thread_cnt;

		const array_view<typename std::remove_const<Container_value_type<AV0>>::type> temp(r.extent);

		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			const auto not_p = [=](const Container_value_type<AV0>& v) { return (!p(v)); };

			const auto t_f = cbegin(r) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + ((tidx.tile[0] != (thread_cnt - 1u) ? work_per_thread : (cend(r) - t_f)));

			tile_static index<2> t_l_m[tidx.tile_dim0 + 1];
			t_l_m[tidx.local[0]][0] = count_if(t_f + tidx.local[0], t_l, p, tidx.tile_dim0);
			t_l_m[tidx.local[0]][1] = count_if(t_f + tidx.local[0], t_l, not_p, tidx.tile_dim0);
			if (tidx.tile_origin == tidx.global) t_l_m[tidx.tile_dim0] = index<2>(0, 0);

			tidx.barrier.wait_with_tile_static_memory_fence();

			scan_block(t_l_m, t_l_m + tidx.tile_dim0 + 1, tidx);

			tidx.barrier.wait_with_tile_static_memory_fence();

			const auto o_true = begin(temp) + offsets[tidx.tile][0] + t_l_m[tidx.local[0]][0];
			const auto o_false = begin(temp) + offsets[thread_cnt][0] + offsets[tidx.tile][1] + t_l_m[tidx.local[0]][1];

			if (t_l_m[tidx.tile_dim0][0]) copy_if(t_f + tidx.local[0], t_l, o_true, p, tidx.tile_dim0);
			if (t_l_m[tidx.tile_dim0][1]) copy_if(t_f + tidx.local[0], t_l, o_false, not_p, tidx.tile_dim0);
		});

		return (temp);
	}*/

//	template<typename AV, typename P>
//	unsigned int partition(const AV& r, P p)
//	{
//		const auto offsets = compute_offsets(r.reinterpret_as<const Container_value_type<AV>>(), p);
//	
//		scan(offsets);
		
//		const auto temp = partition_into_temp(r.reinterpret_as<const Container_value_type<AV>>(), offsets.reinterpret_as<const concurrency::index<2>>(), p);

//		r.discard_data();
//		concurrency::parallel_for_each(r.extent, [=](const concurrency::index<1>& idx) restrict(amp) { r[idx] = temp[idx]; });

//		return ((*(end(offsets) - 1))[0]);
//	}

	template<typename AV0, typename AV1, typename P>
	unsigned int partition_into_tmp(const AV0& in, const AV1& out, P p)
	{
		using namespace concurrency;
		
		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, in.extent.size() / SIMD_w);
		const auto work_per_thread = in.extent.size() / thread_cnt;

		const array_view<std::pair<unsigned int, unsigned int>> offsets(1);
		offsets[{0}] = { 0u, in.extent.size() };

		const auto not_p = [=](const Container_value_type<AV0>& v) restrict(amp) { return (!p(v)); };

		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			const auto t_f = cbegin(in) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + ((tidx.tile[0] != (thread_cnt - 1)) ? work_per_thread : (cend(in) - t_f));

			tile_static unsigned int p_true[tidx.tile_dim0 + 1];
			tile_static unsigned int p_false[tidx.tile_dim0 + 1];

			fill(p_true + tidx.local[0], p_true + tidx.tile_dim0, count_if(t_f + tidx.local[0], t_l, p, tidx.tile_dim0), tidx.tile_dim0);
			fill(p_false + tidx.local[0], p_false + tidx.tile_dim0, count_if(t_f + tidx.local[0], t_l, not_p, tidx.tile_dim0), tidx.tile_dim0);

			if (tidx.tile_origin == tidx.global) {
				p_true[tidx.tile_dim0] = 0u;
				p_false[tidx.tile_dim0] = 0u;
			}

			tidx.barrier.wait_with_tile_static_memory_fence();

			const auto true_cnt = scan_block(p_true, p_true + tidx.tile_dim0 + 1, tidx);
			const auto false_cnt = scan_block(p_false, p_false + tidx.tile_dim0 + 1, tidx);

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (tidx.tile_origin == tidx.global) {
				p_true[tidx.tile_dim0] = atomic_fetch_add(&offsets[{0}].first, p_true[tidx.tile_dim0]);
				p_false[tidx.tile_dim0] = atomic_fetch_sub(&offsets[{0}].second, p_false[tidx.tile_dim0]) - p_false[tidx.tile_dim0];
			}

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (true_cnt) copy_if(t_f + tidx.local[0], t_l, begin(out) + p_true[tidx.tile_dim0] + p_true[tidx.local[0]], p, tidx.tile_dim0);
			if (false_cnt) copy_if(t_f + tidx.local[0], t_l, begin(out) + p_false[tidx.tile_dim0] + p_false[tidx.local[0]], not_p, tidx.tile_dim0);
		});

		return (offsets[{0}].first);
	}

	template<typename AV, typename P>
	unsigned int partition(const AV& r, P p)
	{
		concurrency::array_view<Container_value_type<AV>> tmp(r.extent);

		auto true_cnt = partition_into_tmp(r.reinterpret_as<const Container_value_type<AV>>(), tmp, p);

		experimental::copy(tmp.reinterpret_as<const Container_value_type<AV>>(), r);

		return (true_cnt);
	}

	template<typename I, typename P>
	I partition(I f, I l, P p)
	{
		if (f >= l) return (l); // Empty range passed, probably an error.

		const concurrency::array_view<Iterator_value_type<I>> r(std::distance(f, l), &*f);
		return (f + (partition(r, p)));
	}
}	   // Namespace experimental.
#endif // _AMP_PARTITION_HPP_BUMPTZI