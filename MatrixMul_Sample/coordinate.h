//
//Implementation of index, bounds and bounds_iterator classes per N3851. Be aware that further changes in the interfaces are planned and expected.
//
#pragma once
#include <algorithm>
#include <array>
#include <assert.h>
#include <numeric>
#include <type_traits>

	namespace details
	{
		template <typename ConcreteType, typename ValueType, int Rank>
		class coordinate_facade
		{
			static_assert(std::is_fundamental<ValueType>::value, "ValueType must be fundamental!");
			static_assert(Rank > 0, "Rank must be greater than 0!");

		public:
			using reference = ValueType&;
			using const_reference = const ValueType&;
			using size_type = size_t;
			using value_type = ValueType;
			static const int rank = Rank;

			coordinate_facade()
			{
				static_assert(std::is_base_of<coordinate_facade, ConcreteType>::value, "ConcreteType must be derived from coordinate_facade.");
				for (int i = 0; i < rank; ++i)
					elems[i] = value_type{};
			}

			coordinate_facade(const coordinate_facade& rhs)
			{
				for (int i = 0; i < rank; ++i)
				elems[i] = rhs.elems[i];
			}

			coordinate_facade(value_type e0)
			{
				static_assert(std::is_base_of<coordinate_facade, ConcreteType>::value, "ConcreteType must be derived from coordinate_facade.");
				static_assert(rank == 1, "This constructor can only be used with rank == 1.");
				elems[0] = e0;
			}

			// Preconditions: il.size() == rank
			coordinate_facade(const std::initializer_list<value_type>& il)
			{
				static_assert(std::is_base_of<coordinate_facade, ConcreteType>::value, "ConcreteType must be derived from coordinate_facade.");
				assert(il.size() == rank);
				for (int i = 0; i < rank; ++i)
				{
					elems[i] = begin(il)[i];
				}
			}

		protected:
			coordinate_facade& operator=(const coordinate_facade& rhs)
			{
				for (int i = 0; i < rank; ++i)
				elems[i] = rhs.elems[i];
				return *this;
			}

			// Preconditions: component_idx < rank
			reference operator[](size_type component_idx)
			{
				return elems[component_idx];
			}

			// Preconditions: component_idx < rank
			const_reference operator[](size_type component_idx) const
			{
				return elems[component_idx];
			}

			bool operator==(const ConcreteType& rhs) const
			{
				return std::equal(std::begin(elems), std::end(elems), std::begin(rhs.elems)); // Note: assuming equal cannot throw.
			}

			bool operator!=(const ConcreteType& rhs) const
			{
				return !(to_concrete() == rhs);
			}

			ConcreteType operator+() const
			{
				return to_concrete();
			}

			ConcreteType operator-() const
			{
				ConcreteType ret = to_concrete();
				for (int i = 0; i < rank; ++i)
					ret.elems[i] = -ret.elems[i];
				return ret;
			}

			ConcreteType operator+(const ConcreteType& rhs) const
			{
				ConcreteType ret = to_concrete();
				ret += rhs;
				return ret;
			}

			ConcreteType operator-(const ConcreteType& rhs) const
			{
				ConcreteType ret = to_concrete();
				ret -= rhs;
				return ret;
			}

			ConcreteType& operator+=(const ConcreteType& rhs)
			{
				for (int i = 0; i < rank; ++i)
				elems[i] += rhs.elems[i];
				return to_concrete();
			}

			ConcreteType& operator-=(const ConcreteType& rhs)
			{
				for (int i = 0; i < rank; ++i)
				elems[i] -= rhs.elems[i];
				return to_concrete();
			}

			ConcreteType& operator++()
			{
				static_assert(rank == 1, "This operator can only be used with rank == 1.");
				++elems[0];
				return to_concrete();
			}

			ConcreteType operator++(int)
			{
				static_assert(rank == 1, "This operator can only be used with rank == 1.");
				ConcreteType ret = to_concrete();
				++(*this);
				return ret;
			}

			ConcreteType& operator--()
			{
				static_assert(rank == 1, "This operator can only be used with rank == 1.");
				--elems[0];
				return to_concrete();
			}

			ConcreteType operator--(int)
			{
				static_assert(rank == 1, "This operator can only be used with rank == 1.");
				ConcreteType ret = to_concrete();
				--(*this);
				return ret;
			}

			template <typename ArithmeticType>
			std::enable_if_t<std::is_arithmetic<ArithmeticType>::value, ConcreteType> mul(ArithmeticType v) const
			{
				ConcreteType ret = to_concrete();
				ret *= v;
				return ret;
			}

			template <typename ArithmeticType>
			std::enable_if_t<std::is_arithmetic<ArithmeticType>::value, ConcreteType> div(ArithmeticType v) const
			{
				ConcreteType ret = to_concrete();
				ret /= v;
				return ret;
			}

			template <typename ArithmeticType>
			friend std::enable_if_t<std::is_arithmetic<ArithmeticType>::value, ConcreteType> operator*(ArithmeticType v, const ConcreteType& rhs)
			{
				return rhs * v;
			}

			template <typename ArithmeticType>
			std::enable_if_t<std::is_arithmetic<ArithmeticType>::value, ConcreteType&> mul_eq(ArithmeticType v)
			{
				for (int i = 0; i < rank; ++i)
				elems[i] *= v;
				return to_concrete();
			}

			template <typename ArithmeticType>
			std::enable_if_t<std::is_arithmetic<ArithmeticType>::value, ConcreteType&> div_eq(ArithmeticType v)
			{
				for (int i = 0; i < rank; ++i)
				elems[i] /= v;
				return to_concrete();
			}

			value_type elems[rank];

		private:
			const ConcreteType& to_concrete() const
			{
				   return static_cast<const ConcreteType&>(*this);
		}

			ConcreteType& to_concrete()
			{
				return static_cast<ConcreteType&>(*this);
			}
		};
	}

