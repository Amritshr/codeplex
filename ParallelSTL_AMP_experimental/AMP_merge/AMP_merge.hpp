// This is an implementation of fast, balanced, sync-free(ish) parallel merge, based
// on Siebert, C. & Traff, J. L. (2013) "Perfectly load-balanced, optimal, stable,
// parallel merge". It also does a first iteration at solving the custom comparator
// and restrict(amp) conundrum.

#pragma once
#ifndef _AMP_MERGE_HPP_BUMPTZI
#define _AMP_MERGE_HPP_BUMPTZI

#include "AMP_copy.hpp"
#include "AMP_functional.hpp"
#include "AMP_helpers.hpp"

#include <amp.h>
#include <amp_math.h>
#include <amp_short_vectors.h>
#include <algorithm>
#include <iterator>

namespace experimental
{
	template<typename I1, typename I2>
	concurrency::graphics::uint_2 co_rank(I1 f_1, I1 l_1, I2 f_2, I2 l_2, unsigned int idx) restrict(cpu, amp)
	{
		using namespace concurrency::fast_math;
		using Co_ranks = concurrency::graphics::uint_2;

		const unsigned int A_sz = l_1 - f_1;
		const unsigned int B_sz = l_2 - f_2;

		auto j = min(idx, A_sz);
		auto k = idx - j;

		auto j_low = idx < B_sz ? 0u : (idx - B_sz);
		auto k_low = k;

		while (true) {
			const auto case_0 = j && (k < B_sz) && (*(f_2 + k) < *(f_1 + j - 1));
			const auto case_1 = k && (j < A_sz) && (*(f_1 + j) <= *(f_2 + k - 1));

			if (!case_0 && !case_1) return (Co_ranks(j, k));

			if (case_0) { 
				const auto delta = static_cast<unsigned int>(ceil((j - j_low) / 2.f));
				k_low = k;
				j -= delta;
				k += delta;
			}
			if (case_1) {
				const auto delta = static_cast<unsigned int>(ceil((k - k_low) / 2.f));
				j_low = j;
				j += delta;
				k -= delta;
			}
		}

		return (Co_ranks(j, k));
	}

	template<typename I1, typename I2, typename O>
	O merge_per_lane(I1 f_1, I1 l_1, I2 f_2, I2 l_2, O f_o) restrict(amp)
	{
		while ((f_1 != l_1) && (f_2 != l_2)) {
			*f_o++ = (*f_1 < *f_2) ? *f_1++ : *f_2++;
		}

		copy(f_1, l_1, f_o);
		copy(f_2, l_2, f_o);

		return (f_o);
	}

	template<typename I1, typename I2, typename O, typename Idx>
	O merge_per_thread(I1 f_1, I1 l_1, I2 f_2, I2 l_2, O f_o, const Idx& tidx) restrict(amp)
	{
		const auto block_sz = ((l_1 - f_1) + (l_2 - f_2)) / tidx.tile_dim0;

		using Co_ranks = decltype(co_rank(f_1, l_1, f_2, l_2, 0u));

		tile_static Co_ranks co_ranks[tidx.tile_dim0 + 1];
		
		co_ranks[tidx.local[0]] = co_rank(f_1, l_1, f_2, l_2, tidx.local[0] * block_sz);
		if (tidx.tile_origin == tidx.global) co_ranks[tidx.tile_dim0] = Co_ranks(l_1 - f_1, l_2 - f_2);

		tidx.barrier.wait_with_tile_static_memory_fence();

		merge_per_lane(f_1 + co_ranks[tidx.local[0]].x, f_1 + co_ranks[tidx.local[0] + 1].x, f_2 + co_ranks[tidx.local[0]].y, f_2 + co_ranks[tidx.local[0] + 1].y, f_o + tidx.local[0] * block_sz);

		return (f_o);
	}

