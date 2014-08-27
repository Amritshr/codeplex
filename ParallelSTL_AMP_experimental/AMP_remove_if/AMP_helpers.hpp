// This is an assortment of various helpers e.g. scan, reduce or allocator for tile_static (thread_local) stuff.
#pragma once
#ifndef _AMP_HELPERS_HPP_BUMPTZI
#define _AMP_HELPERS_HPP_BUMPTZI

#include <amp.h>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <type_traits>
#include <utility>

namespace experimental
{
	// A helper for timing the runtime of a particular function. Returns it in milliseconds by default.
	template<typename F, typename T = std::chrono::milliseconds>
	inline auto time_fn(const F& fn) -> decltype(T().count())
	{
		auto start = std::chrono::high_resolution_clock::now();

		fn();

		auto stop = std::chrono::high_resolution_clock::now();

		return (std::chrono::duration_cast<T>(stop - start).count());
	}


	// An implementation of std::fill for use in restrict(amp) contexts.
	template<typename I, typename T>
	void fill(I f, I l, const T& v, unsigned int off = 1u) restrict(amp)
	{
		while (f < l) {
			*f = v;
			f += off;
		}
	}

	// An implementation of serial upper_bound and lower_bound for restrict(amp) contexts.
	template<typename I, typename T>
	I lower_bound(I f, I l, const T& val) restrict(amp)
	{
		while (f < l) {
			auto i0 = f + (l - f) / 2;

			if (*i0 < val) {
				f = ++i0;
			}
			else {
				l = i0;
			}
		}

		return (f);
	}

	template<typename I, typename T>
	I upper_bound(I f, I l, const T& val) restrict(amp)
	{
		while (f < l) {
			auto i0 = f + (l - f) / 2;

			if (val < *i0) {
				l = i0;
			}
			else {
				f = ++i0;
			}
		}

		return (f);
	}

	// An implementation of copy that can be strided so as to use ale lanes in the copy.
	// process.
	template<typename I, typename O>
	O copy(I i_f, I i_l, O o_f, unsigned int off = 1u) restrict(amp)
	{
		while (i_f < i_l) {
			*o_f = *i_f;

			o_f += off;
			i_f += off;
		}

		return (o_f);
	}

	// An implementation of serial merge that can be used in a restrict(amp) context.
	template<typename I1, typename I2, typename O>
	O merge(I1 f_1, I1 l_1, I2 f_2, I2 l_2, O o_f) restrict(amp)
	{
		while ((f_1 != l_1) && (f_2 != l_2)) {
			*o_f++ = (*f_1 < *f_2) ? *f_1++ : *f_2++;
		}

		while (f_1 != l_1) {
			*o_f++ = *f_1++;
		}

		while (f_2 != l_2) {
			*o_f++ = *f_2++;
		}

		return (o_f);
	}

	// An implementation of swap for restrict(amp) contexts.
	template<typename T>
	void swap(T& a, T& b) restrict(amp)
	{
		const auto tmp = a;
		a = b;
		b = tmp;
	}

	// An implementation of swap_ranges that can be strided so as to use all lanes in
	// the swapping process.
	template<typename I1, typename I2>
	void swap_ranges(I1 f_A, I1 l_A, I2 f_B, unsigned int off = 1u) restrict(cpu, amp)
	{
		while (f_A < l_A) {
			swap(*f_A, *f_B);

			f_A += off;
			f_B += off;
		}
	}
	// Two functions approximately similar to the new expression (but not excessively so), hiding tile_static (thread_local)
	// allocation from prying eyes.
	template<typename T>
	T* thread_local_new() restrict(amp)
	{
		tile_static T new_T;
		return (&new_T);
	}
	template<typename T, unsigned int sz>
	T* thread_local_new() restrict(amp)
	{
		tile_static T new_T_arr[sz];
		return (new_T_arr);
	}

