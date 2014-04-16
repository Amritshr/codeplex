#include "stdafx.h"
#include <experimental\execution_policy>

namespace ParallelSTL_Tests
{
	TEST_CLASS(execution_policy_tests)
	{
		template<typename _ExPolicy>
		void PolicyTestImpl()
		{
			// Compile time check
			_ExPolicy policy;
			_ExPolicy policy2(policy);
			policy = policy2;

			std::swap(policy, policy2);

			static_assert(is_execution_policy<_ExPolicy>::value, "This type must be marked as execution policy");
		}

	public:
		TEST_METHOD(Static_ExPolicy)
		{
			PolicyTestImpl<sequential_execution_policy>();
			PolicyTestImpl<parallel_execution_policy>();
			PolicyTestImpl<vector_execution_policy>();
		}

		TEST_METHOD(Dynamic_ExPolicy)
		{
			execution_policy ex_seq(seq);
			execution_policy ex_par(par);
			execution_policy ex_vec(vec);

			Assert::IsTrue(ex_seq.type() == typeid(seq));
			Assert::IsTrue(ex_par.type() == typeid(par));
			Assert::IsTrue(ex_vec.type() == typeid(vec));

			execution_policy ex(seq);
			ex = par;
			Assert::IsTrue(ex.type() == typeid(par));

			ex = seq;
			Assert::IsTrue(ex.type() == typeid(seq));

			ex = vec;
			Assert::IsTrue(ex.type() == typeid(vec));

			ex = ex; // TEST assigment to itself

			ex.get<sequential_execution_policy>();
			ex.get<parallel_execution_policy>();
			ex.get<vector_execution_policy>();

			std::swap(ex_seq, ex_par);
			Assert::IsTrue(ex_seq.type() == typeid(parallel_execution_policy));
			Assert::IsNotNull(ex_seq.get<parallel_execution_policy>());
			Assert::IsNull(ex_seq.get<sequential_execution_policy>());

			Assert::IsTrue(ex_par.type() == typeid(sequential_execution_policy));
			Assert::IsNotNull(ex_par.get<sequential_execution_policy>());
			Assert::IsNull(ex_par.get<parallel_execution_policy>());
		}
	}; // TEST_CLASS(execution_policy)
} // namespace ParallelSTL_Tests