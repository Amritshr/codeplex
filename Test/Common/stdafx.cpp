#include "stdafx.h"

namespace
{
	void _dummy()
	{
		// This is a workaround for multiple warnings C4505 reported in CppUnitTestFramework under /W4.

		using namespace Microsoft::VisualStudio::CppUnitTestFramework;

		ToString<bool>(bool{});
		ToString<int>(int{});
		ToString<long>(long{});
		ToString<short>(short{});
		ToString<char>(char{});
		ToString<wchar_t>(wchar_t{});
		ToString<signed char>(signed char{});
		ToString<unsigned int>(unsigned int{});
		ToString<unsigned long>(unsigned long{});
		ToString<unsigned long long>(unsigned long long{});
		ToString<unsigned char>(unsigned char{});
		ToString<std::string>(std::string{});
		ToString<std::wstring>(std::wstring{});
		ToString<double>(double{});
		ToString<float>(float{});

		ToString<bool>(static_cast<bool*>(nullptr));
		ToString<int>(static_cast<int*>(nullptr));
		ToString<long>(static_cast<long*>(nullptr));
		ToString<short>(static_cast<short*>(nullptr));
		ToString<signed char>(static_cast<signed char*>(nullptr));
		ToString<unsigned int>(static_cast<unsigned int*>(nullptr));
		ToString<unsigned long>(static_cast<unsigned long*>(nullptr));
		ToString<unsigned long long>(static_cast<unsigned long long*>(nullptr));
		ToString<unsigned char>(static_cast<unsigned char*>(nullptr));
		ToString<char>(static_cast<char*>(nullptr));
		ToString<wchar_t>(static_cast<wchar_t*>(nullptr));
		ToString<double>(static_cast<double*>(nullptr));
		ToString<float>(static_cast<float*>(nullptr));
		ToString<void>(static_cast<void*>(nullptr));

		ToString<bool>(static_cast<const bool*>(nullptr));
		ToString<int>(static_cast<const int*>(nullptr));
		ToString<long>(static_cast<const long*>(nullptr));
		ToString<short>(static_cast<const short*>(nullptr));
		ToString<signed char>(static_cast<const signed char*>(nullptr));
		ToString<unsigned int>(static_cast<const unsigned int*>(nullptr));
		ToString<unsigned long>(static_cast<const unsigned long*>(nullptr));
		ToString<unsigned long long>(static_cast<const unsigned long long*>(nullptr));
		ToString<unsigned char>(static_cast<const unsigned char*>(nullptr));
		ToString<char>(static_cast<const char*>(nullptr));
		ToString<wchar_t>(static_cast<const wchar_t*>(nullptr));
		ToString<double>(static_cast<const double*>(nullptr));
		ToString<float>(static_cast<const float*>(nullptr));
		ToString<void>(static_cast<const void*>(nullptr));
	}
}
