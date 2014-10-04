// This revives a pretty old idea first suggested by Knuth and then properly fleshed out
// in Dobosiewicz, W. (1980) "An efficient variation of Bubble Sort". Oldie but goodie.
#pragma once
#ifndef _AMP_DOBOSIEWICZ_SORT_HPP_BUMPTZI
#define _AMP_DOBOSIEWICZ_SORT_HPP_BUMPTZI

#include "AMP_helpers.hpp"

#include <amp.h>
#include <amp_math.h>
#include <iterator>

namespace experimental
{
	template<typename I>
	void insertion_sort(I f, I l) restrict(cpu, amp)
	{
		if (f >= l) {
			return; // Empty range, nothing to do.
		}

		auto i0 = f;
		while (++i0 != l) {
			const auto tmp = *i0;

			auto i1 = i0;
			while ((i1 != f) && (*(i1 - 1) > tmp)) {
				*i1 = *(i1 - 1);
				--i1;
			}
			*i1 = tmp;
		}
	}

	template<typename I>
	bool signalling_insertion_sort(I f, I l) restrict(cpu, amp)
	{
		if (f >= l) {
			return (false); // Empty range, nothing to do.
		}

		bool unsorted = false;
		auto i0 = f;
		while (++i0 != l) {
			const auto tmp = *i0;

			auto i1 = i0;

			while ((i1 != f) && (*(i1 - 1) > tmp)) {
				*i1 = *(i1 - 1);
				--i1;

				unsorted = true;
			}
			*i1 = tmp;
		}

		return (unsorted);
	}

	template<typename T>
	inline bool compare_and_swap(T& x0, T& x1) restrict(amp)
	{
		if (x0 > x1) {
			swap(x0, x1);
			return (true);
		}

		return(false);
	}

	inline unsigned int next_dobosiewicz_gap(unsigned int prev_gap) restrict(cpu, amp)
	{
		// This dark and fearful magic comes from the bowels of diminishing increment sorts. 
		// Also, Sedgewick, R. (1996) "Analysis of Shellsort and Related Algorithms", albeit he merely
		// suggests a ratio that's less than 1.7.

		if ((prev_gap < 17u) && (prev_gap > 13u)) return (11u);
		return (prev_gap * 10u / 15u); 
	}

	template<typename I, typename Idx>
	inline void sort_gap_forward(I f, I l, const Idx& tidx, unsigned int gap) restrict(cpu, amp)
	{
		auto lidx = tidx.local[0];
		while (lidx < gap) {
			auto i0 = f + lidx + gap;
			
			while (i0 < l) {
				compare_and_swap(*(i0 - gap), *i0);

				i0 += gap;
			}

			lidx += tidx.tile_dim0;
		}
	}

	template<typename I, typename Idx>
	inline void sort_gap_backward(I f, I l, const Idx& tidx, unsigned int gap) restrict(cpu, amp)
	{
		auto lidx = tidx.local[0];
		while (lidx < gap) {
			auto i0 = l - lidx - gap - 1;

			while ((i0 - f) > -1) {
				compare_and_swap(*i0, *(i0 + gap));

				i0 -= gap;
			}

			lidx += tidx.tile_dim0;
		}
	}

	template<typename I, typename Idx>
	void dobosiewicz_sort(I f, I l, const Idx& tidx, unsigned int min_gap = 17u) restrict(cpu, amp)
	{
		auto gap = (l - f);
		while (gap > min_gap) {
			gap = next_dobosiewicz_gap(gap);
			sort_gap_forward(f, l, tidx, gap);

			tidx.barrier.wait_with_tile_static_memory_fence();

			gap = next_dobosiewicz_gap(gap);	
			sort_gap_backward(f, l, tidx, gap);

			tidx.barrier.wait_with_tile_static_memory_fence();
		}
	}

	template<typename AV>
	void almost_sort_elems(const AV& R, unsigned int min_gap)
	{
		using namespace concurrency;

		auto gap = next_dobosiewicz_gap(R.extent.size());
		while (gap >= min_gap) {
			parallel_for_each(extent<1>(gap), [=](const index<1>& idx) restrict(amp) {
				auto i0 = begin(R) + idx[0] + gap;
				while (i0 < end(R)) {
					compare_and_swap(*(i0 - gap), *i0);

					i0 += gap;
				}
			});

			gap = next_dobosiewicz_gap(gap);

			parallel_for_each(extent<1>(gap), [=](const index<1>& idx) restrict(amp) {
				auto i0 = end(R) - idx[0] - gap - 1;
				while ((i0 - begin(R)) > -1) {
					compare_and_swap(*i0, *(i0 + gap));

					i0 -= gap;
				}
			});

			gap = next_dobosiewicz_gap(gap);
		}
	}

