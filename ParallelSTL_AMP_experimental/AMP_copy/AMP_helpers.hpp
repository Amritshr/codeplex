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
}	   // Namespace experimental.
#endif // _AMP_HELPERS_HPP_BUMPTZI