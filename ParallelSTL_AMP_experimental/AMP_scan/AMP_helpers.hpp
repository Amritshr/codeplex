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

	// Simple, serial accumulate, to be done per SIMD lane. First base, using addition.
	template<typename I, typename T>
	T serial_accumulate(I f, I l, T init) restrict(cpu, amp)
	{
		if (f >= l) return (init); // Empty range passed, probably an error.

		while (f != l) init += *f++;

		return (init);
	}
}	   // Namespace experimental.
#endif // _AMP_HELPERS_HPP_BUMPTZI