	// Since it is annoying to call the different {i, u}{min, max} functions in concurrency::direct3d, we use some enable_if
	// magic to get back to a min / max normality for restrict(amp) work. Also some template aliases to minimize eye-bleed.
	template<typename T>
	using Signed_integer = typename std::enable_if<(std::is_integral<T>::value) && (std::is_signed<T>::value), T>::type;
	template<typename T>
	using Unsigned_integer = typename std::enable_if<(std::is_integral<T>::value) && (std::is_unsigned<T>::value), T>::type;
	// For ints.
	template<typename T, Signed_integer<T> = 0>
	T max(const T& x0, const T& x1) restrict(amp)
	{
		return (concurrency::direct3d::imax(x0, x1));
	}
	template<typename T, Signed_integer<T> = 0>
	T min(const T& x0, const T& x1) restrict(amp)
	{
		return (concurrency::direct3d::imin(x0, x1));
	}
	// For unsigned ints.
	template<typename T, Unsigned_integer<T> = 0u>
	T max(const T& x0, const T& x1) restrict(amp)
	{
		return (concurrency::direct3d::imax(x0, x1));
	}
	template<typename T, Unsigned_integer<T> = 0u>
	T min(const T& x0, const T& x1) restrict(amp)
	{
		return (concurrency::direct3d::imin(x0, x1));
	}

	// A quick implementation of the Egyptian algorithm for fast squaring, since it is quite annoying to not have a version
	// of pow for integers. Computes a^n for a, n integers. More information can be found in Stepanov, A. & McJones, P. (2009)
	// "Elements of Programming".
	template<typename T>
	using Integer = typename std::enable_if<std::is_integral<T>::value, T>::type;
	template<typename T, typename Op = std::multiplies<T>, Integer<T> = T>
	T pow(T a, T n, Op op = Op())
	{
		if (n == T(0)) {
			return (T(1));
		}

		const auto pow_accum = [](T r, T a, T n, Op o) {
			while (true) {
				if (n % T(2) == T(1)) {
					r = o(r, a);
					if (n == T(1)) {
						return (r);
					}
				}
				a = o(a, a);
				n /= T(2);
			}
		};

		while (n % T(2) == T(0)) {
			a = op(a, a);
			n /= T(2);
		}
		n /= T(2);
		
		if (n == T(0)) {
			return (a);
		}
		return (pow_accum(a, op(a, a), n, op));
	}
	// Specializations for begin / end and cbegin / cend for the case in which the argument is an array_view<T, 1>.
	template<typename T>
	auto begin(const concurrency::array_view<T, 1>& c) restrict(cpu, amp) -> decltype(c.data())
	{
		return (c.data());
	}
	template<typename T>
	auto end(const concurrency::array_view<T, 1>& c) restrict(cpu, amp) -> decltype(c.data())
	{
		return (c.data() + c.extent.size());
	}
	template<typename T>
	auto cbegin(const concurrency::array_view<T, 1>& c) restrict(cpu, amp) -> decltype(c.reinterpret_as<const T>().data())
	{
		return (c.data());
	}
	template<typename T>
	auto cend(const concurrency::array_view<T, 1>& c) restrict(cpu, amp) -> decltype(c.reinterpret_as<const T>().data())
	{
		return (c.data() + c.extent.size());
	}

	// Simple, relatively storage oblivious, reduce, using the binary tree technique.
	template<typename I, typename Idx>
	I reduce(I f, I l, const Idx& tidx) restrict(amp)
	{
		if (f >= l) return (l); // Empty range passed, probably an error.

		auto m = tidx.tile_dim0 / 2u;		
		while (m) {
			if (tidx.local[0] < m) *(f + tidx.local[0]) += *(f + m + tidx.local[0]);

			tidx.barrier.wait_with_tile_static_memory_fence();

			m /= 2u;
		}

		return (f);
	}
	// Simple, serial scan, to be done per SIMD lane. First base, using addition.
	template<typename I>
	I serial_scan(I f, I l) restrict(cpu, amp)
	{
		if (f >= l) return (l); // Empty range passed, probably an error.

		using T = typename std::iterator_traits<I>::value_type;
		
		T res = T();
		while (f != l) {
			auto tmp = *f;
			*f++ = res;
			res += tmp;
		}

		return (--l); // We return the result of the scan.
	}

	// Now custom binary operation case.
	template<typename I, typename Op>
	I serial_scan(I f, I l, Op op) restrict(cpu, amp)
	{
		if (f >= l) {
			return (l); // Empty range passed, probably an error.
		}

		decltype(*f) res;
		while (f != l) {
			auto tmp = *f;
			*f++ = res;
			res = op(res, tmp);
		}

		return (--l);
	}

