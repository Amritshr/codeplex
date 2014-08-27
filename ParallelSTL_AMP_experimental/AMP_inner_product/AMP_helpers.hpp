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
	template<typename I>
	using Iterator_value_type = typename std::iterator_traits<I>::value_type;

	template<typename C>
	using Container_value_type = typename C::value_type;

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
}	   // Namespace experimental.
#endif // _AMP_HELPERS_HPP_BUMPTZI