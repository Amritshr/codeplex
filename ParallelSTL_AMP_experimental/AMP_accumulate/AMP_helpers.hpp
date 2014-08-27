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

	// min and max for restrict(amp) contexts, with overloads for int and unsigned int using the
	// direct3d intrinsincs provided in concurrency::direct3d. First max.
	template<typename T>
	T max(const T& x, const T& y) restrict(amp)
	{
		return ((y < x) ? x : y);
	}
	int max(int x, int y) restrict(amp)
	{
		return (concurrency::direct3d::imax(x, y));
	}
	unsigned int max(unsigned int x, unsigned int y) restrict(amp)
	{
		return (concurrency::direct3d::umax(x, y));
	}
	// Then min.
	template<typename T>
	T min(const T& x, const T& y) restrict(amp)
	{
		return ((x < y) ? x : y);
	}
	int min(int x, int y) restrict(amp)
	{
		return (concurrency::direct3d::imin(x, y));
	}
	unsigned int min(unsigned int x, unsigned int y) restrict(amp)
	{
		return (concurrency::direct3d::umin(x, y));
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

	// Simple, tile sized, reduce, using the binary tree technique.
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
	// Simple, tile sized, reduce, using the binary tree technique, with a custom binary operator.
	template<typename I, typename Idx, typename Op>
	I reduce(I f, I l, const Idx& tidx, Op op) restrict(amp)
	{
		if (f >= l) return (l);

		auto m = tidx.tile_dim0 / 2u;
		while (m) {
			if (tidx.local[0] < m) *(f + tidx.local[0]) = op(*(f + tidx.local[0]), *(f + m + tidx.local[0]));

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

	// Simple, serial accumulate, to be done per SIMD lane. First base, using addition.
	template<typename I, typename T>
	T serial_accumulate(I f, I l, T init) restrict(cpu, amp)
	{
		if (f >= l) return (init); // Empty range passed, probably an error.

		while (f != l) init += *f++;

		return (init);
	}

	// A wrapper structure for Binary operators with identity elements. Properly we should 
	// describe / talk about monoids.
	template<typename F, typename T>
	struct Binary_function_with_identity {
		Binary_function_with_identity(const T& id) restrict(cpu, amp) : identity_element(id) {};

		template<typename U, typename V>
		auto operator()(U&& x, V&& y) const restrict(amp) -> decltype(F()(x, y)) { return (F()(x, y)); };

		const T identity_element;
	};

}	   // Namespace experimental.
#endif // _AMP_HELPERS_HPP_BUMPTZI