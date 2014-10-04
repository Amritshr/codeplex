// Remove is just a special case of partition, so the algorithmic approach is simi-
// lar. Note that this is not very optimised, as we are over-allocating the temporary
// buffer. We also provide a(n overloaded) version for restrict(amp) contexts.
#pragma once
#ifndef _AMP_REMOVE_HPP_BUMPTZI
#define _AMP_REMOVE_HPP_BUMPTZI

#include "AMP_accumulate.hpp"
#include "AMP_copy.hpp"
#include "AMP_fill.hpp"
#include "AMP_helpers.hpp"
#include "AMP_scan.hpp"

#include <algorithm>
#include <amp.h>
#include <iterator>

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

	template<typename I, typename T>
	I remove(I f, I l, const T& val, unsigned int off = 1u) restrict(amp)
	{
		auto it = f;
		
		while (it < l) {
			if (*it != val) {
				*f = *it;
				f += off;
			}
			it += off;
		}

		return (f);
	}

	template<typename AV, typename T>
	unsigned int remove(const AV& r, const T& val)
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, r.extent.size() / SIMD_w);
		const auto work_per_thread = r.extent.size() / thread_cnt;

		const array_view<unsigned int> valid_cnt(1);
		valid_cnt[{0}] = 0u;

		const array_view<Container_value_type<AV>> tmp(r.extent.size());
		const auto is_valid = [=](const Container_value_type<AV>& v) restrict(amp) { return (v != val); };

		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			const auto t_f = cbegin(r) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + ((tidx.tile[0] != (thread_cnt - 1)) ? work_per_thread : (cend(r) - t_f));

			tile_static unsigned int valid[tidx.tile_dim0 + 1];
			fill(valid + tidx.local[0], valid + tidx.tile_dim0, count_if(t_f + tidx.local[0], t_l, is_valid, tidx.tile_dim0), tidx.tile_dim0);
			if (tidx.tile_origin == tidx.global) valid[tidx.tile_dim0] = 0u;

			tidx.barrier.wait_with_tile_static_memory_fence();

			const auto cnt = scan_block(valid, valid + tidx.tile_dim0 + 1, tidx);

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (tidx.tile_origin == tidx.global) valid[tidx.tile_dim0] = atomic_fetch_add(&valid_cnt[{0}], valid[tidx.tile_dim0]);

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (cnt) copy_if(t_f + tidx.local[0], t_l, begin(tmp) + valid[tidx.tile_dim0] + valid[tidx.local[0]], is_valid, tidx.tile_dim0);
		});

		if (valid_cnt[{0}]) experimental::copy(tmp.reinterpret_as<const Container_value_type<AV>>().section(0, valid_cnt[{0}]), r.section(0, valid_cnt[{0}]));

		return (valid_cnt[{0}]);
	}

	template<typename I, typename T>
	I remove(I f, I l, const T& val)
	{
		if (f >= l) return (l);

		return (f + remove(concurrency::array_view<Iterator_value_type<I>>(std::distance(f, l), &*f), val));
	}
}	   // Namespace experimental.
#endif // _AMP_REMOVE_HPP_BUMPTZI