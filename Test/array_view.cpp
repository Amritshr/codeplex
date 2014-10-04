#include "stdafx.h"

#include <experimental/array_view>

using namespace std::experimental::D4087;

namespace ParallelSTL_Tests
{
	TEST_CLASS(ArrayView)
	{
		template <typename T>
		void check_av_cv_ctor(const T& av)
		{
			Assert::IsTrue(av.bounds() == bounds<1>{ 1 });
			Assert::IsTrue(av[0] == 5);
		}

		TEST_METHOD(ArrayViewConstructors)
		{
			// Check default constructor
			{
				array_view<int> av1;
				Assert::IsTrue(av1.bounds() == bounds<1>{ 0 });
				Assert::IsTrue(av1.data() == nullptr);

				array_view<float, 2> av2;
				Assert::IsTrue((av2.bounds() == bounds<2>{ 0, 0 }));
				Assert::IsTrue((av2.data() == nullptr));
			}

			// Check rank 1 Viewable constructor
			{
				std::vector<int> vec1 = { 1, 2, 3 };
				array_view<int, 1> av1{ vec1 }; // T -> T
				Assert::IsTrue(av1.bounds() == bounds<1>{ 3 });
				Assert::IsTrue(av1[0] == 1 && av1[2] == 3);

				const std::vector<float> vec2 = { 2.2f };
				array_view<const float> av2{ vec2 }; // const T -> const T;
				Assert::IsTrue(av2.bounds() == bounds<1>{ 1 });
				Assert::IsTrue(av2[0] == 2.2f);

				std::vector<double> vec3 = { 3.14 };
				array_view<const double> av3{ vec3 }; // T -> const T
				Assert::IsTrue(av3.bounds() == bounds<1>{ 1 });
				Assert::IsTrue(av3[0] == 3.14);
			}

			// Check rank 1 array_view constructor (flattening)
			{
				std::vector<int> vec4 = { 1, 2, 3, 4 };
				array_view<int, 2> src4{ { 2, 2 }, vec4 };
				array_view<int> av4{ src4 }; // 2D -> 1D
				Assert::IsTrue(av4.bounds() == bounds<1>{ 4 });
				Assert::IsTrue(av4[0] == 1 && av4[2] == 3);
			}

			// Check array constructor
			{
				int arr1[4] = { 1, 2, 3, 4 };
				array_view<int, 1> av1{ arr1 }; // T -> T
				Assert::IsTrue(av1.bounds() == bounds<1>{ 4 });
				Assert::IsTrue(av1[0] == 1 && av1[3] == 4);

				const double arr2[3][2] = { { 1.1, 2.2 }, { 3.3, 4.4 }, { 5.5, 6.6 } };
				array_view<const double, 2> av2{ arr2 }; // const T -> const T
				Assert::IsTrue((av2.bounds() == bounds<2>{ 3, 2 }));
				Assert::IsTrue((av2[{0, 0}] == 1.1 && av2[{2, 0}] == 5.5));

				char arr3[3][1][2] = { { { 0, 1 } }, { { 2, 3 } }, { { 4, 5 } } };
				array_view<const char, 3> av3{ arr3 }; // T -> const T
				Assert::IsTrue((av3.bounds() == bounds<3>{ 3, 1, 2 }));
				Assert::IsTrue((av3[{0, 0, 1}] == 1 && av3[{2, 0, 1}] == 5));
			}

			// Check Viewable constructor
			{
				std::vector<int> vec = { 1, 2, 3, 4, 5, 6, 7, 8 };
				const std::vector<int> cvec = vec;

				array_view<int, 1> av1{ { 1 }, vec }; // T -> T
				Assert::IsTrue(av1.bounds() == bounds<1>{ 1 });
				Assert::IsTrue(av1[0] == 1);

				array_view<const int, 2> av2{ { 3, 2 }, cvec }; // const T -> const T
				Assert::IsTrue((av2.bounds() == bounds<2>{ 3, 2 }));
				Assert::IsTrue((av2[{0, 0}] == 1 && av2[{2, 0}] == 5));

				array_view<const int, 1> av3{ { 1 }, vec }; // T -> const T
				Assert::IsTrue(av3.bounds() == bounds<1>{ 1 });
				Assert::IsTrue(av3[0] == 1);

				array_view<int, 2> src4{ { 4, 2 }, vec };
				array_view<int, 2> av4{ { 2, 4 }, src4 }; // array_view<N> -> array_view<N>
				Assert::IsTrue((av4.bounds() == bounds<2>{ 2, 4 }));
				Assert::IsTrue((av4[{1, 3}] == 8));

				array_view<int, 2> src5{ { 4, 2 }, vec };
				array_view<int, 3> av5{ { 2, 2, 2 }, src5 }; // array_view<N> -> array_view<K>
				Assert::IsTrue((av5.bounds() == bounds<3>{ 2, 2, 2 }));
				Assert::IsTrue((av5[{1, 1, 1}] == 8));
			}

			// Check pointer constructor
			{
				std::vector<int> vec1 = { 1, 2, 3 };
				array_view<int, 1> av1{ 3, vec1.data() }; // T -> T
				Assert::IsTrue(av1.bounds() == bounds<1>{ 3 });
				Assert::IsTrue(av1[0] == 1 && av1[2] == 3);

				const std::vector<int> vec2 = { 1, 2, 3, 4, 5, 6 };
				array_view<const int, 2> av2{ { 3, 2 }, vec2.data() }; // const T -> const T
				Assert::IsTrue((av2.bounds() == bounds<2>{ 3, 2 }));
				Assert::IsTrue((av2[{0, 0}] == 1 && av2[{2, 0}] == 5));

				std::vector<int> vec3 = { 1 };
				array_view<const int, 1> av3{ 1, vec3.data() }; // T -> const T
				Assert::IsTrue(av3.bounds() == bounds<1>{ 1 });
				Assert::IsTrue(av3[0] == 1);
			}

			// Check cv-casting constructor
			{
				int arr[1] = { 5 };

				// From non-cv-qualified source
				{
					const array_view<int> src{ arr };

					array_view<const int> av_c{ src };
					check_av_cv_ctor(av_c);
					array_view<volatile int> av_v{ src };
					check_av_cv_ctor(av_v);
					array_view<const volatile int> av_cv{ src };
					check_av_cv_ctor(av_cv);
				}

				// From const-qualified source
				{
					const array_view<const int> src{ arr };

					array_view<const volatile int> av_cv{ src };
					check_av_cv_ctor(av_cv);
				}

				// From volatile-qualified source
				{
					const array_view<volatile int> src{ arr };

					array_view<const volatile int> av_cv{ src };
					check_av_cv_ctor(av_cv);
				}

				static_assert(std::is_convertible<const array_view<int>, array_view<const int>>::value, "ctor is not implicit!");
			}

			// Check copy constructor
			{
				std::vector<int> vec1 = { 3, 4 };
				const array_view<int, 1> src1{ { 2 }, vec1 };
				array_view<int, 1> av1{ src1 };
				Assert::IsTrue(av1.bounds() == bounds<1>{ 2 });
				Assert::IsTrue(av1[0] == 3);

				const std::vector<int> vec2 = { 1, 2, 3, 4, 5, 6 };
				const array_view<const int, 2> src2{ { 3, 2 }, vec2 };
				array_view<const int, 2> av2{ src2 };
				Assert::IsTrue((av2.bounds() == bounds<2>{ 3, 2 }));
				Assert::IsTrue((av2[{0, 0}] == 1 && av2[{2, 0}] == 5));
			}

			// Check const-casting assignment operator
			{
				int arr1[1] = { 1 };
				int arr2[2] = { 2, 3 };
				const array_view<int, 2> src{ { 1, 1 }, arr1 };
				array_view<const int, 2> av{ { 1, 2 }, arr2 };
				array_view<const int, 2>& av_ref = (av = src);
				Assert::IsTrue((av.bounds() == bounds<2>{ 1, 1 }));
				Assert::IsTrue((av[{0, 0}] == 1));
				Assert::IsTrue(&av_ref == &av);
			}

			// Check copy assignment operator
			{
				std::vector<int> vec1 = { 3, 4 };
				std::vector<int> vec1b = { 0 };
				const array_view<int, 1> src1{ { 2 }, vec1 };
				array_view<int, 1> av1{ vec1b };
				array_view<int, 1>& av1_ref = (av1 = src1);
				Assert::IsTrue(av1.bounds() == bounds<1>{ 2 });
				Assert::IsTrue(av1[0] == 3);
				Assert::IsTrue(&av1_ref == &av1);

				const vector<int> vec2 = { 1, 2, 3, 4, 5, 6 };
				const vector<int> vec2b = { 0 };
				const array_view<const int, 2> src2{ { 3, 2 }, vec2 };
				array_view<const int, 2> av2{ { 1, 1 }, vec2b };
				array_view<const int, 2>& av2_ref = (av2 = src2);
				Assert::IsTrue((av2.bounds() == bounds<2>{ 3, 2 }));
				Assert::IsTrue((av2[{0, 0}] == 1 && av2[{2, 0}] == 5));
				Assert::IsTrue(&av2_ref == &av2);
			}

			// Check various constructors for array_view conversions
			// Here we're just verifying that they compile and have the right noexcept specification.
			{
				using T = int;
				using U = const int;

				// From non-const objects
				{
					array_view<T, 1> avT1;
					array_view<T, 2> avT2;

					auto avT1_avT1 = array_view<T, 1>{avT1};
					TEST_NOEXCEPT((array_view<T, 1>{avT1}), true);

					auto avT2_avT1 = array_view<T, 1>{avT2};
					TEST_NOEXCEPT((array_view<T, 1>{avT2}), true);

					auto avT2_avT2 = array_view<T, 2>{avT2};
					TEST_NOEXCEPT((array_view<T, 2>{avT2}), true);

					auto avT1_avU1 = array_view<U, 1>{avT1};
					TEST_NOEXCEPT((array_view<U, 1>{avT1}), true);

					auto avT2_avU1 = array_view<U, 1>{avT2};
					TEST_NOEXCEPT((array_view<U, 1>{avT2}), true);

					auto avT2_avU2 = array_view<U, 2>{avT2};
					TEST_NOEXCEPT((array_view<U, 2>{avT2}), true);
				}

				// From const objects
				{
					const array_view<T, 1> avT1;
					const array_view<T, 2> avT2;

					auto avT1_avT1 = array_view<T, 1>{avT1};
					TEST_NOEXCEPT((array_view<T, 1>{avT1}), true);

					auto avT2_avT1 = array_view<T, 1>{avT2};
					TEST_NOEXCEPT((array_view<T, 1>{avT2}), true);

					auto avT2_avT2 = array_view<T, 2>{avT2};
					TEST_NOEXCEPT((array_view<T, 2>{avT2}), true);

					auto avT1_avU1 = array_view<U, 1>{avT1};
					TEST_NOEXCEPT((array_view<U, 1>{avT1}), true);

					auto avT2_avU1 = array_view<U, 1>{avT2};
					TEST_NOEXCEPT((array_view<U, 1>{avT2}), true);

					auto avT2_avU2 = array_view<U, 2>{avT2};
					TEST_NOEXCEPT((array_view<U, 2>{avT2}), true);
				}
			}
		}

		TEST_METHOD(ArrayViewSubscript)
		{
			std::vector<int> data = { 1, 2, 3, 4 };
			const array_view<int, 2> av{ { 2, 2 }, data };
			av[{1, 0}] = -42;
			Assert::IsTrue(data[0] == 1 && data[1] == 2 && data[2] == -42 && data[3] == 4);
			Assert::IsTrue((av[{0, 0}] == 1 && av[{0, 1}] == 2 && av[{1, 0}] == -42 && av[{1, 1}] == 4));
		}

		TEST_METHOD(ArrayViewSlice)
		{
			std::vector<int> data(5 * 10);
			std::iota(begin(data), end(data), 0);
			const array_view<int, 2> av({ 5, 10 }, data);
			const array_view<const int, 2> cav({ 5, 10 }, data);

			array_view<int, 1> av_sl = av[2];
			Assert::IsTrue(av_sl[0] == 20);
			Assert::IsTrue(av_sl[9] == 29);

			array_view<const int, 1> cav_sl = av[3];
			Assert::IsTrue(cav_sl[0] == 30);
			Assert::IsTrue(cav_sl[9] == 39);

			Assert::IsTrue(av[4][0] == 40);
			Assert::IsTrue(av[4][9] == 49);
		}

		TEST_METHOD(ArrayViewSection)
		{
			std::vector<int> data(5 * 10);
			std::iota(begin(data), end(data), 0);
			const array_view<int, 2> av({ 5, 10 }, data);

			strided_array_view<int, 2> av_section_1 = av.section({ 1, 2 }, { 3, 4 });
			Assert::IsTrue((av_section_1[{0, 0}] == 12));
			Assert::IsTrue((av_section_1[{0, 1}] == 13));
			Assert::IsTrue((av_section_1[{1, 0}] == 22));
			Assert::IsTrue((av_section_1[{2, 3}] == 35));

			strided_array_view<int, 2> av_section_2 = av_section_1.section({ 1, 2 });
			Assert::IsTrue((av_section_2[{0, 0}] == 24));
			Assert::IsTrue((av_section_2[{0, 1}] == 25));
			Assert::IsTrue((av_section_2[{1, 0}] == 34));
		}

		template <typename T>
		void check_sav_av_ctor(const T& sav)
		{
			Assert::IsTrue(sav.bounds() == bounds<1>{ 2 });
			Assert::IsTrue(sav.stride() == index<1>{ 1 });
			Assert::IsTrue(sav[1] == 2);
		}

		TEST_METHOD(StridedArrayViewConstructors)
		{
			// Check default constructor
			{
				strided_array_view<int> sav1;
				Assert::IsTrue(sav1.bounds() == bounds<1>{ 0 });

				strided_array_view<float, 2> sav2;
				Assert::IsTrue((sav2.bounds() == bounds<2>{ 0, 0 }));
			}

			// Check stride constructor
			{
				int arr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };
				const int carr[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9 };

				strided_array_view<int> sav1{ 9, 1, arr }; // T -> T
				Assert::IsTrue(sav1.bounds() == bounds<1>{ 9 });
				Assert::IsTrue(sav1.stride() == index<1>{ 1 });
				Assert::IsTrue(sav1[0] == 1 && sav1[8] == 9);

				strided_array_view<const int> sav2{ bounds<1>{ 4 }, index<1>{ 2 }, carr }; // const T -> const T
				Assert::IsTrue(sav2.bounds() == bounds<1>{ 4 });
				Assert::IsTrue(sav2.stride() == index<1>{ 2 });
				Assert::IsTrue(sav2[0] == 1 && sav2[3] == 7);

				strided_array_view<int, 2> sav3{ { 2, 2 }, { 6, 2 }, arr }; // T -> const T
				Assert::IsTrue((sav3.bounds() == bounds<2>{ 2, 2 }));
				Assert::IsTrue((sav3.stride() == index<2>{ 6, 2 }));
				Assert::IsTrue((sav3[{0, 0}] == 1 && sav3[{0, 1}] == 3 && sav3[{1, 0}] == 7));
			}

			// Check array_view constructor
			{
				int arr[] = { 1, 2 };

				// From non-cv-qualified source
				{
					const array_view<int> src{ arr };

					strided_array_view<int> sav{ src };
					check_sav_av_ctor(sav);
					strided_array_view<const int> sav_c{ src };
					check_sav_av_ctor(sav_c);
					strided_array_view<volatile int> sav_v{ src };
					check_sav_av_ctor(sav_v);
					strided_array_view<const volatile int> sav_cv{ src };
					check_sav_av_ctor(sav_cv);
				}

				// From const-qualified source
				{
					const array_view<const int> src{ arr };

					strided_array_view<const int> sav_c{ src };
					check_sav_av_ctor(sav_c);
					strided_array_view<const volatile int> sav_cv{ src };
					check_sav_av_ctor(sav_cv);
				}

				// From volatile-qualified source
				{
					const array_view<volatile int> src{ arr };

					strided_array_view<volatile int> sav_v{ src };
					check_sav_av_ctor(sav_v);
					strided_array_view<const volatile int> sav_cv{ src };
					check_sav_av_ctor(sav_cv);
				}

				// From cv-qualified source
				{
					const array_view<const volatile int> src{ arr };

					strided_array_view<const volatile int> sav_cv{ src };
					check_sav_av_ctor(sav_cv);
				}
			}

			// Check const-casting constructor
			{
				int arr[2] = { 4, 5 };
				const strided_array_view<int> src{ 2, 1, arr };
				strided_array_view<const int> sav{ src };
				Assert::IsTrue(sav.bounds() == bounds<1>{ 2 });
				Assert::IsTrue(sav.stride() == index<1>{ 1 });
				Assert::IsTrue(sav[1] == 5);
				
				static_assert(std::is_convertible<const strided_array_view<int>, strided_array_view<const int>>::value, "ctor is not implicit!");
			}

			// Check copy constructor
			{
				int arr1[2] = { 3, 4 };
				const strided_array_view<int, 1> src1{ 2, 1, arr1 };
				strided_array_view<int, 1> sav1{ src1 };
				Assert::IsTrue(sav1.bounds() == bounds<1>{ 2 });
				Assert::IsTrue(sav1.stride() == index<1>{ 1 });
				Assert::IsTrue(sav1[0] == 3);

				int arr2[6] = { 1, 2, 3, 4, 5, 6 };
				const strided_array_view<const int, 2> src2{ { 3, 2 }, { 2, 1 }, arr2 };
				strided_array_view<const int, 2> sav2{ src2 };
				Assert::IsTrue((sav2.bounds() == bounds<2>{ 3, 2 }));
				Assert::IsTrue((sav2.stride() == index<2>{ 2, 1 }));
				Assert::IsTrue((sav2[{0, 0}] == 1 && sav2[{2, 0}] == 5));
			}

			// Check array_view assignment operator
			{
				int arr[] = { 1, 2 };
				int arr2[] = { 9 };
				const array_view<int> av{ arr };
				const array_view<const int> cav{ arr };

				strided_array_view<int> sav1{ 1, 1, arr2 };
				strided_array_view<int>& sav1_ref = (sav1 = av); // T -> T
				Assert::IsTrue(sav1.bounds() == bounds<1>{ 2 });
				Assert::IsTrue(sav1.stride() == index<1>{ 1 });
				Assert::IsTrue(sav1[1] == 2);
				Assert::IsTrue(&sav1_ref == &sav1);

				strided_array_view<const int> sav2{ 1, 1, arr2 };
				strided_array_view<const int>& sav2_ref = (sav2 = cav); // const T -> const T
				Assert::IsTrue(sav2.bounds() == bounds<1>{ 2 });
				Assert::IsTrue(sav2.stride() == index<1>{ 1 });
				Assert::IsTrue(sav2[1] == 2);
				Assert::IsTrue(&sav2_ref == &sav2);

				strided_array_view<const int> sav3{ 1, 1, arr2 };
				strided_array_view<const int>& sav3_ref = (sav3 = av); // T -> const T
				(sav3_ref);
				Assert::IsTrue(sav3.bounds() == bounds<1>{ 2 });
				Assert::IsTrue(sav3.stride() == index<1>{ 1 });
				Assert::IsTrue(sav3[1] == 2);
				Assert::IsTrue(&sav2_ref == &sav2);
			}

			// Check const-casting assignment operator
			{
				int arr1[2] = { 1, 2 };
				int arr2[6] = { 3, 4, 5, 6, 7, 8 };
				const strided_array_view<int> src{ 2, 1, arr1 };
				strided_array_view<const int> sav{ 3, 2, arr2 };
				strided_array_view<const int>& sav_ref = (sav = src);
				Assert::IsTrue(sav.bounds() == bounds<1>{ 2 });
				Assert::IsTrue(sav.stride() == index<1>{ 1 });
				Assert::IsTrue(sav[0] == 1);
				Assert::IsTrue(&sav_ref == &sav);
			}

			// Check copy assignment operator
			{
				int arr1[2] = { 3, 4 };
				int arr1b[1] = { 0 };
				const strided_array_view<int, 1> src1{ 2, 1, arr1 };
				strided_array_view<int, 1> sav1{ 1, 1, arr1b };
				strided_array_view<int, 1>& sav1_ref = (sav1 = src1);
				Assert::IsTrue(sav1.bounds() == bounds<1>{ 2 });
				Assert::IsTrue(sav1.stride() == index<1>{ 1 });
				Assert::IsTrue(sav1[0] == 3);
				Assert::IsTrue(&sav1_ref == &sav1);

				const int arr2[6] = { 1, 2, 3, 4, 5, 6 };
				const int arr2b[1] = { 0 };
				const strided_array_view<const int, 2> src2{ { 3, 2 }, { 2, 1 }, arr2 };
				strided_array_view<const int, 2> sav2{ { 1, 1 }, { 1, 1 }, arr2b };
				strided_array_view<const int, 2>& sav2_ref = (sav2 = src2);
				Assert::IsTrue((sav2.bounds() == bounds<2>{ 3, 2 }));
				Assert::IsTrue((sav2.stride() == index<2>{ 2, 1 }));
				Assert::IsTrue((sav2[{0, 0}] == 1 && sav2[{2, 0}] == 5));
				Assert::IsTrue(&sav2_ref == &sav2);
			}
		}

