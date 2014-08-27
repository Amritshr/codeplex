// This includes a straightforward implementation of generate, with the particular
// wrinkle that the onus is on the caller to ensure that it is safe / defined
// behaviour to call the generator in parallel, from multiple threads and the lanes
// associated with the latter. We also provide a (n overload) version intended for
// use in restrict(amp) contexts.
#pragma once
#ifndef _AMP_GENERATE_HPP_BUMPTZI
#define _AMP_GENERATE_HPP_BUMPTZI

#include "AMP_helpers.hpp"

#include <algorithm>
#include <amp.h>

namespace experimental
{
	template<typename I, typename G>
	void generate(I f, I l, G gen, unsigned int off = 1u) restrict(amp)
	{
		while (f < l) {
			*f = gen();
			f += off;
		}
	}

	template<typename AV, typename G>
	inline void generate(const AV& R, G gen)
	{
		R.discard_data();

		concurrency::parallel_for_each(R.extent, [=](const concurrency::index<1>& idx) restrict(amp) { R[idx] = gen(); });
	}

	template<typename I, typename G>
	void generate(I f, I l, G gen)
	{
		if (f >= l) return; // Empty range passed, probably an error.

		generate(concurrency::array_view<Iterator_value_type<I>, 1>(std::distance(f, l), &*f), gen);
	}
}	   // Namespace experimental.
#endif // _AMP_GENERATE_HPP_BUMPTZI