	template<typename I, typename Idx>
	void odd_even_sort_block(I f, I l, const Idx& tidx) restrict(amp)
	{
		const auto block_sz = (l - f) / tidx.tile_dim0 + ((l - f) % tidx.tile_dim0 ? 1 : 0);

		const auto odd = f + (tidx.local[0] * 2 + 1) * block_sz;
		const auto even = f + tidx.local[0] * 2 * block_sz;

		tile_static bool unsorted;
		do {
			tidx.barrier.wait_with_tile_static_memory_fence();

			unsorted = false;

			const auto odd_l = odd + min(2 * block_sz, max(l - odd, 0));
			const bool swapped_odd = signalling_insertion_sort(odd, odd_l);

			tidx.barrier.wait_with_tile_static_memory_fence();

			const auto even_l = even + min(2 * block_sz, max(l - even, 0));
			const bool swapped_even = signalling_insertion_sort(even, even_l);

			if (swapped_odd || swapped_even) {
				unsorted = true;
			}

			tidx.barrier.wait_with_tile_static_memory_fence();
		} while (unsorted);
	}

	template<unsigned int block_sz, typename AV>
	void sort_blocks(const AV& R, unsigned int min_gap = 17u)
	{
		using namespace concurrency;

		static constexpr unsigned int max_block_cnt = 65535u; // The most annoying DirectX limitation of all timeTM.
		static constexpr unsigned int SIMD_w = 256u;

		const auto block_cnt = R.extent.size() / block_sz + (R.extent.size() % block_sz ? 1u : 0u);
		const auto exec_domain = extent<1>(std::min(block_cnt, max_block_cnt));
		
		parallel_for_each((exec_domain * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			using T = typename AV::value_type;

			tile_static T t_l_m[block_sz];
				
			auto done = 0u;
			while (done < block_cnt) {
				const auto f = cbegin(R) + (tidx.tile[0] + done) * block_sz; 
				const auto l = f + min<unsigned int>(block_sz, max(0, cend(R) - f));
				copy(f + tidx.local[0], l, t_l_m + tidx.local[0], tidx.tile_dim0);

				tidx.barrier.wait_with_tile_static_memory_fence();

				dobosiewicz_sort(t_l_m, t_l_m + (l - f), tidx, min_gap);

				tidx.barrier.wait_with_tile_static_memory_fence();

				odd_even_sort_block(t_l_m, t_l_m + (l - f), tidx);

				copy(t_l_m + tidx.local[0], t_l_m + (l - f), begin(R) + (tidx.tile[0] + done) * block_sz + tidx.local[0], tidx.tile_dim0);

				done += exec_domain.size();
			}
				
		});
	}

	struct Co_ranks {
		Co_ranks(unsigned int j_in, unsigned int k_in) restrict(cpu, amp) : j(j_in), k(k_in) {}
		unsigned int j;
		unsigned int k;
	};
	template<typename I1, typename I2>
	Co_ranks co_rank(I1 f_A, I1 l_A, I2 f_B, I2 l_B, unsigned int idx) restrict(amp)
	{
		const unsigned int A_sz = l_A - f_A;
		const unsigned int B_sz = l_B - f_B;

		auto j = min(idx, A_sz);
		auto k = idx - j;
		
		auto j_low = max(0u, idx - B_sz);
		auto k_low = k;

		while (true) {
			const auto case_0 = j && (k < B_sz) && (*(f_B + k) < *(f_A + j - 1));
			const auto case_1 = k && (j < A_sz) && (*(f_A + j) <= *(f_B + k - 1));

			if (!case_0 && !case_1) return (Co_ranks(j, k));

			const int delta_j = static_cast<int>(concurrency::fast_math::ceil((j - j_low) / 2.f));
			const int delta_k = static_cast<int>(concurrency::fast_math::ceil((k - k_low) / 2.f));
			const int delta = delta_j * case_0 + delta_k * case_1;
			
			k_low = k * case_0 + k_low * case_1;
			j_low = j * case_1 + j_low * case_0;
			
			j += -delta * case_0 + delta * case_1;
			k += delta * case_0 + -delta * case_1;
		}
	}