		TEST_METHOD(StridedArrayViewSlice)
		{
			std::vector<int> data(5 * 10);
			std::iota(begin(data), end(data), 0);
			const array_view<int, 2> src{ { 5, 10 }, data };

			const strided_array_view<int, 2> sav{ src };
			const strided_array_view<const int, 2> csav{ src };

			strided_array_view<int, 1> sav_sl = sav[2];
			Assert::IsTrue(sav_sl[0] == 20);
			Assert::IsTrue(sav_sl[9] == 29);

			strided_array_view<const int, 1> csav_sl = sav[3];
			Assert::IsTrue(csav_sl[0] == 30);
			Assert::IsTrue(csav_sl[9] == 39);

			Assert::IsTrue(sav[4][0] == 40);
			Assert::IsTrue(sav[4][9] == 49);
		}

		TEST_METHOD(StridedArrayViewColumnMajor)
		{
			// strided_array_view may be used to accomodate more peculiar
			// use cases, such as column-major multidimensional array
			// (aka. "FORTRAN" layout).

			int cm_array[3 * 5] = {
				1, 4, 7, 10, 13,
				2, 5, 8, 11, 14,
				3, 6, 9, 12, 15
			};
			strided_array_view<int, 2> cm_sav{ { 5, 3 }, { 1, 5 }, cm_array };
			
			// Accessing elements
			Assert::IsTrue((cm_sav[{0, 0}] == 1));
			Assert::IsTrue((cm_sav[{0, 1}] == 2));
			Assert::IsTrue((cm_sav[{1, 0}] == 4));
			Assert::IsTrue((cm_sav[{4, 2}] == 15));

			// Slice
			strided_array_view<int, 1> cm_sl = cm_sav[3];
			Assert::IsTrue(cm_sl[0] == 10);
			Assert::IsTrue(cm_sl[1] == 11);
			Assert::IsTrue(cm_sl[2] == 12);

			// Section
			strided_array_view<int, 2> cm_sec = cm_sav.section({ 2, 1 });
			Assert::IsTrue((cm_sec.bounds() == bounds<2>{3, 2}));
			Assert::IsTrue((cm_sec[{0, 0}] == 8));
			Assert::IsTrue((cm_sec[{0, 1}] == 9));
			Assert::IsTrue((cm_sec[{1, 0}] == 11));
			Assert::IsTrue((cm_sec[{2, 1}] == 15));
		}