	// Simple, thread-wide scan, using the Kogge-Stone construction.
	// First an optimised, divergence free implementation.
	template<typename I, typename Idx>
	I scan_thread_padded(I f, I l, const Idx& tidx) restrict(amp)
	{
		using T = typename std::iterator_traits<I>::value_type;

		auto s = 1u;
		while (s < tidx.tile_dim0) {
			const auto tmp = *(f + tidx.local[0] - s);

			tidx.barrier.wait_with_tile_static_memory_fence();

			*(f + tidx.local[0]) += tmp;

			tidx.barrier.wait_with_tile_static_memory_fence();

			s *= 2u;
		}

		const auto tmp = *(f + tidx.local[0] - 1);

		tidx.barrier.wait_with_tile_static_memory_fence();

		*(f + tidx.local[0]) = tmp;

		return (--l);
	}
	// Second, the generic one.
	template<typename I, typename Idx>
	I scan_thread(I f, I l, const Idx& tidx, bool padded = false) restrict(amp)
	{
		if (f >= l) return (l); // Empty range passed, probably an error.

		if (padded) return (scan_thread_padded(f, l, tidx)); // Caller gives us a nice pad, use the optimal version.

		using T = typename std::iterator_traits<I>::value_type;

		auto stride = 1u;
		while (stride < tidx.tile_dim0) {
			const auto tmp = (tidx.local[0] >= stride) ? *(f + tidx.local[0] - stride) : T();

			tidx.barrier.wait_with_tile_static_memory_fence();

			*(f + tidx.local[0]) += tmp;

			tidx.barrier.wait_with_tile_static_memory_fence();

			stride *= 2u;			
		}
		
		const auto tmp = *(f + tidx.local[0]);

		tidx.barrier.wait_with_tile_static_memory_fence();

		if (tidx.local[0] != (tidx.tile_dim0 - 1)) *(f + tidx.local[0] + 1) = tmp;
		else *f = T();

		return (--l);
	}

	// Simple, serial accumulate, to be done per SIMD lane. First base, using addition.
	template<typename I, typename T>
	T serial_accumulate(I f, I l, T init) restrict(cpu, amp)
	{
		if (f >= l) {
			return (init); // Empty range passed, probably an error.
		}

		while (f != l) {
			init += *f++;
		}

		return (init);
	}
	// Now the custom binary operation case.
	template<typename I, typename T, typename Op>
	T serial_accumulate(I f, I l, T init, Op op) restrict(cpu, amp)
	{
		if (f >= l) {
			return (init); // Empty range passed.
		}

		while (f != l) {
			init = op(init, *f++);
		}

		return (init);
	}