	template<typename I1, typename I2, typename O,/*1, typename O2, */typename Idx>
	void merge_disjoint_sequences(I1 f_A, I1 l_A, I2 f_B, I2 l_B, O f_C, const Idx& tidx) restrict(amp)/*1 f_C, O2 f_D, const Idx& tidx) restrict(amp)*/
	{
		const auto block_0 = tidx.local[0] * ((l_A - f_A) + (l_B - f_B)) / tidx.tile_dim0;
	//	const auto block_1 = (tidx.local[0] + 1) * ((l_A - f_A) + (l_B - f_B)) / tidx.tile_dim0;

		tile_static Co_ranks co_ranks[tidx.tile_dim0 + 1];
		
		co_ranks[tidx.local[0]] = co_rank(f_A, l_A, f_B, l_B, block_0);
		co_ranks[tidx.tile_dim0] = Co_ranks(l_A - f_A, l_B - f_B);

		tidx.barrier.wait_with_tile_static_memory_fence();
		
		const auto co_rank_0 = co_ranks[tidx.local[0]];//co_rank(f_A, l_A, f_B, l_B, block_0);
		const auto co_rank_1 = co_ranks[tidx.local[0] + 1];//co_rank(f_A, l_A, f_B, l_B, block_1);

	//	tidx.barrier.wait_with_tile_static_memory_fence();

		merge(f_A + co_rank_0.j, f_A + co_rank_1.j, f_B + co_rank_0.k, f_B + co_rank_1.k, f_C + block_0);// ((block_0 < (l_A - f_A)) ? (f_C + block_0) : (f_D + block_0 - (l_A - f_A))));
	}

	template<unsigned int block_sz, typename I1, typename I2, typename Idx>
	void merge_blocks(I1 f_A, I1 l_A, I2 f_B, I2 l_B, const Idx& tidx) restrict(amp)
	{
		using T = typename std::common_type<typename std::iterator_traits<I1>::value_type, typename std::iterator_traits<I2>::value_type>::type;

	//	tile_static T A[block_sz];
	//	tile_static T B[block_sz];
		tile_static T t_l_m[block_sz * 2];
	//	copy(f_A + tidx.local[0], l_A, A + tidx.local[0], tidx.tile_dim0);
	//	copy(f_B + tidx.local[0], l_B, B + tidx.local[0], tidx.tile_dim0);

	//	tidx.barrier.wait_with_tile_static_memory_fence();

		merge_disjoint_sequences(f_A, l_A, f_B, l_B, /*A + (l_A - f_A), B, B + (l_B - f_B),*/ t_l_m, tidx);// t_l_m, tidx);//f_A, l_A, f_B, l_B, t_l_m, tidx);

		tidx.barrier.wait_with_tile_static_memory_fence();

		copy(t_l_m + tidx.local[0], t_l_m + (l_A - f_A), f_A + tidx.local[0], tidx.tile_dim0);
		copy(t_l_m + (l_A - f_A) + tidx.local[0], t_l_m + (l_A - f_A) + (l_B - f_B), f_B + tidx.local[0], tidx.tile_dim0);
	}

	template<unsigned int block_sz, unsigned int SIMD_w, typename AV>
	void sort_range_dobosiewicz_forward(const AV& R, unsigned int gap)
	{
		using namespace concurrency;

		if (!gap) return;

		static constexpr unsigned int max_gap = 65535u;
		const auto exec_domain = extent<1>(std::min(gap, max_gap));

		parallel_for_each((exec_domain * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			auto done = 0u;
			while (done < gap) {
				auto i0 = begin(R) + (tidx.tile[0] + done) * block_sz;
				auto i1 = i0 + gap * block_sz;

				tile_static bool must_swap;
				tile_static bool must_sort;

				while (i1 < end(R)) {
					must_swap = *(i1 + min<unsigned int>(block_sz, end(R) - i1) - 1) < *i0;
					must_sort = *i1 < *(i0 + block_sz - 1);

					tidx.barrier.wait_with_tile_static_memory_fence();

					if (must_swap) {
						swap_ranges(i0 + tidx.local[0], i0 + min<unsigned int>(block_sz, end(R) - i1), i1 + tidx.local[0], tidx.tile_dim0);
					}
					else if (must_sort) {
						merge_blocks<block_sz>(i0, i0 + block_sz, i1, i1 + min<unsigned int>(block_sz, end(R) - i1), tidx);
					}

					tidx.barrier.wait_with_tile_static_memory_fence();

					i0 = i1;
					i1 += gap * block_sz;		
				}

				done += exec_domain.size();
			}		
		});
	}

