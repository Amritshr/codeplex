// We implement inner_product as a succesion of in-thread reductions using multiplies<T>
// followed by an across thread reduction using plus<T>. A version for restrict(amp)
// contexts is also provided.
#pragma once
#ifndef _AMP_INNER_PRODUCT_HPP_BUMPTZI
#define _AMP_INNER_PRODUCT_HPP_BUMPTZI

#include "AMP_accumulate.hpp"
#include "AMP_helpers.hpp"

#include <algorithm>
#include <amp.h>
#include <iterator>

namespace experimental
{
	template<typename I0, typename I1, typename T>
	T inner_product(I0 f_0, I0 l_0, I1 f_1, T val, unsigned int off = 1u) restrict(amp)
	{
		while (f_0 < l_0) {
			val += *f_0 * *f_1;
			f_0 += off;
			f_1 += off;
		}

		return (val);
	}

	template<typename AV0, typename AV1, typename T>
	T inner_product(const AV0& r_0, const AV1& r_1, T val)
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, r_0.extent.size() / SIMD_w);
		const auto work_per_thread = r_0.extent.size() / thread_cnt;

		const array_view<T, 1> partial_sums(thread_cnt);

		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			const auto t_f_0 = cbegin(r_0) + tidx.tile[0] * work_per_thread;
			const auto t_l_0 = t_f_0 + ((tidx.tile[0] != (thread_cnt - 1)) ? work_per_thread : (cend(r_0) - t_f_0));
			const auto t_f_1 = cbegin(r_1) + tidx.tile[0] * work_per_thread;

			tile_static T t_l_m[tidx.tile_dim0];
			t_l_m[tidx.local[0]] = inner_product(t_f_0 + tidx.local[0], t_l_0, t_f_1, T(), tidx.tile_dim0);

			tidx.barrier.wait_with_tile_static_memory_fence();

			reduce(t_l_m, t_l_m + tidx.tile_dim0, tidx);

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (tidx.tile_origin == tidx.global) partial_sums[tidx.tile] = t_l_m[0];
		});

		//	return (std::accumulate(cbegin(partial_sums.reinterpret_as<const T>()), cend(partial_sums.reinterpret_as<const T>()), val));

		return (accumulate(partial_sums.reinterpret_as<const T>(), val));
	}
	template<typename I0, typename I1, typename T>
	T inner_product(I0 f_0, I0 l_0, I1 f_1, T val)
	{
		using namespace concurrency;

		if (f_0 >= l_0) return (val);

		return (inner_product(array_view<const Iterator_value_type<I0>, 1>(std::distance(f_0, l_0), &*f_0), array_view<const Iterator_value_type<I1>, 1>(std::distance(f_0, l_0), &*f_1), val));
	}

	template<typename I0, typename I1, typename T, typename Op0, typename Op1>
	T inner_product(I0 f_0, I0 l_0, I1 f_1, T val, Op0 sum_op, Op1 prod_op)
	{
		// This will be possibly tricky to do without acquiring extra information from the caller
		// i.e. the identity element for the sum_op.
	}
}	   // Namespace experimental.
#endif // _AMP_INNER_PRODUCT_HPP_BUMPTZI