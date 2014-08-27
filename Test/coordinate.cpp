#include "stdafx.h"

#include <experimental/coordinate>

using namespace std::experimental::D4087;

namespace ParallelSTL_Tests
{
	TEST_CLASS(Coordinate)
	{
		TEST_METHOD(IndexOneDimension)
		{
			// Check rank
			{
				static_assert(std::is_same<decltype(index<1>::rank), const int>::value, "rank != const int");
				static_assert(index<1>::rank == 1, "rank != 1");
				const int ce_rank = index<1>::rank;
				(ce_rank);
			}

			// Check type aliases
			{
				static_assert(std::is_same<index<1>::reference, ptrdiff_t&>::value, "reference != ptrdiff_t&");
				static_assert(std::is_same<index<1>::const_reference, const ptrdiff_t&>::value, "const_reference != const ptrdiff_t&");
				static_assert(std::is_same<index<1>::size_type, size_t>::value, "size_type != size_t");
				static_assert(std::is_same<index<1>::value_type, ptrdiff_t>::value, "value_type != ptrdiff_t");
			}

			// Check default constructor
			{
				index<1> idx;
				Assert::IsTrue(idx[0] == 0);

				int32_t mess[sizeof(index<1>) / 4 + 1];
				std::fill(std::begin(mess), std::end(mess), 0xffffffff);
				index<1>* pidx = new(mess)index<1>{};
				Assert::IsTrue((*pidx)[0] == 0);
			}

			// Check value_type constructor
			{
				index<1> idx{ 99 };
				Assert::IsTrue(idx[0] == 99);

				static_assert(std::is_convertible<ptrdiff_t, index<1>>::value, "ctor is not implicit!");
			}

			// Check initializer_list constructor
			{
				index<1> idx{ 3 };
				Assert::IsTrue(idx[0] == 3);

				const std::initializer_list<index<1>::value_type> il{ 1 };
				index<1> idx4(il);
				Assert::IsTrue(idx4[0] == 1);
			}

			// Check copy constructor
			{
				index<1> src{ 4 };
				index<1> idx{ src };
				Assert::IsTrue(idx[0] == 4);
			}

			// Check assignment operator
			{
				const index<1> src{ 8 };
				index<1> idx;

				idx = src;
				Assert::IsTrue(idx[0] == 8);

				idx = idx;
				Assert::IsTrue(idx[0] == 8);
			}

			// Check subscript operator
			{
				index<1> idx{ 1 };
				idx[0] = 11;
				Assert::IsTrue(idx[0] == 11);

				const index<1> cidx{ 4 };
				Assert::IsTrue(cidx[0] == 4);
			}

			// Check equality and inequality operators
			{
				const index<1> idx1{ 1 };
				const index<1> idx2{ 2 };
		
				Assert::IsTrue(idx1 == idx1 && idx2 == idx2);
				Assert::IsTrue(idx1 != idx2);
			}

			// Check unary plus and minus operators
			{
				const index<1> src{ -1 };

				index<1> idx = +src;
				Assert::IsTrue(idx[0] == -1);

				index<1> nidx = -src;
				Assert::IsTrue(nidx[0] == 1);
			}

			// Check addition and subtraction operators
			{
				const index<1> op1{ -2 };
				const index<1> op2{ 10 };

				index<1> sum = op1 + op2;
				Assert::IsTrue(sum[0] == 8);

				index<1> dif = op1 - op2;
				Assert::IsTrue(dif[0] == -12);
			}

			// Check add-equal and subtract-equal operators
			{
				const index<2> op{ 4, 6 };

				index<2> idx1{ 1, 2 };
				index<2>& sum_ref = (idx1 += op);
				Assert::IsTrue(idx1[0] == 5 && idx1[1] == 8);
				Assert::IsTrue(&sum_ref == &idx1);

				index<2> idx2{ 1, 2 };
				index<2>& dif_ref = (idx2 -= op);
				Assert::IsTrue(idx2[0] == -3 && idx2[1] == -4);
				Assert::IsTrue(&dif_ref == &idx2);
			}

			// Check increment operators
			{
				index<1> idx{ 3 };

				index<1>& pre = (++idx);
				Assert::IsTrue(idx[0] == 4);
				Assert::IsTrue(&pre == &idx);

				index<1> post = (idx++);
				Assert::IsTrue(idx[0] == 5);
				Assert::IsTrue(post[0] == 4);
			}

			// Check decrement operators
			{
				index<1> idx{ 3 };

				index<1>& pre = (--idx);
				Assert::IsTrue(idx[0] == 2);
				Assert::IsTrue(&pre == &idx);

				index<1> post = (idx--);
				Assert::IsTrue(idx[0] == 1);
				Assert::IsTrue(post[0] == 2);
			}

			// Check multiplication operators
			{
				const index<1> op{ 2 };

				index<1> prod1 = op * 2;
				Assert::IsTrue(prod1[0] == 4);

				index<1> prod2 = op * 1.5;
				Assert::IsTrue(prod2[0] == 2);

				index<1> prod3 = 10 * op;
				Assert::IsTrue(prod3[0] == 20);

				index<1> prod4 = 0.5 * op;
				Assert::IsTrue(prod4[0] == 0);
			}

			// Check division operator
			{
				const index<1> op{ 10 };

				index<1> quo1 = op / 2;
				Assert::IsTrue(quo1[0] == 5);

				index<1> quo2 = op / 1.5;
				Assert::IsTrue(quo2[0] == 10);
			}

			// Check multiply-equal operator
			{
				index<1> idx1{ 2 };
				index<1>& idx1_ref = (idx1 *= 2);
				Assert::IsTrue(idx1[0] == 4);
				Assert::IsTrue(&idx1_ref == &idx1);

				index<1> idx2{ -2 };
				index<1>& idx2_ref = (idx2 *= 1.5);
				Assert::IsTrue(idx2[0] == -2);
				Assert::IsTrue(&idx2_ref == &idx2);
			}

			// Check divide-equal operator
			{
				index<1> idx1{ 10 };
				index<1>& idx1_ref = (idx1 /= 2);
				Assert::IsTrue(idx1[0] == 5);
				Assert::IsTrue(&idx1_ref == &idx1);

				index<1> idx2{ -10 };
				index<1>& idx2_ref = (idx2 /= 1.5);
				Assert::IsTrue(idx2[0] == -10);
				Assert::IsTrue(&idx2_ref == &idx2);
			}

			// Sanity check conveniance syntax in few common operations
			{
				index<1> idx = { 1 };
				idx += {10};
				bool res = (idx == index<1>{11});
				Assert::IsTrue(res);
			}
		}

		TEST_METHOD(Index)
		{
			// Check rank
			{
				static_assert(std::is_same<decltype(index<5>::rank), const int>::value, "rank != const int");
				static_assert(index<6>::rank == 6, "rank != 6");
				const int ce_rank = index<7>::rank;
				(ce_rank);
			}

			// Check type aliases
			{
				static_assert(std::is_same<index<1>::reference, ptrdiff_t&>::value, "reference != ptrdiff_t&");
				static_assert(std::is_same<index<2>::const_reference, const ptrdiff_t&>::value, "const_reference != const ptrdiff_t&");
				static_assert(std::is_same<index<3>::size_type, size_t>::value, "size_type != size_t");
				static_assert(std::is_same<index<4>::value_type, ptrdiff_t>::value, "value_type != ptrdiff_t");
			}

			// Check default constructor
			{
				index<2> idx;
				Assert::IsTrue(idx[0] == 0 && idx[1] == 0);

				int32_t mess[sizeof(index<3>) / 4 + 1];
				std::fill(std::begin(mess), std::end(mess), 0xffffffff);
				index<3>* pidx = new(mess)index<3>{};
				Assert::IsTrue((*pidx)[0] == 0 && (*pidx)[1] == 0 && (*pidx)[2] == 0);
			}

			// Check value_type constructor
			{
				index<1> idx{ 99 };
				Assert::IsTrue(idx[0] == 99);

				static_assert(std::is_convertible<ptrdiff_t, index<1>>::value, "ctor is not implicit!");
			}

			// Check initializer_list constructor
			{
				index<3> idx{ 3, 14, 15 };
				Assert::IsTrue(idx[0] == 3 && idx[1] == 14 && idx[2] == 15);

				const std::initializer_list<index<4>::value_type> il{ 1, 2, -3, 4 };
				index<4> idx4(il);
				Assert::IsTrue(idx4[0] == 1 && idx4[1] == 2 && idx4[2] == -3 && idx4[3] == 4);
			}

			// Check copy constructor
			{
				index<2> src{ 4, 1 };
				index<2> idx{ src };
				Assert::IsTrue(idx[0] == 4 && idx[1] == 1);
			}

			// Check assignment operator
			{
				const index<2> src{ 8, -7 };
				index<2> idx;

				idx = src;
				Assert::IsTrue(idx[0] == 8 && idx[1] == -7);

				idx = idx;
				Assert::IsTrue(idx[0] == 8 && idx[1] == -7);
			}

			// Check subscript operator
			{
				index<3> idx{ 1, 2, 3 };
				idx[0] = 11;
				Assert::IsTrue(idx[0] == 11 && idx[1] == 2 && idx[2] == 3);

				const index<3> cidx{ 4, 5, 6 };
				Assert::IsTrue(cidx[0] == 4 && cidx[1] == 5 && cidx[2] == 6);
			}

			// Check equality and inequality operators
			{
				const index<2> idx1{ 1, 2 };
				const index<2> idx2{ 1, 2 };
				const index<2> idx3{ -1, 2 };
				const index<2> idx4{ 1, -2 };

				Assert::IsTrue(idx1 == idx1 && idx1 == idx2);
				Assert::IsTrue(!(idx1 == idx3) && !(idx1 == idx4));

				Assert::IsTrue(!(idx1 != idx1) && !(idx1 != idx2));
				Assert::IsTrue(idx1 != idx3 && idx1 != idx4);
			}

			// Check unary plus and minus operators
			{
				const index<3> src{ -1, 2, -3 };

				index<3> idx = +src;
				Assert::IsTrue(idx[0] == -1 && idx[1] == 2 && idx[2] == -3);

				index<3> nidx = -src;
				Assert::IsTrue(nidx[0] == 1 && nidx[1] == -2 && nidx[2] == 3);
			}

			// Check addition and subtraction operators
			{
				const index<2> op1{ -2, 3 };
				const index<2> op2{ 10, -10 };

				index<2> sum = op1 + op2;
				Assert::IsTrue(sum[0] == 8 && sum[1] == -7);

				index<2> dif = op1 - op2;
				Assert::IsTrue(dif[0] == -12 && dif[1] == 13);
			}

			// Check add-equal and subtract-equal operators
			{
				const index<2> op{ 4, 6 };

				index<2> idx1{ 1, 2 };
				index<2>& sum_ref = (idx1 += op);
				Assert::IsTrue(idx1[0] == 5 && idx1[1] == 8);
				Assert::IsTrue(&sum_ref == &idx1);

				index<2> idx2{ 1, 2 };
				index<2>& dif_ref = (idx2 -= op);
				Assert::IsTrue(idx2[0] == -3 && idx2[1] == -4);
				Assert::IsTrue(&dif_ref == &idx2);
			}

			// Check increment operators
			{
				index<1> idx{ 3 };

				index<1>& pre = (++idx);
				Assert::IsTrue(idx[0] == 4);
				Assert::IsTrue(&pre == &idx);

				index<1> post = (idx++);
				Assert::IsTrue(idx[0] == 5);
				Assert::IsTrue(post[0] == 4);
			}

			// Check decrement operators
			{
				index<1> idx{ 3 };

				index<1>& pre = (--idx);
				Assert::IsTrue(idx[0] == 2);
				Assert::IsTrue(&pre == &idx);

				index<1> post = (idx--);
				Assert::IsTrue(idx[0] == 1);
				Assert::IsTrue(post[0] == 2);
			}

			// Check multiplication operators
			{
				const index<2> op{ 2, 3 };

				index<2> prod1 = op * 2;
				Assert::IsTrue(prod1[0] == 4 && prod1[1] == 6);

				index<2> prod2 = op * 1.5;
				Assert::IsTrue(prod2[0] == 2 && prod2[1] == 3);

				index<2> prod3 = 10 * op;
				Assert::IsTrue(prod3[0] == 20 && prod3[1] == 30);

				index<2> prod4 = 0.5 * op;
				Assert::IsTrue(prod4[0] == 0 && prod4[1] == 0);
			}

			// Check division operator
			{
				const index<2> op{ 10, 7 };

				index<2> quo1 = op / 2;
				Assert::IsTrue(quo1[0] == 5 && quo1[1] == 3);

				index<2> quo2 = op / 1.5;
				Assert::IsTrue(quo2[0] == 10 && quo2[1] == 7);
			}

			// Check multiply-equal operator
			{
				index<3> idx1{ 2, 3, 4 };
				index<3>& idx1_ref = (idx1 *= 2);
				Assert::IsTrue(idx1[0] == 4 && idx1[1] == 6 && idx1[2] == 8);
				Assert::IsTrue(&idx1_ref == &idx1);

				index<3> idx2{ -2, 3, -4 };
				index<3>& idx2_ref = (idx2 *= 1.5);
				Assert::IsTrue(idx2[0] == -2 && idx2[1] == 3 && idx2[2] == -4);
				Assert::IsTrue(&idx2_ref == &idx2);
			}

			// Check divide-equal operator
			{
				index<3> idx1{ 10, 11, 12 };
				index<3>& idx1_ref = (idx1 /= 2);
				Assert::IsTrue(idx1[0] == 5 && idx1[1] == 5 && idx1[2] == 6);
				Assert::IsTrue(&idx1_ref == &idx1);

				index<3> idx2{ -10, 11, -12 };
				index<3>& idx2_ref = (idx2 /= 1.5);
				Assert::IsTrue(idx2[0] == -10 && idx2[1] == 11 && idx2[2] == -12);
				Assert::IsTrue(&idx2_ref == &idx2);
			}

			// Sanity check conveniance syntax in few common operations
			{
				index<2> idx = { 1, 2 };
				idx += {10, 100};
				bool res = (idx == index<2>{11, 102});
				Assert::IsTrue(res);
			}
		}

		// Check subscript operator
		TEST_ERROR(std::declval<T>()[0] = 1, const index<1>);

		// Check multiplication operator
		TEST_ERROR(T{} *T{}, index<2>);

		// Check division operator
		TEST_ERROR(T{} / T{}, index<2>);
		TEST_ERROR(2 / T{}, index<2>);

		// Check multiply-equal operator
		TEST_ERROR(T{} *= T{}, index<2>);

		// Check divide-equal operator
		TEST_ERROR(T{} /= T{}, index<2>);


		TEST_METHOD(Bounds)
		{
			// Check rank
			{
				static_assert(std::is_same<decltype(bounds<5>::rank), const int>::value, "rank != const int");
				static_assert(bounds<6>::rank == 6, "rank != 6");
				const int ce_rank = bounds<7>::rank;
				(ce_rank);
			}

			// Check type aliases
			{
				static_assert(std::is_same<bounds<1>::reference, ptrdiff_t&>::value, "reference != ptrdiff_t&");
				static_assert(std::is_same<bounds<2>::const_reference, const ptrdiff_t&>::value, "const_reference != const ptrdiff_t&");
				static_assert(std::is_same<bounds<3>::iterator, bounds_iterator<3>>::value, "iterator != bounds_iterator<3>");
				static_assert(std::is_same<bounds<4>::const_iterator, bounds_iterator<4>>::value, "const_iterator != bounds_iterator<4>");
				static_assert(std::is_same<bounds<5>::size_type, size_t>::value, "size_type != size_t");
				static_assert(std::is_same<bounds<6>::value_type, ptrdiff_t>::value, "value_type != ptrdiff_t");
			}

			// Check default constructor
			{
				bounds<2> bnd;
				Assert::IsTrue(bnd[0] == 0 && bnd[1] == 0);

				int32_t mess[sizeof(bounds<3>) / 4 + 1];
				std::fill(std::begin(mess), std::end(mess), 0xffffffff);
				bounds<3>* pbnd = new(mess)bounds<3>{};
				Assert::IsTrue((*pbnd)[0] == 0 && (*pbnd)[1] == 0 && (*pbnd)[2] == 0);
			}

			// Check copy constructor
			{
				bounds<2> src{ 4, 1 };
				bounds<2> bnd{ src };
				Assert::IsTrue(bnd[0] == 4 && bnd[1] == 1);
			}

			// Check value_type constructor
			{
				bounds<1> bnd{ 99 };
				Assert::IsTrue(bnd[0] == 99);

				static_assert(std::is_convertible<ptrdiff_t, bounds<1>>::value, "ctor is not implicit!");
			}

			// Check initializer_list constructor
			{
				bounds<3> bnd{ 3, 14, 15 };
				Assert::IsTrue(bnd[0] == 3 && bnd[1] == 14 && bnd[2] == 15);

				const std::initializer_list<bounds<4>::value_type> il{ 1, 2, 3, 4 };
				bounds<4> bnd4(il);
				Assert::IsTrue(bnd4[0] == 1 && bnd4[1] == 2 && bnd4[2] == 3 && bnd4[3] == 4);
			}

			// Check assignment operator
			{
				const bounds<2> src{ 8, 7 };
				bounds<2> bnd;
				Assert::IsTrue(bnd[0] == 0 && bnd[1] == 0);
				bnd = src;
				Assert::IsTrue(bnd[0] == 8 && bnd[1] == 7);
			}

			// Check subscript operator
			{
				bounds<3> bnd{ 1, 2, 3 };
				bnd[0] = 11;
				Assert::IsTrue(bnd[0] == 11 && bnd[1] == 2 && bnd[2] == 3);

				const bounds<3> cbnd{ 4, 5, 6 };
				Assert::IsTrue(cbnd[0] == 4 && cbnd[1] == 5 && cbnd[2] == 6);
			}

			// Check equality and inequality operators
			{
				const bounds<2> bnd1{ 1, 2 };
				const bounds<2> bnd2{ 1, 2 };
				const bounds<2> bnd3{ 3, 2 };
				const bounds<2> bnd4{ 1, 4 };

				Assert::IsTrue(bnd1 == bnd2);
				Assert::IsTrue(!(bnd1 == bnd3) && !(bnd1 == bnd4));

				Assert::IsTrue(!(bnd1 != bnd2));
				Assert::IsTrue(bnd1 != bnd3 && bnd1 != bnd4);
			}

			// Check multiplication operators
			{
				const bounds<2> op{ 2, 3 };

				bounds<2> prod1 = op * 2;
				Assert::IsTrue(prod1[0] == 4 && prod1[1] == 6);

				bounds<2> prod2 = op * 1.5;
				Assert::IsTrue(prod2[0] == 2 && prod2[1] == 3);

				bounds<2> prod3 = 10 * op;
				Assert::IsTrue(prod3[0] == 20 && prod3[1] == 30);

				bounds<2> prod4 = 0.5 * op;
				Assert::IsTrue(prod4[0] == 0 && prod4[1] == 0);
			}

			// Check division operator
			{
				const bounds<2> op{ 10, 7 };

				bounds<2> quo1 = op / 2;
				Assert::IsTrue(quo1[0] == 5 && quo1[1] == 3);

				bounds<2> quo2 = op / 1.5;
				Assert::IsTrue(quo2[0] == 10 && quo2[1] == 7);
			}

			// Check multiply-equal operator
			{
				bounds<3> bnd1{ 2, 3, 4 };
				bounds<3>& bnd1_ref = (bnd1 *= 2);
				Assert::IsTrue(bnd1[0] == 4 && bnd1[1] == 6 && bnd1[2] == 8);
				Assert::IsTrue(&bnd1_ref == &bnd1);

				bounds<3> bnd2{ 2, 3, 4 };
				bounds<3>& bnd2_ref = (bnd2 *= 0.5);
				Assert::IsTrue(bnd2[0] == 0 && bnd2[1] == 0 && bnd2[2] == 0);
				Assert::IsTrue(&bnd2_ref == &bnd2);
			}

			// Check divide-equal operator
			{
				bounds<3> bnd1{ 10, 11, 12 };
				bounds<3>& bnd1_ref = (bnd1 /= 2);
				Assert::IsTrue(bnd1[0] == 5 && bnd1[1] == 5 && bnd1[2] == 6);
				Assert::IsTrue(&bnd1_ref == &bnd1);

				bounds<3> bnd2{ 10, 11, 12 };
				bounds<3>& bnd2_ref = (bnd2 /= 1.5);
				Assert::IsTrue(bnd2[0] == 10 && bnd2[1] == 11 && bnd2[2] == 12);
				Assert::IsTrue(&bnd2_ref == &bnd2);
			}

			// Check addition and subtraction operators
			{
				const bounds<2> bnd{ 12, 24 };
				const index<2> idx{ 10, -1 };

				bounds<2> sum1 = bnd + idx;
				Assert::IsTrue(sum1[0] == 22 && sum1[1] == 23);

				bounds<2> sum2 = idx + bnd;
				Assert::IsTrue(sum2[0] == 22 && sum2[1] == 23);

				bounds<2> dif = bnd - idx;
				Assert::IsTrue(dif[0] == 2 && dif[1] == 25);
			}

			// Check add-equal and subtract-equal operators
			{
				const index<2> op{ 4, 6 };

				bounds<2> bnd1{ 1, 2 };
				bounds<2>& sum_ref = (bnd1 += op);
				Assert::IsTrue(bnd1[0] == 5 && bnd1[1] == 8);
				Assert::IsTrue(&sum_ref == &bnd1);

				bounds<2> bnd2{ 10, 9 };
				bounds<2>& dif_ref = (bnd2 -= op);
				Assert::IsTrue(bnd2[0] == 6 && bnd2[1] == 3);
				Assert::IsTrue(&dif_ref == &bnd2);
			}

			// Check size function
			{
				const bounds<1> bnd1{ 2 };
				Assert::IsTrue(bnd1.size() == 2);

				const bounds<3> bnd2{ 3, 7, 2 };
				Assert::IsTrue(bnd2.size() == 42);
			}

			// Check contains function
			{
				const index<2> idx1{ 1, 2 };
				const index<2> idx2{ 1, 22 };
				const index<2> idx3{ 11, 2 };
				const index<2> idx4{ 11, 22 };
				const index<2> idx5{ -1, 2 };
				const index<2> idx6{ 2, 3 };
				const index<2> idx7{ 0, 0 };
				const bounds<2> bnd{ 2, 3 };
				Assert::IsTrue(bnd.contains(idx1) && !bnd.contains(idx2) && !bnd.contains(idx3));
				Assert::IsTrue(!bnd.contains(idx4) && !bnd.contains(idx5) && !bnd.contains(idx6));
				Assert::IsTrue(bnd.contains(idx7));
			}

			// Sanity check conveniance syntax in few common operations
			{
				bounds<2> bnd1 = { 1, 2 };
				bnd1 += {10, 100};
				bool res1 = (bnd1 == bounds<2>{11, 102});
				Assert::IsTrue(res1);

				bounds<3> bnd2 = { 6, 1, 8 };
				index<3> idx2{ 2, 0, -3 };
				bounds<3> res2 = bnd2 + 2 * idx2;
				Assert::IsTrue(res2[0] == 10 && res2[1] == 1 && res2[2] == 2);
			}
		}

		// Check subscript operator
		TEST_ERROR(std::declval<T>()[0] = 1, const index<1>);

		// Check multiplication operator
		TEST_ERROR(T{} *T{}, index<2>);

		// Check division operator
		TEST_ERROR(T{} / T{}, index<2>);
		TEST_ERROR(2 / T{}, index<2>);

		// Check multiply-equal operator
		TEST_ERROR(T{} *= T{}, index<2>);

		// Check divide-equal operator
		TEST_ERROR(T{} /= T{}, index<2>);

		TEST_METHOD(BoundsIterator)
		{
			// Check constructors and accessors
			{
				const bounds<3> bnd{ 1, 2, 3 };
				const index<3> idx{ 0, 1, 2 };

				const bounds_iterator<3> it1{ bnd };
				const bounds_iterator<3> it2{ bnd, idx };

				index<3> idx1 = *it1;
				Assert::IsTrue(idx1[0] == 0 && idx1[1] == 0 && idx1[2] == 0);

				index<3> idx2 = *(it2.operator->());
				Assert::IsTrue(idx2[0] == 0 && idx2[1] == 1 && idx2[2] == 2);
			}

			// Check increment operators
			{
				bounds_iterator<2> it{ { 2, 2 } };

				bounds_iterator<2>& it1_ref = ++it;
				Assert::IsTrue((*it == index<2>{0, 1}));

				Assert::IsTrue(&it1_ref == &it);

				bounds_iterator<2> it2 = it++;
				Assert::IsTrue((*it == index<2>{1, 0}));
				Assert::IsTrue((*it2 == index<2>{0, 1}));
			}

			// Check decrement operators
			{
				bounds_iterator<2> it{ { 2, 2 }, { 1, 0 } };

				bounds_iterator<2>& it1_ref = --it;
				Assert::IsTrue((*it == index<2>{0, 1}));
				Assert::IsTrue(&it1_ref == &it);

				bounds_iterator<2> it2 = it--;
				Assert::IsTrue((*it == index<2>{0, 0}));
				Assert::IsTrue((*it2 == index<2>{0, 1}));
			}

			// Check addition operators
			{
				const bounds_iterator<2> op{ { 2, 2 } };

				bounds_iterator<2> sum1 = op + 2;
				Assert::IsTrue((*sum1 == index<2>{1, 0}));

				bounds_iterator<2> sum2 = 2 + op;
				Assert::IsTrue((*sum2 == index<2>{1, 0}));
			}

			// Check add-equal operator
			{
				bounds_iterator<2> it{ { 4, 4 } };

				bounds_iterator<2>& sum_ref = (it += 2);
				Assert::IsTrue((*it == index<2>{0, 2}));
				Assert::IsTrue(&sum_ref == &it);

				it += 2;
				Assert::IsTrue((*it == index<2>{1, 0}));
			}

			// Check subtraction operator
			{
				const bounds_iterator<2> op{ { 2, 2 }, { 1, 0 } };

				bounds_iterator<2> dif = op - 2;
				Assert::IsTrue((*dif == index<2>{0, 0}));
			}

			// Check subtract-equal operator
			{
				bounds_iterator<2> it{ { 2, 2 }, { 1, 0 } };

				bounds_iterator<2>& dif_ref = (it -= 2);
				Assert::IsTrue((*it == index<2>{0, 0}));
				Assert::IsTrue(&dif_ref == &it);
			}

			// Check difference operator
			{
				const bounds_iterator<2> op1{ { 2, 2 }, { 1, 0 } };
				const bounds_iterator<2> op2{ { 2, 2 } };

				bounds_iterator<2>::difference_type dif = op1 - op2;
				Assert::IsTrue(dif == 2);
			}

			// Check subscript operator
			{
				const bounds_iterator<2> it{ { 2, 2 } };
				index<2> idx1 = it[0];
				index<2> idx2 = it[1];
				index<2> idx3 = it[2];
				Assert::IsTrue((idx1 == index<2>{0, 0} && idx2 == index<2>{0, 1} && idx3 == index<2>{1, 0}));
			}

			// Check relational operators
			{
				const bounds_iterator<2> it1{ { 2, 2 } };
				const bounds_iterator<2> it2{ it1 };
				const bounds_iterator<2> it3{ it1 + 2 };

				Assert::IsTrue(it1 == it1 && it1 == it2 && !(it1 == it3));
				Assert::IsTrue(!(it1 != it1) && !(it1 != it2) && it1 != it3);
				Assert::IsTrue(!(it1 < it1) && !(it1 < it2) && it1 < it3 && !(it3 < it1));
				Assert::IsTrue(it1 <= it1 && it1 <= it2 && it1 <= it3 && !(it3 <= it1));
				Assert::IsTrue(!(it1 > it1) && !(it1 > it2) && !(it1 > it3) && it3 > it1);
				Assert::IsTrue(it1 >= it1 && it1 >= it2 && it3 >= it1 && !(it3 <= it1));
			}

			// Check swap function
			{
				bounds_iterator<2> it1{ { 2, 2 } };
				bounds_iterator<2> it2{ { 3, 1 }, { 1, 0 } };

				it1.swap(it2);
				Assert::IsTrue((*it1 == index<2>{1, 0}));
				Assert::IsTrue((*it2 == index<2>{0, 0}));
				Assert::IsTrue((*(++it1) == index<2>{2, 0}));
				Assert::IsTrue((*(++it2) == index<2>{0, 1}));

				it1.swap(it1);
				Assert::IsTrue((*it1 == index<2>{2, 0}));
			}

			// Check bounds iterator usage (rank = 1)
			{
				bounds<1> bnd{ 10 };

				bounds_iterator<1> it_beg = begin(bnd);
				Assert::IsTrue(*it_beg == index<1>{0});

				bounds_iterator<1> it_end = end(bnd);
				Assert::IsTrue(*it_end == index<1>{10});
			}

			// Check bounds iterator usage (rank = 2)
			{
				bounds<2> bnd{ 3, 5 };

				bounds_iterator<2> it_beg = bnd.begin();
				Assert::IsTrue((*it_beg == index<2>{0, 0}));

				bounds_iterator<2> it_end = bnd.end();
				Assert::IsTrue((*it_end == index<2>{3, 5}));
			}
		}

		// Check subscript operator
		TEST_ERROR(std::declval<T>()[0] = 1, const bounds<1>);

		// Check unary plus and minus operators
		TEST_ERROR(+T{}, bounds<2>);
		TEST_ERROR(-T{}, bounds<2>);

		// Check multiplication operator
		TEST_ERROR(T{} *T{}, bounds<2>);

		// Check division operator
		TEST_ERROR(T{} / T{}, bounds<2>);
		TEST_ERROR(2 / T{}, bounds<2>);

		// Check multiply-equal operator
		TEST_ERROR(T{} *= T{}, bounds<2>);

		// Check divide-equal operator
		TEST_ERROR(T{} /= T{}, bounds<2>);

		// Check addition and subtraction operators
		TEST_ERROR(T{} +T{}, bounds<2>);
		TEST_ERROR(T{} -T{}, bounds<2>);

		// Check add-equal and subtract-equal operators
		TEST_ERROR(T{} += T{}, bounds<2>);
		TEST_ERROR(T{} -= T{}, bounds<2>);

		TEST_METHOD(RangeForIndexes)
		{
			{
				int iters = 0;
				int check[10] = {};

				bounds<1> bnd1{ 10 };
				for (auto idx : bnd1)
				{
					Assert::IsTrue(idx[0] >= 0 && idx[0] < 10);

					iters++;
					check[idx[0]]++;
				}

				Assert::IsTrue(iters == 10);
				for (auto v : check)
				{
					Assert::IsTrue(v == 1);
				}
			}

			{
				int iters = 0;
				int check[3][5] = {};

				bounds<2> bnd2{ 3, 5 };
				for (auto idx : bnd2)
				{
					Assert::IsTrue(idx[0] >= 0 && idx[0] < 3);
					Assert::IsTrue(idx[1] >= 0 && idx[1] < 5);

					iters++;
					check[idx[0]][idx[1]]++;
				}

				Assert::IsTrue(iters == 15);
				for (int i = 0; i < 3; i++)
				{
					for (int j = 0; j < 5; j++)
					{
						Assert::IsTrue(check[i][j] == 1);
					}
				}
			}
		}

		TEST_METHOD(ForEachIndexes)
		{
			int iters = 0;
			int check[3][5][2] = {};

			bounds<3> bnd2{ 3, 5, 2 };
			std::for_each(begin(bnd2), end(bnd2), [&](index<3> idx)
			{
				Assert::IsTrue(idx[0] >= 0 && idx[0] < 3);
				Assert::IsTrue(idx[1] >= 0 && idx[1] < 5);
				Assert::IsTrue(idx[2] >= 0 && idx[2] < 2);

				iters++;
				check[idx[0]][idx[1]][idx[2]]++;
			});

			Assert::AreEqual(30, iters);
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 5; j++)
				{
					for (int k = 0; k < 2; k++)
					{
						Assert::AreEqual(1, check[i][j][k]);
					}
				}
			}
		}
	};
} // ParallelSTL_Tests
