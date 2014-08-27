#pragma once

#ifndef _EXECUTION_POLICY_UTILS_H_
#define _EXECUTION_POLICY_UTILS_H_	1

_PSTL_NS1_BEGIN

// Adding debug execution poliyc that will force static paritioner
class debug_static_execution_policy :
	public parallel_execution_policy
{
	size_t _Chunk_size;
	size_t _Chunk_count;
public:
	debug_static_execution_policy(size_t _Count, size_t _Min_chunk_size = 0) _NOEXCEPT : _Chunk_size(0), _Chunk_count(0)
	{
		const unsigned int _HdConc = std::thread::hardware_concurrency();
		_Chunk_size = (_Count + _HdConc - 1) / _HdConc;
		_Chunk_count = _Count / _Chunk_size + 1;

		if (_Count % _Chunk_size < _Min_chunk_size)
			--_Chunk_count;
	}

	size_t get_chunk_size() _NOEXCEPT
	{
		return _Chunk_size;
	}

	size_t get_chunk_count() _NOEXCEPT
	{
		return _Chunk_count;
	}
};

template<> struct is_execution_policy<debug_static_execution_policy> : true_type{};

namespace details {
	template<>
	struct _Partitioner<debug_static_execution_policy> :
		public _Partitioner < static_partitioner_tag >
	{
	};
}
_PSTL_NS1_END // std::experimental::parallel

#endif // _EXECUTION_POLICY_UTILS_H_
