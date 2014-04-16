#pragma once

#ifndef _UTILS_H_
#define _UTILS_H_ 1

#include <vector>
#include <atomic>

namespace utils
{

// test iterator that mimicks all kind of iterators depending on _IterCat
template<typename _IterCat, typename _It>
class test_iterator :
	public std::iterator<
	_IterCat,
	typename _It::value_type,
	typename _It::difference_type,
	typename _It::pointer,
	typename _It::reference
	>
{
public:
	typedef test_iterator<_IterCat, _It> _Myiter;

	test_iterator(const _It& _Arg) : _Myval(_Arg)
	{
	}

	test_iterator()
	{
	}

	reference operator*() const
	{	// return designated object
		return *_Myval;
	}

	bool operator==(const _Myiter& _Right) const
	{	// test for iterator equality
		return (_Myval == _Right._Myval);
	}

	bool operator!=(const _Myiter& _Right) const
	{	// test for iterator inequality
		return (!(*this == _Right));
	}

	// Input iterator
	_Myiter& operator++()
	{
		++_Myval;
		return (*this);
	}

	_Myiter operator++(int)
	{	// postincrement
		_Myiter _Tmp = *this;
		++*this;
		return (_Tmp);
	}

	pointer operator->() const
	{	// return pointer to class object
		static_assert(!std::is_same<_IterCat, std::output_iterator_tag>::value, "Not supported for output iterator.");

		return (_STD pointer_traits<pointer>::pointer_to(**this));
	}

	// Bidirectional and reandom access only
	_Myiter& operator--()
	{	// predecrement
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value || 
			std::is_same<_IterCat, std::bidirectional_iterator_tag>::value, "Supported for random access or bidirectional iterator only.");

		--_Myval;
		return (*this);
	}

	_Myiter operator--(int)
	{	// postdecrement
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value ||
			std::is_same<_IterCat, std::bidirectional_iterator_tag>::value, "Supported for random access or bidirectional iterator only.");

		_Myiter _Tmp = *this;
		--*this;
		return (_Tmp);
	}

	// Random access iterator
	_Myiter& operator+=(difference_type _Off)
	{	// increment by integer
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value, "Supported for random access iterator only.");

		_Myval += _Off;
		return (*this);
	}

	_Myiter operator+(difference_type _Off) const
	{	// return this + integer
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value, "Supported for random access iterator only.");

		_Myiter _Tmp = *this;
		return (_Tmp += _Off);
	}

	_Myiter& operator-=(difference_type _Off)
	{	// decrement by integer
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value, "Supported for random access iterator only.");

		return (*this += -_Off);
	}

	_Myiter operator-(difference_type _Off) const
	{	// return this - integer
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value, "Supported for random access iterator only.");

		_Myiter _Tmp = *this;
		return (_Tmp -= _Off);
	}

	difference_type operator-(const _Myiter& _Right) const
	{	// return difference of iterators
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value, "Supported for random access iterator only.");

		return (_Myval - _Right._Myval);
	}

	value_type& operator[](difference_type _Off) const
	{
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value, "Supported for random access iterator only.");

		return _Myval[_Off];
	}

	bool operator<(const _Myiter& _Right) const
	{	// test if this < _Right
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value, "Supported for random access iterator only.");

		return (_Myval < _Right._Myval);
	}

	bool operator>(const _Myiter& _Right) const
	{	// test if this > _Right
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value, "Supported for random access iterator only.");

		return (_Right < *this);
	}

	bool operator<=(const _Myiter& _Right) const
	{	// test if this <= _Right
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value, "Supported for random access iterator only.");

		return (!(_Right < *this));
	}

	bool operator>=(const _Myiter& _Right) const
	{	// test if this >= _Right
		static_assert(std::is_same<_IterCat, std::random_access_iterator_tag>::value, "Supported for random access iterator only.");

		return (!(*this < _Right));
	}

	_It get_inner() const
	{
		return _Myval;
	}
private:
	_It _Myval;
};

template<typename _IterCat, typename _It>
test_iterator<_IterCat, _It> make_test_iterator(const _It& _Iter)
{
	return test_iterator<_IterCat, _It>(_Iter);
}

struct Algo
{
    static const size_t MAX_ITERATIONS = 1000;
    static const size_t RAND_ITERATIONS = 1000;

    static void Settings(size_t _MIter, size_t _RIter = RAND_ITERATIONS)
    {
        _Max_iterations = _MIter;
        _Rand_iterations = _RIter;
    }
    
protected:
    static size_t _Max_iterations;
    static size_t _Rand_iterations;
};

__declspec(selectany) size_t Algo::_Max_iterations = Algo::MAX_ITERATIONS;
__declspec(selectany) size_t Algo::_Rand_iterations = Algo::RAND_ITERATIONS;