	template<typename AV0, typename AV1, typename AV3>
	void merge(const AV0& in_0, const AV1& in_1, const AV3& out)
	{
		using namespace concurrency;

		static constexpr unsigned int max_thread_cnt = 65535u;
		static constexpr unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, out.extent.size() / SIMD_w);
		const auto work_per_thread = out.extent.size() / thread_cnt;

		out.discard_data();
		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			using Co_ranks = decltype(co_rank(cbegin(in_0), cend(in_0), cbegin(in_1), cend(in_1), 0u));

			tile_static Co_ranks c_r_0;
			tile_static Co_ranks c_r_1;

			if (tidx.tile_origin == tidx.global) {
				const auto block_0 = tidx.tile[0] * work_per_thread;
				const auto block_1 = block_0 + (((tidx.tile[0] != (thread_cnt - 1)) ? work_per_thread : (out.extent.size() - block_0)));

				c_r_0 = co_rank(cbegin(in_0), cend(in_0), cbegin(in_1), cend(in_1), block_0);
				c_r_1 = co_rank(cbegin(in_0), cend(in_0), cbegin(in_1), cend(in_1), block_1);
			}

			tidx.barrier.wait_with_tile_static_memory_fence();

			merge_per_thread(cbegin(in_0) + c_r_0.x, cbegin(in_0) + c_r_1.x, cbegin(in_1) + c_r_0.y, cbegin(in_1) + c_r_1.y, begin(out) + tidx.tile[0] * work_per_thread, tidx);
		});


	}
	template<typename I1, typename I2, typename O>
	O merge(I1 f_1, I1 l_1, I2 f_2, I2 l_2, O f_o)
	{
		

		if ((f_1 == l_1) && (f_2 == l_2)) return (f_o); // Nothing to do, both ranges are empty.
		
		const array_view<const T1, 1> A(std::distance(f_1, l_1), &*f_1);
		const array_view<const T2, 1> B(std::distance(f_2, l_2), &*f_2);
		const array_view<T3, 1> C(A.extent + B.extent, &*f_o);

		return (f_o);
	}

	template<typename I1, typename I2, typename C>
	concurrency::graphics::uint_2 co_rank(I1 f_1, I1 l_1, I2 f_2, I2 l_2, unsigned int idx, C cmp) restrict(cpu, amp)
	{
		using namespace concurrency::fast_math;
		using Co_ranks = concurrency::graphics::uint_2;

		const unsigned int A_sz = l_1 - f_1;
		const unsigned int B_sz = l_2 - f_2;

		auto j = min(idx, A_sz);
		auto k = idx - j;

		auto j_low = idx < B_sz ? 0u : (idx - B_sz);
		auto k_low = k;

		while (true) {
			const auto case_0 = j && (k < B_sz) && (cmp(*(f_2 + k), *(f_1 + j - 1)));
			const auto case_1 = k && (j < A_sz) && (cmp(*(f_1 + j), *(f_2 + k - 1)) || (*(f_1 + j) == *(f_2 + k - 1)));

			if (!case_0 && !case_1) return (Co_ranks(j, k));

			if (case_0) {
				const auto delta = static_cast<unsigned int>(ceil((j - j_low) / 2.f));
				k_low = k;
				j -= delta;
				k += delta;
			}
			if (case_1) {
				const auto delta = static_cast<unsigned int>(ceil((k - k_low) / 2.f));
				j_low = j;
				j += delta;
				k -= delta;
			}
		}

		return (Co_ranks(j, k));
	}

	template<typename I1, typename I2, typename O, typename C>
	inline O merge(I1 f_1, I1 l_1, I2 f_2, I2 l_2, O f_o, C cmp) restrict(amp)
	{
		while ((f_1 != l_1) && (f_2 != l_2)) {
			*f_o++ = cmp(*f_1, *f_2) ? *f_1++ : *f_2++;
		}

		copy(f_1, l_1, f_o);
		copy(f_2, l_2, f_o);

		return (f_o);
	}

	template<typename I1, typename I2, typename O, typename Idx, typename C>
	O merge_per_thread(I1 f_1, I1 l_1, I2 f_2, I2 l_2, O f_o, const Idx& tidx, C cmp) restrict(amp)
	{
		const auto block_sz = ((l_1 - f_1) + (l_2 - f_2)) / tidx.tile_dim0;

		using Co_ranks = decltype(co_rank(f_1, l_1, f_2, l_2, 0u));

		tile_static Co_ranks co_ranks[tidx.tile_dim0 + 1];

		co_ranks[tidx.local[0]] = co_rank(f_1, l_1, f_2, l_2, tidx.local[0] * block_sz, cmp);
		if (tidx.tile_origin == tidx.global) co_ranks[tidx.tile_dim0] = Co_ranks(l_1 - f_1, l_2 - f_2);

		tidx.barrier.wait_with_tile_static_memory_fence();

		merge(f_1 + co_ranks[tidx.local[0]].x, f_1 + co_ranks[tidx.local[0] + 1].x, f_2 + co_ranks[tidx.local[0]].y, f_2 + co_ranks[tidx.local[0] + 1].y, f_o + tidx.local[0] * block_sz, cmp);

		return (f_o);
	}

	template<typename AV0, typename AV1, typename AV2, typename C>
	void merge(const AV0& in_0, const AV1& in_1, const AV2& out, C cmp)
	{
		using namespace concurrency;

		static_assert(std::is_arithmetic<typename std::common_type<Container_value_type<AV0>, Container_value_type<AV1>>::type>::value, "Decorating the comparator with restrict(amp) is required");

		static constexpr unsigned int max_thread_cnt = 65535u;
		static constexpr unsigned int SIMD_w = 256u;

		const auto thread_cnt = std::min(max_thread_cnt, out.extent.size() / SIMD_w);
		const auto work_per_thread = out.extent.size() / thread_cnt;

		out.discard_data();
		parallel_for_each(extent<1>(thread_cnt * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			using Co_ranks = decltype(co_rank(cbegin(in_0), cend(in_0), cbegin(in_1), cend(in_1), 0u));

			tile_static Co_ranks c_r_0;
			tile_static Co_ranks c_r_1;

			if (tidx.tile_origin == tidx.global) {
				const auto block_0 = tidx.tile[0] * work_per_thread;
				const auto block_1 = block_0 + (((tidx.tile[0] != (thread_cnt - 1)) ? work_per_thread : (out.extent.size() - block_0)));

				c_r_0 = co_rank(cbegin(in_0), cend(in_0), cbegin(in_1), cend(in_1), block_0, less<>());
				c_r_1 = co_rank(cbegin(in_0), cend(in_0), cbegin(in_1), cend(in_1), block_1, less<>());
			}

			tidx.barrier.wait_with_tile_static_memory_fence();

			merge_per_thread(cbegin(in_0) + c_r_0.x, cbegin(in_0) + c_r_1.x, cbegin(in_1) + c_r_0.y, cbegin(in_1) + c_r_1.y, begin(out) + tidx.tile[0] * work_per_thread, tidx, less<>());
		});
	}

	template<typename I0, typename I1, typename O, typename Cmp>
	O merge(I0 f_1, I0 l_1, I1 f_2, I1 l_2, O f_o, Cmp cmp)
	{
		if ((f_1 >= l_1) && (f_2 >= l_2)) return (f_o); // Nothing to do, both ranges are empty.

		static_assert(std::is_arithmetic<typename std::common_type<Iterator_value_type<I0>, Iterator_value_type<I1>>::type>::value, "Decorating the comparator with restrict(amp) is required");

		const concurrency::array_view<const Iterator_value_type<I0>, 1> A(std::distance(f_1, l_1), &*f_1);
		const concurrency::array_view<const Iterator_value_type<I1>, 1> B(std::distance(f_2, l_2), &*f_2);
		const concurrency::array_view<Iterator_value_type<O>, 1> C(A.extent + B.extent, &*f_o);

		merge(A, B, C, less<>());

		return (f_o);
	}
}	   // Namespace experimental.
#endif // _AMP_MERGE_HPP_BUMPTZI