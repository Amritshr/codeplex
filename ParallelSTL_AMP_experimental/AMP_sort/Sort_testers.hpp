// Here we provide the mechanism for generating / retrieving vectors populated with
// elements of the sorted type, drawn from the distributions typically encountered
// in the literature. It is NOT optimised, and probably will never be as it is just
// a quick way of getting input for some easy going tests.
#pragma once

#ifndef _SORT_TESTERS_HPP_BUMPTZI
#define _SORT_TESTERS_HPP_BUMPTZI

#include <algorithm>
#include <cstdlib>
#include <limits>
#include <numeric>
#include <random>
#include <vector>
#include <type_traits>

namespace experimental
{

	template<typename T>
	typename std::enable_if<std::is_integral<T>::value, std::vector<T>>::type uniform_distributed_data(std::size_t N)
	{
		using namespace std;

		mt19937_64 g;
		uniform_int_distribution<T> d(numeric_limits<T>::min(), numeric_limits<T>::max());
		vector<T> r(N);

		generate(begin(r), end(r), rand);// [&]() { return (d(g)); });

		return (r);
	}

	template<typename T>
	typename std::enable_if<std::is_floating_point<T>::value, std::vector<T>>::type uniform_distributed_data(std::size_t N)
	{
		using namespace std;

		mt19937_64 g;
		uniform_real_distribution<T> d(numeric_limits<T>::min(), numeric_limits<T>::max());
		vector<T> r(N);

		generate(begin(r), end(r), [&]() { return (d(g)); });

		return (r);
	}

	template<typename T>
	std::vector<T> sorted_data(std::size_t N)
	{
		std::vector<T> r(N);
		std::iota(begin(r), end(r), T());

		return (r);
	}

	template<typename T>
	std::vector<T> reverse_sorted_data(std::size_t N)
	{
		std::vector<T> r(N);
		std::iota(rbegin(r), rend(r), T());

		return (r);
	}

	template<typename T>
	std::vector<T> unique_value_data(std::size_t N)
	{
		return (std::vector<T>(N, rand()));
	}

	template<typename T>
	typename std::enable_if<std::is_integral<T>::value, std::vector<T>>::type bucketed_data(std::size_t N, unsigned int bucket_cnt = 128u)
	{
		// Bucket generation comes from Cederman, D. & Tsigas, P. (2009)
		// "GPU-Quicksort: A Practical Quicksort Algorithm for Graphics
		// Processors"

		using namespace std;

		vector<T> r(N);
		const auto b_mul = numeric_limits<T>::max() / bucket_cnt;
		mt19937_64 g;

		for (auto i = 0u; i != bucket_cnt; ++i) {
			for (auto j = 0u; j != bucket_cnt; ++j) {
				auto f = begin(r) + i * (N / bucket_cnt) + j * (N / (bucket_cnt * bucket_cnt));
				auto l = f + (N / (bucket_cnt * bucket_cnt));
	
				uniform_int_distribution<T> d(j * b_mul, (j + 1) * b_mul - T(1));
				
				generate(f, l, [&]() { return (d(g)); });
			}
		}

		return (r);
	}

	template<typename T>
	typename std::enable_if<std::is_floating_point<T>::value, std::vector<T>>::type bucketed_data(std::size_t N, unsigned int bucket_cnt = 128u)
	{
		// Bucket generation comes from Cederman, D. & Tsigas, P. (2009)
		// "GPU-Quicksort: A Practical Quicksort Algorithm for Graphics
		// Processors"

		using namespace std;

		vector<T> r(N);
		const auto b_mul = numeric_limits<T>::max() / bucket_cnt;
		mt19937_64 g;

		for (auto i = 0u; i != bucket_cnt; ++i) {
			for (auto j = 0u; j != bucket_cnt; ++j) {
				auto f = begin(r) + i * (N / bucket_cnt) + j * (N / (bucket_cnt * bucket_cnt));
				auto l = f + (N / (bucket_cnt * bucket_cnt));
								
				uniform_real_distribution<T> d(j * b_mul, (j + 1) * b_mul - T(1));

				generate(f, l, [&]() { return (d(g)); });
			}
		}

		return (r);
	}

