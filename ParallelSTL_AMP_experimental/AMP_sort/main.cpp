#include "AMP_helpers.hpp"
#include "AMP_dobosiewicz_sort.hpp"
#include "Sort_testers.hpp"
#include <thrust\sort.h>   // You'll need the CUDA toolkit installed for this to work, sadly.
#include <bolt\amp\sort.h> // Include this if you want to test BOLT, after downloading it:-).

#include <algorithm>
#include <cstdlib>
#include <exception>
#include <functional>
#include <iostream>
#include <vector>

int main()
{
	using namespace concurrency;
	using namespace experimental;
	using namespace std;

	//accelerator::set_default(accelerator::direct3d_warp);
	try {
		using Sorted_type = float;
		using Sorted_type_iter = vector<Sorted_type>::iterator;
		auto Sort_impl = experimental::sort<Sorted_type_iter>;
						// parallel_sort<Sorted_type_iter>;
						//   static_cast<void(*)(Sorted_type_iter, Sorted_type_iter)>(bolt::amp::sort<Sorted_type_iter>); // This is ugly because the fn is overloaded and a cast is used to resolve it.
						// thrust::sort<Sorted_type_iter>;
						// std::sort<Sorted_type_iter>;
		
		static constexpr unsigned int elem_cnt = (1 << 22);
		static constexpr unsigned int bucket_cnt = 128u; // To be used for the bucket and staggered distributions.
		
		using Test = tuple<string, function<vector<Sorted_type>(size_t)>>;
		constexpr vector<Test> tests = { Test("unsorted, uniformly distributed", uniform_distributed_data<Sorted_type>),
							  			 Test("sorted", sorted_data<Sorted_type>),
										 Test("reverse sorted", reverse_sorted_data<Sorted_type>),
										 Test("equal", unique_value_data<Sorted_type>),
										 Test("unsorted, bucketed", bind(bucketed_data<Sorted_type>, placeholders::_1, bucket_cnt)),
										 Test("unsorted, gaussian distributed", gaussian_data<Sorted_type>),
										 Test("unsorted, staggered", bind(staggered_data<Sorted_type>, placeholders::_1, bucket_cnt)) };
		
		for (const Test& test : tests) {
			auto sort_input = get<1>(test)(elem_cnt);
			
			auto sort_time = time_fn([&]() { Sort_impl(begin(sort_input), end(sort_input)); });

			cout << "Sorting rate for " << get<0>(test) << " range: " << elem_cnt / (1000u * sort_time)
				 << " Mkeys / s." << endl;
		}
		cin.get();
		
		return (EXIT_SUCCESS);
	}
	catch (exception& ex) {
		cerr << ex.what() << endl;
		cin.get();

		return (EXIT_FAILURE);
	}
}
