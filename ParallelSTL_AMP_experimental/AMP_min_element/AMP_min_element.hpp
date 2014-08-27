// This includes a straightforward implementation of min_element, in which each
// lane finds the min_element for a subset, after which the min_element per thread
// is obtained via reduction across all lanes, with the global min_element being
// the result of reducing across all threads. We also provide a(n overloaded) 
// version for restrict(amp) contexts.
#pragma once
#ifndef _AMP_MIN_ELEMENT_HPP_BUMPTZI
#define _AMP_MIN_ELEMENT_HPP_BUMPTZI

#include "AMP_accumulate.hpp"
#include "AMP_functional.hpp"
#include "AMP_helpers.hpp"

#include <algorithm>
#include <amp.h>
#include <iterator>
#include <utility>

namespace experimental
{
	template<typename I>
	I min_element(I f, I l, unsigned int off = 1u) restrict(amp)
	{
		if (f >= l) return(l); // Empty range passed, probably an error.

		auto m = f;
		f += off;

		while (f < l) {
			if (*f < *m) m = f;
			f += off;
		}

		return (m);
	}

	template<typename T, typename C>
	struct Extreme_val {
		Extreme_val() = default;
		Extreme_val(const T& val, unsigned int idx) restrict(cpu, amp) : v(val), i(idx) {};
		Extreme_val(const Extreme_val& y) restrict(cpu, amp) : v(y.v), i(y.i) {};

		bool operator<(const Extreme_val& r) const restrict(cpu, amp) { return (C()(v, r.v)); };

		T v;
		unsigned int i;
	};

	template<typename AV>
	unsigned int min_element(const AV& r)
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, r.extent.size() / SIMD_w);
		const auto work_per_thread = r.extent.size() / thread_cnt;

		using Minimum = Extreme_val<typename std::remove_const<Container_value_type<AV>>::type, less<>>;

		const array_view<Minimum> min_idx(thread_cnt);

		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			const auto t_f = cbegin(r) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + ((tidx.tile[0] != (thread_cnt - 1)) ? work_per_thread : (cend(r) - t_f));

			const auto min_it = min_element(t_f + tidx.local[0], t_l, tidx.tile_dim0);

			tile_static Minimum t_l_m[tidx.tile_dim0];
			t_l_m[tidx.local[0]] = Minimum(*min_it, min_it - cbegin(r));

			tidx.barrier.wait_with_tile_static_memory_fence();
			
			reduce(t_l_m, t_l_m + tidx.tile_dim0, tidx, [](const Minimum& x, const Minimum& y) { return (min(x, y)); });

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (tidx.tile_origin == tidx.global) min_idx[tidx.tile] = t_l_m[0];
		});

		return (std::accumulate(cbegin(min_idx), cend(min_idx), min_idx[{0}], min<Minimum>).i);
	}

	template<typename I>
	I min_element(I f, I l)
	{
		if (f >= l) return (l);

		const concurrency::array_view<const Iterator_value_type<I>> r(std::distance(f, l), &*f);

		return(f + (min_element(r) - cbegin(r)));
	}

	template<typename I, typename C>
	I min_element(I f, I l, unsigned int off = 1u, C cmp = C()) restrict(amp)
	{
		if (f >= l) return(l); // Empty range passed, probably an error.

		auto m = f;
		f += off;

		while (f < l) {
			if (cmp(*f, *m)) m = f;
			f += off;
		}

		return (m);
	}

	template<typename AV, typename C>
	auto min_element(const AV& r, C cmp) -> decltype(cbegin(r))
	{
		using namespace concurrency;

		static const unsigned int max_thread_cnt = 65535u;
		static const unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, r.extent.size() / SIMD_w);
		const auto work_per_thread = r.extent.size() / thread_cnt;

		using Minimum = Extreme_val<typename std::remove_const<Container_value_type<AV>>::type, C>;

		const array_view<Minimum> min_idx(thread_cnt);

		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			const auto t_f = cbegin(r) + tidx.tile[0] * work_per_thread;
			const auto t_l = t_f + ((tidx.tile[0] != (thread_cnt - 1)) ? work_per_thread : (cend(r) - t_f));

			const auto min_it = min_element(t_f + tidx.local[0], t_l, cmp, tidx.tile_dim0);

			tile_static Minimum t_l_m[tidx.tile_dim0];
			t_l_m[tidx.local[0]] = Minimum(*min_it, min_it - cbegin(r));

			tidx.barrier.wait_with_tile_static_memory_fence();

			reduce(t_l_m, t_l_m + tidx.tile_dim0, tidx, [](const Minimum& x, const Minimum& y) { return (min(x, y)); });

			tidx.barrier.wait_with_tile_static_memory_fence();

			if (tidx.tile_origin == tidx.global) min_idx[tidx.tile] = t_l_m[0];
		});

		return (cbegin(r) + std::accumulate(cbegin(min_idx), cend(min_idx), min_idx[{0}], min<Minimum>).i);
	}

	template<typename I, typename C>
	I min_element(I f, I l, C cmp)
	{
		if (f >= l) return (l);

		const concurrency::array_view<const Iterator_value_type<I>> r(std::distance(f, l), &*f);

		return(f + (min_element(r, cmp) - cbegin(r)));
	}
}	   // Namespace experimental.
#endif // _AMP_MIN_ELEMENT_HPP_BUMPTZI