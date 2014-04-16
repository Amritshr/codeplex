#include "stdafx.h"

#include <experimental\exception>

namespace ParallelSTL_Tests
{
	TEST_CLASS(exception_list_tests)
	{
		const int MAX_EXCEPTIONS = 50;

		exception_list_tests& operator=(const exception_list_tests &) {}
	public:
		TEST_METHOD(ExceptionList)
		{
			std::list<exception_ptr> known_exceptions;
			for (int i = 0; i < rand() % MAX_EXCEPTIONS; i++) {
				try {
					throw std::invalid_argument("Test exception");
				}
				catch (...) {
					known_exceptions.push_back(std::current_exception());
				}
			}

			std::list<exception_ptr> known_exceptions_copy(std::begin(known_exceptions), std::end(known_exceptions));
			exception_list ex_list(std::move(known_exceptions));

			Assert::AreEqual(ex_list.size(), known_exceptions_copy.size());
			Assert::AreEqual(size_t{ 0 }, known_exceptions.size());

			auto known_iter = std::begin(known_exceptions_copy);
			for (auto iter = std::begin(ex_list); iter != std::end(ex_list); ++iter, ++known_iter)
				Assert::IsTrue(*iter == *known_iter);

			try {
				throw std::invalid_argument("Test exception");
			}
			catch (...) {
				exception_list ex_list2(std::move(std::current_exception()));
				Assert::AreEqual(size_t{ 1 }, ex_list2.size());
			}
		}
	}; // TEST_CLASS(exception_list_tests)
} // namespace ParallelSTL_Tests