		TEST_METHOD(ArrayViewAlgorithms)
		{
			const auto M = 32;
			const auto W = 64;
			const auto N = 96;

			const std::vector<float> va(M * W);
			const std::vector<float> vb(W * N);
			std::vector<float> vc(M * N);

			array_view<const float, 2> a{ { M, W }, va };
			array_view<const float, 2> b{ { W, N }, vb };
			array_view<float, 2> c{ { M, N }, vc };

			std::for_each(begin(c.bounds()), end(c.bounds()), [&](index<2> idx)
			{
				auto row = idx[0]; auto col = idx[1];
				float sum = 0.0f;
				for (int i = 0; i < W; i++)
					sum += a[{row, i}] * b[{i, col}];
				c[idx] = sum;
			});
		}

		TEST_METHOD(ArrayViewAlgorithmsExample)
		{
			const auto M = 32;
			const auto N = 64; // Note: In real code M and N would likely not be constant.

			auto vA = vector<float>(M * N);
			auto vB = vector<float>(N);      // Likewise, vA and vB would be filled with data.
			auto vC = vector<float>(M * N);

			auto a = array_view<float, 2>{{ M, N }, vA }; // An MxN view on vA.
			auto b = array_view<float>{ vB };             // A view on vB.
			auto c = array_view<float, 2>{{ M, N }, vC }; // An MxN view on vC.

			bounds_iterator<2> first = begin(c.bounds()); // Named object for clarity.
			bounds_iterator<2> last = end(c.bounds());
			for_each(
				first, last,
				[&](index<2> idx)
				// or shortly:  for(auto idx : c.bounds())
			{
				float sum = 0.0f;
				for (auto i = 0; i < N; i++)
					sum += a[{idx[0], i}] * b[i];
				c[idx] = sum;
			});
		}
	};
} // ParallelSTL_Tests