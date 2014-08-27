// This hosts a straightforward implementation of copy, alongside a set of overloads
// for restrict(amp) contexts, that have proven to be rather useful in practice.
#pragma once
#ifndef _AMP_COPY_HPP_BUMPTZI
#define _AMP_COPY_HPP_BUMPTZI

#include "AMP_helpers.hpp"

#include <amp.h>
#include <iterator>

namespace experimental
{
	// This is a (n almost, given the extra parameter) overload of copy to be used in restrict(amp)
	// contexts. By using the offset parameter one can do copies using all lanes in a thread, as is
	// demonstrated elsewhere in the implemented algorithms.
	template<typename I, typename O>
	inline O copy(I i_f, I i_l, O o_f, unsigned int off = 1u) restrict(amp)
	{
		while (i_f < i_l) {
			*o_f = *i_f;
			i_f += off;
			o_f += off;
		}

		return (o_f);
	}
	// This is the actually useful variant, as both array_views might reside on the same accelerator
	// and it is desirable to avoid copying back to host just for the copy.
	template<typename AV1, typename AV2>
	inline void copy(const AV1& in, const AV2& out)
	{
		out.discard_data();

		if (in.source_accelerator_view == out.source_accelerator_view) {
			concurrency::parallel_for_each(in.extent, [=](const concurrency::index<1>& idx) restrict(amp) { out[idx] = in[idx]; });
		}
		else {
			concurrency::copy(in, out);
		}
	}

	// This is the probably not very useful variant, but the one that is needed for alignment with
	// existing practice.
	template<typename I, typename O>
	O copy(I i_f, I i_l, O o_f)
	{
		using namespace concurrency;

		if (i_f >= i_l) return (o_f); // Empty range passed, probably an error.

		copy(array_view<const Iterator_value_type<I>, 1>(std::distance(i_f, i_l), &*i_f), array_view<Iterator_value_type<I>, 1> out(in.extent, &*o_f));

		return (o_f);
	}

	// This is just a token of how it is possible to handle the insertion adapters. If doing copies on an
	// accelerator across two ranges that live elsewhere is silly, this is even sillier, but it can be done.
	//template<typename I, typename C>
	//std::back_insert_iterator<C> copy(I i_f, I i_l, std::back_insert_iterator<C> o_f)
	//{
	//	using namespace concurrency;

	//	if (i_f >= i_l) return (o_f); // Empty range passed, probably an error.

	//	array_view<const Iterator_value_type<I>, 1> in(std::distance(i_f, i_l), &*i_f);
	//	array_view<Iterator_value_type<I>, 1> out(in.extent);

	//	copy(in, out);

	//	auto f = cbegin(out);
	//	while (f != cend(out)) 	*o_f++ = *f++;

	//	return (o_f);
	//}
}	   // Namespace experimental.
#endif // _AMP_COPY_HPP_BUMPTZI