	template<unsigned int block_sz, unsigned int SIMD_w, typename AV>
	void sort_range_dobosiewicz_backward(const AV& R, unsigned int gap)
	{
		using namespace concurrency;

		if (!gap) return;

		static constexpr unsigned int max_gap = 65535u;
		const auto exec_domain = extent<1>(std::min(gap, max_gap));
		const auto tail_sz = R.extent.size() % block_sz;

		parallel_for_each((exec_domain * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			auto done = 0u;
			while (done < gap) {
				auto i0 = end(R) - (tidx.tile[0] + done) * block_sz - block_sz - tail_sz; // The tail is not associated with full blocks.
				auto i1 = i0 - gap * block_sz;

				tile_static bool must_swap;
				tile_static bool must_sort;

				while ((i1 - begin(R)) > -1) {
					must_swap = *(i0 + block_sz - 1) < *i1;
					must_sort = *i0 < *(i1 + block_sz - 1);

					tidx.barrier.wait_with_tile_static_memory_fence();

					if (must_swap) { // Blocks are unordered.
						swap_ranges(i1 + tidx.local[0], i1 + block_sz, i0 + tidx.local[0], tidx.tile_dim0);
					}
					else if (must_sort) {
						merge_blocks<block_sz>(i1, i1 + block_sz, i0, i0 + block_sz, tidx);
					}

					tidx.barrier.wait_with_tile_static_memory_fence();

					i0 = i1;
					i1 -= gap * block_sz;
				}

				done += exec_domain.size();
			}
			
		});
	}

	template<unsigned int block_sz, typename AV>
	void almost_sort_range(const AV& R, unsigned int min_gap = 17u)
	{
		using namespace concurrency;
		
		static constexpr unsigned int SIMD_w = 256u;

		auto gap = next_dobosiewicz_gap(R.extent.size() / block_sz + ((R.extent.size() % block_sz) ? 1u : 0u));
		while (gap >= min_gap) {
			sort_range_dobosiewicz_forward<block_sz, SIMD_w>(R, gap);

			gap = next_dobosiewicz_gap(gap);

			sort_range_dobosiewicz_backward<block_sz, SIMD_w>(R, gap);

			gap = next_dobosiewicz_gap(gap);
		}
	}

	template<unsigned int block_sz, unsigned int SIMD_w, typename RAV, typename USAV>
	void odd_even_pass(const RAV& R, const USAV& unsorted, bool odd = true)
	{
		using namespace concurrency;

		static constexpr unsigned int max_block_cnt = 65535u;
		
		const auto block_cnt = R.extent.size() / block_sz + ((R.extent.size() % block_sz) ? 1u : 0u);
		const auto exec_domain = extent<1>(std::min(block_cnt / 2u, max_block_cnt));

		parallel_for_each((exec_domain * SIMD_w).tile<SIMD_w>(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			auto done = 0u;
			while (done < block_cnt) {
				const auto i0 = begin(R) + ((tidx.tile[0] + done) * 2 + odd) * block_sz;
				const auto i1 = i0 + block_sz;

				tile_static bool must_swap;
				tile_static bool must_sort;

				if (i1 < end(R)) {
					const auto i1_l = i1 + min<unsigned int>(block_sz, end(R) - i1);

					if (tidx.tile_origin == tidx.global) {
						must_swap = *(i1_l - 1) < *i0;
						must_sort = !must_swap && (*i1 < *(i0 + block_sz - 1));
					}

					tidx.barrier.wait_with_tile_static_memory_fence();

					if (must_swap) {
						swap_ranges(i0 + tidx.local[0], i0 + (i1_l - i1), i1 + tidx.local[0], tidx.tile_dim0);
					}
					if (must_sort) {
						merge_blocks<block_sz>(i0, i0 + block_sz, i1, i1_l, tidx);
					}

					if ((tidx.tile_origin == tidx.global) && (must_swap || must_sort)) {
						unsorted[{0}] = 1u;
					}
				}

				tidx.barrier.wait_with_tile_static_memory_fence();

				done += exec_domain.size();
			}		
		});
	}

	template<unsigned int block_sz, typename AV>
	void odd_even_sort_range(const AV& R)
	{
		static constexpr unsigned int SIMD_w = 256u;

		concurrency::array_view<unsigned int, 1> unsorted(1);
		do {
			unsorted[{0}] = 0u;

			// Odd pass.
			odd_even_pass<block_sz, SIMD_w>(R, unsorted);

			// Even pass.
			odd_even_pass<block_sz, SIMD_w>(R, unsorted, false);
		} while (unsorted[{0}]);	
	}

	template<typename I>
	void sort(I f, I l)
	{
		if (f >= l) {
			return; // Nothing to do, empty range.
		}

		concurrency::array_view<typename std::iterator_traits<I>::value_type, 1> R(std::distance(f, l), &*f);

		static constexpr unsigned int block_sz = 1024u;

		almost_sort_elems(R, 2048u);
		sort_blocks<block_sz>(R, 10u);
		almost_sort_range<block_sz>(R, 29u);
		odd_even_sort_range<block_sz>(R);
	}
}	   // Namespace experimental.
#endif // _AMP_DOBOSIEWICZ_SORT_HPP_BUMPTZI