template<typename _AlgoObj, typename _InIterCat, typename _Type, typename _RetResult = void, typename _OutIterCat = _InIterCat>
struct AlgoTest : public Algo
{
public:
	template<typename _ExpectedResultType, typename _DefaultResultType>
	struct _ResultType {
		typedef _ExpectedResultType type;
	};

	template<typename _DefaultResultType>
	struct _ResultType<void, _DefaultResultType> {
		typedef _DefaultResultType type;
	};

	typedef typename std::vector<_Type> container;
	typedef typename test_iterator<_InIterCat, typename container::iterator> in_iterator;
	typedef typename test_iterator<_OutIterCat, typename container::iterator> out_iterator;

	AlgoTest() 
	{
		size_t _Count = _Max_iterations;
		if (_Rand_iterations != 0)
			_Count += rand() % _Rand_iterations;

		_Ct.reserve(_Count);
		_Ct.resize(_Count);
	}

	template<typename _Func>
	void Catch(const _Func& _F)
	{
		try {
			_F();
		}
		catch (...) {
			_Exception = std::current_exception();
		}
	}

	void set_result(const typename _ResultType<_RetResult, out_iterator>::type& _R)
	{
		_Result = _R;
	}

	typename in_iterator begin_in()
	{
		return make_test_iterator<in_iterator::iterator_category>(std::begin(_Ct));
	}

	typename in_iterator end_in()
	{
		return make_test_iterator<in_iterator::iterator_category>(std::end(_Ct));
	}

	typename out_iterator begin_dest()
	{
		return make_test_iterator<out_iterator::iterator_category>(std::begin(_Ct_copy));
	}

	typename out_iterator end_dest()
	{
		return make_test_iterator<out_iterator::iterator_category>(std::end(_Ct_copy));
	}

    size_t size_in()
    {
		return _Ct.size();
    }

	container _Ct;
	container _Ct_copy;
	typename _ResultType<_RetResult, out_iterator>::type _Result;
    std::exception_ptr _Exception;
};

	struct MovableOnly
	{
	private:
		MovableOnly(MovableOnly&);
		MovableOnly &operator=(MovableOnly&);
	public:
		MovableOnly() {}
		MovableOnly(MovableOnly&&) {}
		MovableOnly &operator=(MovableOnly&&) { return *this; }
	};

	struct CopyableOnly
	{
	private:
		CopyableOnly(CopyableOnly&&) {}
		CopyableOnly &operator=(CopyableOnly&&) { return *this; }
	public:
		CopyableOnly() :_Copy_count(0) {}
		CopyableOnly(const CopyableOnly& _Other)
		{
			_Copy_count = _Other._Copy_count;
			_Copy_count++;
		};
		CopyableOnly &operator=(const CopyableOnly& _Other)
		{
			_Copy_count = _Other._Copy_count;
			_Copy_count++;
			return *this;
		}

		int _Copy_count;
	};

	struct CustomException {};

	// Helper counts destructions and constructions of the object
	// The helper randomly throws an exception in copy constructor 
	struct UnintializeHelper {
		private:
			static std::atomic<int> _Count;
			static std::atomic<bool> _Throw;
		public:
			static void set_no_exception()
			{
				_Count = INT_MAX;
			}

			static void reset()
			{
				_Count = 0;
				_Throw = true;
			}

			static int get_count()
			{
				return _Count;
			}

			UnintializeHelper()
			{
				if (INT_MAX != _Count)
					++_Count;
			}

			UnintializeHelper(const UnintializeHelper&)
			{
				if (INT_MAX != _Count) {
					// We need at least one exception being thrown
					if (_Throw == true || rand() % 4 == 0) {
						_Throw = false;
						throw CustomException();
					}

					++_Count;
				}
			}

			~UnintializeHelper()
			{
				if (INT_MAX != _Count)
					--_Count;
			}
	};

	__declspec(selectany) std::atomic<int> UnintializeHelper::_Count(0);
	__declspec(selectany) std::atomic<bool> UnintializeHelper::_Throw(true);
} //namespace utils

namespace std {
	// The test iterator must be marked as checked iterator
	template<typename _IterCat, typename _It>
	struct _Is_checked_helper<utils::test_iterator<_IterCat, _It> >
		: public true_type
	{
	};

	// UnintializeHelper must be marked as checked iterator to avoid warning
	template<>
	struct _Is_checked_helper<utils::UnintializeHelper*>
		: public true_type
	{
	};

	// int* is used commonly for testing thus avoid warning
	template<>
	struct _Is_checked_helper<int*>
		: public true_type
	{
	};
}

#define RunBatch(_Func) \
    _Func(seq); \
    _Func(par); \
    _Func(vec)

#endif // _UTILS_H_