#define USING_COORDINATE_FACADE_OPERATOR_MUL(BaseT, ThisT) \
	template <typename ArithmeticType> std::enable_if_t<std::is_arithmetic<ArithmeticType>::value, ThisT> operator*(ArithmeticType v) const { return BaseT::mul(v); }
#define USING_COORDINATE_FACADE_OPERATOR_DIV(BaseT, ThisT) \
	template <typename ArithmeticType> std::enable_if_t<std::is_arithmetic<ArithmeticType>::value, ThisT> operator/(ArithmeticType v) const { return BaseT::div(v); }
#define USING_COORDINATE_FACADE_OPERATOR_MUL_EQ(BaseT, ThisT) \
	template <typename ArithmeticType> std::enable_if_t<std::is_arithmetic<ArithmeticType>::value, ThisT&> operator*=(ArithmeticType v) { return BaseT::mul_eq(v); }
#define USING_COORDINATE_FACADE_OPERATOR_DIV_EQ(BaseT, ThisT) \
	template <typename ArithmeticType> std::enable_if_t<std::is_arithmetic<ArithmeticType>::value, ThisT&> operator/=(ArithmeticType v) { return BaseT::div_eq(v); }

	template <int Rank>
	class index : private details::coordinate_facade<index<Rank>, ptrdiff_t, Rank>
	{
		using Base = details::coordinate_facade<index<Rank>, ptrdiff_t, Rank>;
		friend Base;

	public:
		using reference = typename Base::reference;
		using const_reference = typename Base::const_reference;
		using size_type = typename Base::size_type;
		using value_type = typename Base::value_type;
		using Base::rank;

		index() : Base(){}
		index(const coordinate_facade& rhs) : Base(rhs){}
		index(value_type e0) : Base(e0){}
		index(const std::initializer_list<value_type>& il) : Base(il){}
		
		using Base::operator[];
		using Base::operator==;
		using Base::operator!=;
		using Base::operator+;
		using Base::operator-;
		using Base::operator+=;
		using Base::operator-=;
		using Base::operator++;
		using Base::operator--;
		USING_COORDINATE_FACADE_OPERATOR_MUL(Base, index);
		USING_COORDINATE_FACADE_OPERATOR_DIV(Base, index);
		USING_COORDINATE_FACADE_OPERATOR_MUL_EQ(Base, index);
		USING_COORDINATE_FACADE_OPERATOR_DIV_EQ(Base, index);

		index& operator=(const index& rhs)
		{
			Base::operator=(rhs);
			return *this;
		}
	};

	template <int Rank> struct bounds_iterator;

	// Preconditions (for all functions): after operation:
	//  - every component must be greater than or equal to zero
	//  - product of all components must not overflow ptrdiff_t
	template <int Rank>
	class bounds : private details::coordinate_facade<bounds<Rank>, ptrdiff_t, Rank>
	{
		using Base = details::coordinate_facade<bounds<Rank>, ptrdiff_t, Rank>;
		friend Base;

	public:
		using reference = typename Base::reference;
		using const_reference = typename Base::const_reference;
		using size_type = typename Base::size_type;
		using value_type = typename Base::value_type;
		using Base::rank;

		bounds() :Base(){}
		bounds(const coordinate_facade& rhs) : Base(rhs){}
		bounds(value_type e0) : Base(e0){}
		bounds(const std::initializer_list<value_type>& il) : Base(il){}


		using Base::operator[];
		using Base::operator==;
		using Base::operator!=;
		USING_COORDINATE_FACADE_OPERATOR_MUL(Base, bounds);
		USING_COORDINATE_FACADE_OPERATOR_DIV(Base, bounds);
		USING_COORDINATE_FACADE_OPERATOR_MUL_EQ(Base, bounds);
		USING_COORDINATE_FACADE_OPERATOR_DIV_EQ(Base, bounds);

		bounds& operator=(const bounds& rhs)
		{
			Base::operator=(rhs);
			return *this;
		}

		bounds operator+(const index<rank>& rhs) const
		{
			auto ret = *this;
			ret += rhs;
			return ret;
		}

		bounds operator-(const index<rank>& rhs) const
		{
			auto ret = *this;
			ret -= rhs;
			return ret;
		}

		friend bounds operator+(const index<rank>& lhs, const bounds& rhs)
		{
			return rhs + lhs;
		}

		bounds& operator+=(const index<rank>& rhs)
		{
			for (int i = 0; i < rank; ++i)
			Base::elems[i] += rhs[i];
			return *this;
		}

		bounds& operator-=(const index<rank>& rhs)
		{
			for (int i = 0; i < rank; ++i)
			Base::elems[i] -= rhs[i];
			return *this;
		}

		size_type size() const
		{
			size_type ret = Base::elems[0];
			for (int i = 1; i < rank; ++i)
				ret *= Base::elems[i];
			return ret;
		}

		bool contains(const index<rank>& idx) const
		{
			for (int i = 0; i < rank; ++i)
			{
				if (idx[i] < 0 || idx[i] >= Base::elems[i])
					return false;
			}
			return true;
		}

		bounds_iterator<rank> begin() const
		{
			return bounds_iterator<Rank>{ *this };
		}

		bounds_iterator<rank> end() const
		{
			index<rank> idx_end;
			for (int i = 0; i < rank; ++i)
				idx_end[i] = (*this)[i];
			return bounds_iterator<Rank>{ *this, idx_end };
		}
	};

	template <int Rank>
	struct bounds_iterator
		: public std::iterator<std::random_access_iterator_tag, const index<Rank>, ptrdiff_t, const index<Rank>*, const index<Rank>>
	{
		// Note: This is a "stashing" iterator which is not fully conformant with the requirements of the random access iterator. Use with caution.

		// Preconditions: bnd.contains(curr) unless bnd.size() == 0
		explicit bounds_iterator(bounds<Rank> bnd, index<Rank> curr = index<Rank>{})
		: bnd{ std::move(bnd) }
		, curr{ std::move(curr) }
		{}

		bounds_iterator(const bounds_iterator& rhs) = default;
		bounds_iterator& operator=(const bounds_iterator& rhs) = default;

		reference operator*() const
		{
			return curr;
		}

		pointer operator->() const
		{
			return &curr;
		}

		bounds_iterator& operator++()
		{
			for (int i = Rank; i-- > 0;)
			{
				if (++curr[i] < bnd[i])
				{
					return *this;
				}
				else
				{
					curr[i] = 0;
				}
			}
			// If we're here we've wrapped over - set to past-the-end.
			for (int i = 0; i < Rank; ++i)
			{
				curr[i] = bnd[i];
			}
			return *this;
		}

		bounds_iterator operator++(int)
		{
			auto ret = *this;
			++(*this);
			return ret;
		}

		bounds_iterator& operator--()
		{
			for (int i = Rank; i-- > 0;)
			{
				if (curr[i]-- > 0)
				{
					return *this;
				}
				else
				{
					curr[i] = bnd[i] - 1;
				}
			}
			// If we're here the preconditions were violated
			// "pre: there exists s such that r == ++s"
			assert(false);
			return *this;
		}

		bounds_iterator operator--(int)
		{
			auto ret = *this;
			--(*this);
			return ret;
		}

		bounds_iterator operator+(difference_type n) const
		{
			bounds_iterator ret{ *this };
			return ret += n;
		}

		bounds_iterator& operator+=(difference_type n)
		{
			auto linear_idx = linearize(curr) + n;

			bounds<Rank> stride;
			stride[Rank - 1] = 1;
			for (int i = Rank - 1; i-- > 0;)
			{
				stride[i] = stride[i + 1] * bnd[i + 1];
			}

			for (int i = 0; i < Rank; ++i)
			{
				curr[i] = linear_idx / stride[i];
				linear_idx = linear_idx % stride[i];
			}

			return *this;
		}

		bounds_iterator operator-(difference_type n) const
		{
			bounds_iterator ret{ *this };
			return ret -= n;
		}

		bounds_iterator& operator-=(difference_type n)
		{
			return *this += -n;
		}

		difference_type operator-(const bounds_iterator& rhs) const
		{
			return linearize(curr) - linearize(rhs.curr);
		}

		reference operator[](difference_type n) const
		{
			return *(*this + n);
		}

		bool operator==(const bounds_iterator& rhs) const
		{
			return curr == rhs.curr;
		}

		bool operator!=(const bounds_iterator& rhs) const
		{
			return !(*this == rhs);
		}

		bool operator<(const bounds_iterator& rhs) const
		{
			for (int i = 0; i < Rank; ++i)
			{
				if (curr[i] < rhs.curr[i])
					return true;
			}
			return false;
		}

		bool operator<=(const bounds_iterator& rhs) const
		{
			return !(rhs < *this);
		}

		bool operator>(const bounds_iterator& rhs) const
		{
			return rhs < *this;
		}

		bool operator>=(const bounds_iterator& rhs) const
		{
			return !(rhs > *this);
		}

		void swap(bounds_iterator& rhs)
		{
			std::swap(bnd, rhs.bnd);
			std::swap(curr, rhs.curr);
		}

	private:
		ptrdiff_t linearize(const index<Rank>& idx) const
		{
			   // Check if past-the-end
			   bool pte = true;
			   for (int i = 0; i < Rank; ++i)
			   {
				   if (idx[i] != bnd[i])
				   {
					   pte = false;
					   break;
				   }
			   }

			   ptrdiff_t multiplier = 1;
			   ptrdiff_t res = 0;

			   if (pte)
			   {
				   res = 1;
				   for (int i = Rank; i-- > 0;)
				   {
					   res += (idx[i] - 1) * multiplier;
					   multiplier *= bnd[i];
				   }
			   }
			   else
			   {
				   for (int i = Rank; i-- > 0;)
				   {
					   res += idx[i] * multiplier;
					   multiplier *= bnd[i];
				   }
			   }

			   return res;
	}

		bounds<Rank> bnd;
		index<Rank> curr;
	};

	template <int Rank>
	bounds_iterator<Rank> operator+(typename bounds_iterator<Rank>::difference_type n, const bounds_iterator<Rank>& rhs)
	{
		return rhs + n;
	}
