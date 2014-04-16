#include "stdafx.h"

#include <vector>

namespace std {
	namespace experimental {
		namespace parallel {
			struct MyPolicy : public parallel_execution_policy {};
			template<> struct is_execution_policy<MyPolicy> : true_type{};
		}
	}
}

namespace ParallelSTL_Tests
{
	using namespace std::experimental::parallel::details;

	class EmptyTester
	{
	public:

		template<typename _T>
		void test(_T)
		{
		}
	};

	// Tester that detects if invalid execution policy was passed
	class ExceptionTester
	{
		int _Tests;
		int _Passed;
	public:
		ExceptionTester() : _Tests(0), _Passed(0)
		{
		}

		template<typename _T>
		void test(_T _Func)
		{
			_Tests++;
			try {
				_Func();

				// The functor must throw invalid_argument exception otherwise it doesn't implement generic execution policy
				Assert::Fail();
			}
			catch (std::invalid_argument&) {
				_Passed++;
			}

			Assert::AreEqual(_Tests, _Passed);
		}
	};

	TEST_CLASS(GenericTests)
	{
		template<typename _AlgoTester, typename _PolicyType, typename _InIt>
		void InstantiateAlgo(_PolicyType _Policy, _InIt _First, _InIt _Last)
		{
			static_assert(is_execution_policy<_PolicyType>::value, "Require execution policy type");

			typedef std::iterator_traits<_InIt>::value_type value_type;
			std::iterator_traits<_InIt>::difference_type count_res;
			value_type value_res;
			std::pair<_InIt, _InIt> pair_iter_res;
			bool bool_res;
			_InIt iter_res;

			_AlgoTester _Tester;

			_Tester.test([&](){
				count_res = count(_Policy, _First, _Last, 1);
			});

			_Tester.test([&](){
				count_res = count_if(_Policy, _First, _Last, [](value_type){ return true; });
			});

			_Tester.test([&](){
				bool_res = equal(_Policy, _First, _Last, _First);
			});

			_Tester.test([&](){
				bool_res = equal(_Policy, _First, _Last, _First, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				value_res = reduce(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				value_res = reduce(_Policy, _First, _Last, value_type());
			});

			_Tester.test([&](){
				value_res = reduce(_Policy, _First, _Last, value_type(), [](value_type, value_type){ return value_type();  });
			});

			_Tester.test([&](){
				iter_res = min_element(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				iter_res = min_element(_Policy, _First, _Last, [](value_type, value_type){ return value_type(); });
			});

			_Tester.test([&](){
				iter_res = max_element(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				iter_res = max_element(_Policy, _First, _Last, [](value_type, value_type){ return value_type(); });
			});

			_Tester.test([&](){
				pair_iter_res = minmax_element(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				pair_iter_res = minmax_element(_Policy, _First, _Last, [](value_type, value_type){ return value_type(); });
			});

			_Tester.test([&](){
				iter_res = find(_Policy, _First, _Last, value_type());
			});

			_Tester.test([&](){
				iter_res = find_if(_Policy, _First, _Last, [](value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = find_if_not(_Policy, _First, _Last, [](value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = find_first_of(_Policy, _First, _Last, _First, _Last);
			});

			_Tester.test([&](){
				iter_res = find_first_of(_Policy, _First, _Last, _First, _Last, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = find_end(_Policy, _First, _Last, _First, _Last);
			});

			_Tester.test([&](){
				iter_res = find_end(_Policy, _First, _Last, _First, _Last, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				for_each(_Policy, _First, _Last, [](value_type){});
			});

			_Tester.test([&](){
				iter_res = for_each_n(_Policy, _First, count_res, [](value_type){});
			});

			_Tester.test([&](){
				generate(_Policy, _First, _Last, [](){ return value_type(); });
			});

			_Tester.test([&](){
				generate_n(_Policy, _First, count_res, [](){ return value_type(); });
			});

			_Tester.test([&](){
				fill(_Policy, _First, _Last, value_type());
			});

			_Tester.test([&](){
				fill_n(_Policy, _First, count_res, value_type());
			});

			_Tester.test([&](){
				uninitialized_fill(_Policy, _First, _Last, value_type());
			});

			_Tester.test([&](){
				iter_res = uninitialized_fill_n(_Policy, _First, count_res, value_type());
			});

			_Tester.test([&](){
				uninitialized_copy(_Policy, _First, _Last, _First);
			});

			_Tester.test([&](){
				iter_res = uninitialized_copy_n(_Policy, _First, count_res, _First);
			});

			_Tester.test([&](){
				any_of(_Policy, _First, _Last, [](value_type){ return true; });
			});

			_Tester.test([&](){
				none_of(_Policy, _First, _Last, [](value_type){ return true; });
			});

			_Tester.test([&](){
				all_of(_Policy, _First, _Last, [](value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = adjacent_find(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				iter_res = adjacent_find(_Policy, _First, _Last, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				pair_iter_res = mismatch(_Policy, _First, _Last, _First);
			});

			_Tester.test([&](){
				pair_iter_res = mismatch(_Policy, _First, _Last, _First, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				pair_iter_res = mismatch(_Policy, _First, _Last, _First, _Last);
			});

			_Tester.test([&](){
				pair_iter_res = mismatch(_Policy, _First, _Last, _First, _Last, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = remove(_Policy, _First, _Last, value_type());
			});

			_Tester.test([&](){
				iter_res = remove_if(_Policy, _First, _Last, [](value_type) { return true; });
			});

			_Tester.test([&](){
				iter_res = remove_copy(_Policy, _First, _Last, _First, value_type());
			});

			_Tester.test([&](){
				iter_res = remove_copy_if(_Policy, _First, _Last, _First, [](value_type) { return true; });
			});

			_Tester.test([&](){
				iter_res = unique(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				iter_res = unique(_Policy, _First, _Last, [](value_type, value_type) { return true; });
			});

			_Tester.test([&](){
				iter_res = unique_copy(_Policy, _First, _Last, _First);
			});

			_Tester.test([&](){
				iter_res = unique_copy(_Policy, _First, _Last, _First, [](value_type, value_type) { return true; });
			});

			_Tester.test([&](){			
				bool_res = lexicographical_compare(_Policy, _First, _Last, _First, _Last);
			});

			_Tester.test([&](){
				bool_res = lexicographical_compare(_Policy, _First, _Last, _First, _Last, [](value_type, value_type) {return true; });
			});

			_Tester.test([&](){
				rotate(_Policy, _First, _Last, _Last);
			});

			_Tester.test([&](){
				rotate_copy(_Policy, _First, _Last, _Last, _First);
			});

			_Tester.test([&](){
				bool_res = is_partitioned(_Policy, _First, _Last, [](value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = partition(_Policy, _First, _Last, [](value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = stable_partition(_Policy, _First, _Last, [](value_type) { return true; });
			});

			_Tester.test([&](){
				pair_iter_res = partition_copy(_Policy, _First, _Last, _First, _First, [](value_type) { return true; });
			});

			_Tester.test([&](){
				bool_res = includes(_Policy, _First, _Last, _First, _Last);
			});

			_Tester.test([&](){
				bool_res = includes(_Policy, _First, _Last, _First, _Last, [](value_type, value_type) { return true; });
			});

			_Tester.test([&](){
				bool_res = includes(_Policy, _First, _Last, _First, _Last);
			});

			_Tester.test([&](){
				bool_res = includes(_Policy, _First, _Last, _First, _Last, [](value_type, value_type) { return true; });
			});

			_Tester.test([&](){
				iter_res = set_union(_Policy, _First, _Last, _First, _Last, _Last);
			});

			_Tester.test([&](){
				iter_res = set_union(_Policy, _First, _Last, _First, _Last, _Last, [](value_type, value_type) { return true; });
			});

			_Tester.test([&](){
				iter_res = set_difference(_Policy, _First, _Last, _First, _Last, _Last);
			});

			_Tester.test([&](){
				iter_res = set_difference(_Policy, _First, _Last, _First, _Last, _Last, [](value_type, value_type) { return true; });
			});

			_Tester.test([&](){
				iter_res = search(_Policy, _First, _Last, _First, _Last);
			});

			_Tester.test([&](){
				iter_res = search(_Policy, _First, _Last, _First, _Last, [](value_type, value_type) {return true; });
			});

			_Tester.test([&](){
				iter_res = search_n(_Policy, _First, _Last, value_type(), value_type());
			});

			_Tester.test([&](){
				iter_res = search_n(_Policy, _First, _Last, value_type(), value_type(), [](value_type, value_type) { return true; });
			});

			_Tester.test([&](){
				iter_res = set_intersection(_Policy, _First, _Last, _First, _Last, _First);
			});

			_Tester.test([&](){
				iter_res = set_intersection(_Policy, _First, _Last, _First, _Last, _First, [](value_type, value_type) { return true; });
			});
			
			_Tester.test([&](){
				iter_res = set_symmetric_difference(_Policy, _First, _Last, _First, _Last, _First);
			});

			_Tester.test([&](){
				iter_res = set_symmetric_difference(_Policy, _First, _Last, _First, _Last, _First, [](value_type, value_type) { return true; });
			});

			_Tester.test([&](){
				iter_res = copy(_Policy, _First, _Last, _First);
			});

			_Tester.test([&](){
				iter_res = copy_n(_Policy, _First, count_res, _First);
			});

			_Tester.test([&](){
				iter_res = copy_if(_Policy, _First, _Last, _First, [](value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = move(_Policy, _First, _Last, _First);
			});

			_Tester.test([&](){
				iter_res = swap_ranges(_Policy, _First, _Last, _First);
			});

			_Tester.test([&](){
				iter_res = transform(_Policy, _First, _Last, _First, [](value_type){ return value_type();  });
			});

			_Tester.test([&](){
				iter_res = transform(_Policy, _First, _Last, _First, _First, [](value_type, value_type){ return value_type(); });
			});

			_Tester.test([&](){
				replace(_Policy, _First, _Last, value_type(), value_type());
			});

			_Tester.test([&](){
				replace_if(_Policy, _First, _Last, [](value_type){ return true; }, value_type());
			});

			_Tester.test([&](){
				replace_copy(_Policy, _First, _Last, _First, value_type(), value_type());
			});

			_Tester.test([&](){
				replace_copy_if(_Policy, _First, _Last, _First, [](value_type){ return true; }, value_type());
			});

			_Tester.test([&](){
				reverse(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				reverse_copy(_Policy, _First, _Last, _First);
			});

			_Tester.test([&](){
				bool_res = is_sorted(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				bool_res = is_sorted(_Policy, _First, _Last, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = is_sorted_until(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				iter_res = is_sorted_until(_Policy, _First, _Last, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				merge(_Policy, _First, _Last, _First, _Last, _First);
			});

			_Tester.test([&](){
				merge(_Policy, _First, _Last, _First, _Last, _First, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				inplace_merge(_Policy, _First, _Last, _First);
			});

			_Tester.test([&](){
				inplace_merge(_Policy, _First, _Last, _First, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = exclusive_scan(_Policy, _First, _Last, _First);
			});

			_Tester.test([&](){
				iter_res = inclusive_scan(_Policy, _First, _Last, _First);
			});
		}

		template<typename _AlgoTester, typename _PolicyType, typename _InIt>
		void InstantiateAlgoRandomAccessOnly(_PolicyType _Policy, _InIt _First, _InIt _Last)
		{
			static_assert(is_execution_policy<_PolicyType>::value, "Require execution policy type");

			typedef std::iterator_traits<_InIt>::value_type value_type;
			_InIt iter_res;
			_AlgoTester _Tester;
			
			_Tester.test([&](){
				nth_element(_Policy, _First, _Last, _Last);
			});

			_Tester.test([&](){
				nth_element(_Policy, _First, _Last, _Last, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = partial_sort_copy(_Policy, _First, _Last, _First, _Last, [](value_type, value_type){ return true; });
			});

			_Tester.test([&](){
				iter_res = partial_sort_copy(_Policy, _First, _Last, _First, _Last);
			});

			_Tester.test([&](){
				sort(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				sort(_Policy, _First, _Last, std::less<>());
			});

			_Tester.test([&](){
				stable_sort(_Policy, _First, _Last);
			});

			_Tester.test([&](){
				stable_sort(_Policy, _First, _Last, std::less<>());
			});
			
			_Tester.test([&](){
				partial_sort(_Policy, _First, _Last, _Last);
			});

			_Tester.test([&](){
				partial_sort(_Policy, _First, _Last, _Last, std::less<>());
			});
		}

		// The test is designed just to instantiate the templates with different types of common input parameters in particular raw pointers
		TEST_METHOD(Instantiate)
		{
			// This test must just instantiate all template no runtime execution is requred
			volatile bool _Exit = true; // Removes constant expresssion warning
			int* _BeginRaw = nullptr, *_EndRaw = nullptr;

			MyPolicy custom;
			execution_policy policy(custom);			
			InstantiateAlgo<ExceptionTester>(policy, _BeginRaw, _EndRaw);
			InstantiateAlgoRandomAccessOnly<ExceptionTester>(policy, _BeginRaw, _EndRaw);

			if (_Exit) return;

			std::vector<int> _Vec;
			std::list<int> _List;

			InstantiateAlgo<EmptyTester>(par, _BeginRaw, _EndRaw);
			InstantiateAlgo<EmptyTester>(par, std::begin(_Vec), std::end(_Vec));
			InstantiateAlgo<EmptyTester>(par, std::begin(_List), std::end(_List));

			InstantiateAlgoRandomAccessOnly<EmptyTester>(par, _BeginRaw, _EndRaw);
			InstantiateAlgoRandomAccessOnly<EmptyTester>(par, std::begin(_Vec), std::end(_Vec));

			execution_policy ex(par);
			InstantiateAlgo<EmptyTester>(ex, _BeginRaw, _EndRaw);
			InstantiateAlgo<EmptyTester>(ex, std::begin(_Vec), std::end(_Vec));
			InstantiateAlgo<EmptyTester>(ex, std::begin(_List), std::end(_List));

			InstantiateAlgoRandomAccessOnly<EmptyTester>(ex, _BeginRaw, _EndRaw);
			InstantiateAlgoRandomAccessOnly<EmptyTester>(ex, std::begin(_Vec), std::end(_Vec));
		}

		template<typename _Policy>
		bool is_policy(typename details::_enable_if_policy<_Policy, _Policy>::type) {
			return true;
		}

		template<typename _Policy>
		bool is_policy(_Policy) {
			return false;
		}

		template<typename _Policy>
		bool is_parallel_policy(typename details::_enable_if_parallel<_Policy, _Policy>::type) {
			return true;
		}

		template<typename _Policy>
		bool is_parallel_policy(_Policy) {
			return false;
		}

		class seq_priv_execution_policy : public sequential_execution_policy {};
		class par_priv_execution_policy : public parallel_execution_policy {};
		class vec_priv_execution_policy : public vector_execution_policy {};

		TEST_METHOD(InstantiatePolicyEnable)
		{
			// Positive test cases
			Assert::IsTrue(is_policy<sequential_execution_policy>(seq));
			Assert::IsTrue(is_policy<parallel_execution_policy>(par));
			Assert::IsTrue(is_policy<vector_execution_policy>(vec));

			Assert::IsTrue(is_policy<seq_priv_execution_policy>(seq_priv_execution_policy()));
			Assert::IsTrue(is_policy<par_priv_execution_policy>(par_priv_execution_policy()));
			Assert::IsTrue(is_policy<vec_priv_execution_policy>(vec_priv_execution_policy()));
			// Negative test cases for policy
			Assert::IsFalse(is_policy(int{ 0 }));
			Assert::IsFalse(is_policy(bool{ true }));

			Assert::IsFalse(is_parallel_policy<sequential_execution_policy>(seq));
			Assert::IsTrue(is_parallel_policy<parallel_execution_policy>(par));
			Assert::IsTrue(is_parallel_policy<vector_execution_policy>(vec));

			Assert::IsFalse(is_parallel_policy<seq_priv_execution_policy>(seq_priv_execution_policy()));
			Assert::IsTrue(is_parallel_policy<par_priv_execution_policy>(par_priv_execution_policy()));
			Assert::IsTrue(is_parallel_policy<vec_priv_execution_policy>(vec_priv_execution_policy()));
		}

		template<typename _ExPolicy>
		void CheckPolicy(_ExPolicy _Policy, bool _Parallel)
		{
			combinable<int> data([]{ return 0; });
			execution_policy ex(_Policy);
			std::vector<int> vec(1024);

			for_each(ex, std::begin(vec), std::end(vec), [&data](int){ auto &_Val = data.local(); _Val = 1; });

			auto _Sum = data.combine([](int _Val, int _Val2){ return _Val + _Val2; });

			if (_Parallel) Assert::IsTrue(_Sum > 1);
			else Assert::AreEqual(1, _Sum);
		}

		TEST_METHOD(ExecutionPolicyDynmicDispatch)
		{
			CheckPolicy(seq, false);

			if (std::thread::hardware_concurrency() > 1) {
				// Note - there is possibility that this test will result in false positive if all the pieces of work will work on one thread
				CheckPolicy(par, true);
				CheckPolicy(vec, true);
			}

			bool _Exception = false;
			try {
				MyPolicy _Policy;
				CheckPolicy(_Policy, false);
			}
			catch (std::invalid_argument&) {
				_Exception = true;
			}

			Assert::AreEqual(true, _Exception);
		}
	};
} //ParallelSTL_Tests