	// An implementation of partition that is both data-parallel and in-place. Note that the in-place version is not 
	// guaranteed to work with the current DirectX specification (or OpenCL for that matter), since we cannot guarantee
	// that threads (tiles) will not deadlock if the scheduling policy is not uniform i.e. they don't all get swapped in
	// and out fairly.
	// First, some helpers for locking / unlocking / spinning. Not for general use.
	template<typename B, typename Idx>
	void acquire_block(const B& blocks, const Idx& tidx, unsigned int block) restrict(amp)
	{
		if (tidx.tile_origin == tidx.global) {
			unsigned int unlocked = 0u;
			unsigned int locked = 1u;
			while (!concurrency::atomic_compare_exchange(&blocks(block), &unlocked, locked)) { /* Spin */ };
		}
		tidx.barrier.wait_with_global_memory_fence();
	}
	template<typename B, typename Idx>
	void release_block(const B& blocks, const Idx& tidx, unsigned int block) restrict(amp)
	{
		if (tidx.tile_origin == tidx.global) {
			concurrency::atomic_exchange(&blocks(block), 0u);
		}
		tidx.barrier.wait_with_global_memory_fence();
	}
	// Finally the implementation.
	template<typename I, typename P>
	I partition(I f, I l, P p)
	{
		if (f >= l) {
			return (l); // Empty range passed.
		}

		using namespace concurrency;
		using T = typename std::iterator_traits<I>::value_type;
		using Counters = typename std::pair<unsigned int, unsigned int>;

		static constexpr unsigned int SIMD_w = 64u;
	
		array_view<T, 1> range(std::distance(f, l), &*f);

		const auto domain = range.extent.tile<SIMD_w>().pad();
		array_view<unsigned int, 1> blocks(domain.size() / SIMD_w);
		parallel_for_each(blocks.extent, [=](const index<1>& idx) restrict(amp) { blocks[idx] = 1u; });
		
		Counters cnt(0u, range.extent.size());
		array_view<Counters, 1> counters(1, &cnt);
	
		parallel_for_each(domain, [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
			const auto valid = range.extent.contains(tidx.global); 
			const auto tmp = valid ? range[tidx.global] : T();
			release_block(blocks, tidx, tidx.tile[0]);

			tile_static unsigned int p_t[SIMD_w];
			tile_static unsigned int p_f[SIMD_w];
			
			array_view<unsigned int, 1> p_true(p_t);
			array_view<unsigned int, 1> p_false(p_f);
			
			p_true[tidx.local] = valid ? p(tmp) : 0u;
			p_false[tidx.local] = valid ? !p(tmp) : 0u;

			tidx.barrier.wait_with_tile_static_memory_fence();

			scan(begin(p_true), end(p_true), tidx);
			scan(begin(p_false), end(p_false), tidx);

			tidx.barrier.wait_with_tile_static_memory_fence();

			tile_static unsigned int false_idx;
			tile_static unsigned int true_idx;	
			if (tidx.local[0] == (SIMD_w - 1u)) {
				const auto true_off = p_true[tidx.local] + (valid ? p(tmp) : 0u);
				const auto false_off = p_false[tidx.local] + (valid ? !p(tmp) : 0u);
				true_idx = atomic_fetch_add(&counters(0).first, true_off);
				false_idx = atomic_fetch_sub(&counters(0).second, false_off) - false_off;
			}
			
			tidx.barrier.wait_with_tile_static_memory_fence();

			acquire_block(blocks, tidx, true_idx / SIMD_w);

			if (valid && p(tmp)) {
				range(true_idx + p_true[tidx.local]) = tmp;
			}
			tidx.barrier.wait_with_global_memory_fence();

			release_block(blocks, tidx, true_idx / SIMD_w);

			acquire_block(blocks, tidx, false_idx / SIMD_w);

			if (valid && !p(tmp)) {
				range(false_idx + p_false[tidx.local]) = tmp;
			}
			tidx.barrier.wait_with_global_memory_fence();

			release_block(blocks, tidx, false_idx / SIMD_w);
		});
		counters.synchronize();
		return (f + counters[{0}].first);
	}
	// An O(n) storage version of partition, without the quirkiness of the synchronizations.
//	template<typename I, typename P>
//	I partition(I f, I l, P p)
//	{
//		if (f >= l) {
//			return (l); // Empty range passed.
//		}

//		using namespace concurrency;
//		using T = typename std::iterator_traits<I>::value_type;
//		using Counters = typename std::pair<unsigned int, unsigned int>;

//		static constexpr unsigned int SIMD_w = 64u;

//		Counters cnt(0, distance(f, l));
//		array_view<Counters, 1> counters(1, &cnt);
		
//		array_view<const T, 1> in(std::distance(f, l), &*f);
//		array_view<T, 1> out(std::distance(f, l));

//		parallel_for_each(in.extent.tile<SIMD_w>().pad(), [=](const tiled_index<SIMD_w>& tidx) restrict(amp) {
//			const auto tmp = in.extent.contains(tidx.global) ? in[tidx.global] : T();

//			tile_static unsigned int p_t[SIMD_w];
//			tile_static unsigned int p_f[SIMD_w];

//			array_view<unsigned int, 1> p_true(p_t);
//			array_view<unsigned int, 1> p_false(p_f);

//			p_true[tidx.local] = in.extent.contains(tidx.global) ? p(tmp) : 0u;
//			p_false[tidx.local] = in.extent.contains(tidx.global) ? !p(tmp) : 0u;

//			tidx.barrier.wait_with_tile_static_memory_fence();

//			scan(begin(p_true), end(p_true), tidx);
//			scan(begin(p_false), end(p_false), tidx);

//			tile_static unsigned int true_idx;
//			tile_static unsigned int false_idx;

//			if (tidx.local[0] == (SIMD_w - 1)) {
//				true_idx = atomic_fetch_add(&counters[{0}].first, p_true[tidx.local[0]] + p(tmp));
//				false_idx = atomic_fetch_sub(&counters[{0}].second, p_false[tidx.local[0]] + !p(tmp)) - p_false[tidx.local[0]] - !p(tmp);
//			}
//			tidx.barrier.wait_with_tile_static_memory_fence();

//			if (out.extent.contains(tidx.global)) {
//				const int out_idx = p(tmp) ? (true_idx + p_true[tidx.local]) : (false_idx + p_false[tidx.local]);
//				out[out_idx] = tmp;
//			}
//		});
//		concurrency::copy(out, f);
//		return (f + counters[{0}].first);
//	}
}	   // Namespace experimental.
#endif // _AMP_HELPERS_HPP_BUMPTZI