	template<typename T>
	typename std::enable_if<std::is_integral<T>::value, std::vector<T>>::type gaussian_data(std::size_t N)
	{
		using namespace std;

		vector<T> r(N);
		mt19937_64 g;
		uniform_int_distribution<T> d(numeric_limits<T>::min(), numeric_limits<T>::max());

		// A pretty clumsy way of constructing a Gaussian from averages of 4 random vals. The wonky long-hand div is to avoid overflow.
		generate(begin(r), end(r), [&]() { return (d(g) / T(4) + d(g) / T(4) + d(g) / T(4) + d(g) / T(4)); }); 
		
		return (r);
	}

	template<typename T>
	typename std::enable_if<std::is_floating_point<T>::value, std::vector<T>>::type gaussian_data(std::size_t N)
	{
		using namespace std;

		vector<T> r(N);
		mt19937_64 g;
		normal_distribution<T> d((numeric_limits<T>::max() - numeric_limits<T>::min()) / T(2), (numeric_limits<T>::max() - numeric_limits<T>::min()) / T(8));

		generate(begin(r), end(r), [&]() { return (d(g)); });

		return (r);
	}

	template<typename T>
	typename std::enable_if<std::is_integral<T>::value, std::vector<T>>::type staggered_data(std::size_t N, unsigned int bucket_cnt = 128u)
	{
		// Stagger generation comes from Cederman, D. & Tsigas, P. (2009)
		// "GPU-Quicksort: A Practical Quicksort Algorithm for Graphics
		// Processors"

		using namespace std;

		vector<T> r(N);
		mt19937_64 g;

		const auto b_mul = numeric_limits<T>::max() / bucket_cnt;
		for (auto i = 0u; i != bucket_cnt; ++i) {
			auto f = begin(r) + i * (N / bucket_cnt);
			auto l = f + (N / bucket_cnt);
			
			const auto b_min = (i < (bucket_cnt / T(2))) ? ((2u * i + 1u) * b_mul) : ((2u * i - bucket_cnt) * b_mul);			
			const auto b_max = (i < (bucket_cnt / T(2))) ? ((2u * i + 2u) * b_mul - 1u) : ((2u * i - bucket_cnt + 1u) * (b_mul - 1u));
				
			uniform_int_distribution<T> d(b_min, b_max);

			generate(f, l, [&]() { return (d(g)); });
		}

		return (r);
	}

	template<typename T>
	typename std::enable_if<std::is_floating_point<T>::value, std::vector<T>>::type staggered_data(std::size_t N, unsigned int bucket_cnt = 128u)
	{
		// Stagger generation comes from Cederman, D. & Tsigas, P. (2009)
		// "GPU-Quicksort: A Practical Quicksort Algorithm for Graphics
		// Processors"

		using namespace std;

		vector<T> r(N);
		mt19937_64 g;

		const auto b_mul = numeric_limits<T>::max() / bucket_cnt;
		for (auto i = 0u; i != bucket_cnt; ++i) {
			auto f = begin(r) + i * (N / bucket_cnt);
			auto l = f + (N / bucket_cnt);

			const auto b_min = (i < (bucket_cnt / T(2))) ? ((2u * i + 1u) * b_mul) : ((2u * i - bucket_cnt) * b_mul);
			const auto b_max = (i < (bucket_cnt / T(2))) ? ((2u * i + 2u) * b_mul - 1u) : ((2u * i - bucket_cnt + 1u) * (b_mul - 1u));

			uniform_real_distribution<T> d(b_min, b_max);

			generate(f, l, [&]() { return (d(g)); });
		}

		return (r);
	}

}	   // Namespace experimental.	
#endif // _SORT_TESTERS_HPP_BUMPTZI
