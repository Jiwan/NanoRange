// Range v3 library
//
//  Copyright Eric Niebler 2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3

//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <nanorange/algorithm/equal.hpp>
#include "../catch.hpp"
#include "../test_iterators.hpp"
#include "../test_utils.hpp"

namespace ranges = nano;

namespace {

void test()
{
	using namespace ranges;
	int ia[] = {0, 1, 2, 3, 4, 5};
	constexpr unsigned s = sizeof(ia)/sizeof(int);
	//int ib[s] = {0, 1, 2, 5, 4, 5};
	int* ib = new int[s]{0, 1, 2, 5, 4, 5};
	CHECK(equal(input_iterator<const int*>(ia),
				sentinel<const int*>(ia + s),
				input_iterator<const int*>(ia)));
	CHECK(equal(input_iterator<const int*>(ia),
				sentinel<const int*>(ia + s),
				input_iterator<const int*>(ia),
				sentinel<const int*>(ia + s)));
	using ra_t = random_access_iterator<const int*>;
	static_assert(InputIterator<ra_t>, "");
	static_assert(IndirectlyComparable<ra_t, ra_t, equal_to>, "");
	CHECK(equal(random_access_iterator<const int*>(ia),
				random_access_iterator<const int*>(ia + s),
				random_access_iterator<const int*>(ia),
				random_access_iterator<const int*>(ia + s)));
	CHECK(equal(random_access_iterator<const int*>(ia),
				sentinel<const int*>(ia + s),
				random_access_iterator<const int*>(ia),
				sentinel<const int*>(ia + s)));
	CHECK(!equal(input_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s),
				 input_iterator<const int*>(ib)));
	CHECK(!equal(input_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s),
				 input_iterator<const int*>(ib),
				 sentinel<const int*>(ib + s)));
	CHECK(!equal(random_access_iterator<const int*>(ia),
				 random_access_iterator<const int*>(ia + s),
				 random_access_iterator<const int*>(ib),
				 random_access_iterator<const int*>(ib + s)));
	CHECK(!equal(random_access_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s),
				 random_access_iterator<const int*>(ib),
				 sentinel<const int*>(ib + s)));
	CHECK(!equal(input_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s),
				 input_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s - 1)));
	CHECK(!equal(random_access_iterator<const int*>(ia),
				 random_access_iterator<const int*>(ia + s),
				 random_access_iterator<const int*>(ia),
				 random_access_iterator<const int*>(ia + s - 1)));
	CHECK(!equal(random_access_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s),
				 random_access_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s - 1)));
	delete[] ib;
}

void test_rng()
{
	using namespace ranges;

	int ia[] = {0, 1, 2, 3, 4, 5};
	constexpr unsigned s = size(ia);
	int ib[s] = {0, 1, 2, 5, 4, 5};
	/*CHECK(equal(make_subrange(input_iterator<const int*>(ia),
												 sentinel<const int*>(ia+s)),
							input_iterator<const int*>(ia)));
	CHECK(equal(make_subrange(input_iterator<const int*>(ia),
												 sentinel<const int*>(ia+s)),
							make_subrange(input_iterator<const int*>(ia),
												 sentinel<const int*>(ia+s))));*/
	CHECK(equal(subrange(random_access_iterator<const int*>(ia),
							  random_access_iterator<const int*>(ia + s)),
				subrange(random_access_iterator<const int*>(ia),
							  random_access_iterator<const int*>(ia + s))));
	/*CHECK(equal(make_subrange(random_access_iterator<const int*>(ia),
												 sentinel<const int*>(ia+s)),
							make_subrange(random_access_iterator<const int*>(ia),
												 sentinel<const int*>(ia + s))));*/
	CHECK(!equal(subrange(input_iterator<const int*>(ia),
							   input_iterator<const int*>(ia + s)),
				 input_iterator<const int*>(ib)));
	CHECK(!equal(subrange(input_iterator<const int*>(ia),
							   input_iterator<const int*>(ia + s)),
				 subrange(input_iterator<const int*>(ib),
							   input_iterator<const int*>(ib + s))));
	CHECK(!equal(subrange(random_access_iterator<const int*>(ia),
							   random_access_iterator<const int*>(ia + s)),
				 subrange(random_access_iterator<const int*>(ib),
							   random_access_iterator<const int*>(ib + s))));
	CHECK(!equal(subrange(random_access_iterator<const int*>(ia),
							   sentinel<const int*>(ia + s)),
				 subrange(random_access_iterator<const int*>(ib),
							   sentinel<const int*>(ib + s))));
	CHECK(!equal(subrange(input_iterator<const int*>(ia),
							   sentinel<const int*>(ia + s)),
				 subrange(input_iterator<const int*>(ia),
							   sentinel<const int*>(ia + s - 1))));
	CHECK(!equal(subrange(random_access_iterator<const int*>(ia),
							   random_access_iterator<const int*>(ia + s)),
				 subrange(random_access_iterator<const int*>(ia),
							   random_access_iterator<const int*>(
									   ia + s - 1))));
	CHECK(!equal(subrange(random_access_iterator<const int*>(ia),
							   sentinel<const int*>(ia + s)),
				 subrange(random_access_iterator<const int*>(ia),
							   sentinel<const int*>(ia + s - 1))));
}

int comparison_count = 0;

template <typename T>
bool counting_equals(const T& a, const T& b)
{
	++comparison_count;
	return a == b;
}

void test_pred()
{
	using namespace ranges;

	int ia[] = {0, 1, 2, 3, 4, 5};
	constexpr unsigned s = size(ia);
	int ib[s] = {0, 1, 2, 5, 4, 5};
	CHECK(equal(input_iterator<const int*>(ia),
				sentinel<const int*>(ia + s),
				input_iterator<const int*>(ia),
				std::equal_to<int>()));
	CHECK(equal(input_iterator<const int*>(ia),
				sentinel<const int*>(ia + s),
				input_iterator<const int*>(ia),
				sentinel<const int*>(ia + s),
				std::equal_to<int>()));
	CHECK(equal(random_access_iterator<const int*>(ia),
				random_access_iterator<const int*>(ia + s),
				random_access_iterator<const int*>(ia),
				random_access_iterator<const int*>(ia + s),
				std::equal_to<int>()));
	CHECK(equal(random_access_iterator<const int*>(ia),
				sentinel<const int*>(ia + s),
				random_access_iterator<const int*>(ia),
				sentinel<const int*>(ia + s),
				std::equal_to<int>()));

	comparison_count = 0;
	CHECK(!equal(input_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s),
				 input_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s - 1),
				 counting_equals<int>));
	CHECK(comparison_count > 0);
	comparison_count = 0;
	CHECK(!equal(random_access_iterator<const int*>(ia),
				 random_access_iterator<const int*>(ia + s),
				 random_access_iterator<const int*>(ia),
				 random_access_iterator<const int*>(ia + s - 1),
				 counting_equals<int>));
	CHECK(comparison_count == 0);
	comparison_count = 0;
	CHECK(!equal(random_access_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s),
				 random_access_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s - 1),
				 counting_equals<int>));
	CHECK(comparison_count > 0);
	CHECK(!equal(input_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s),
				 input_iterator<const int*>(ib),
				 std::equal_to<int>()));
	CHECK(!equal(input_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s),
				 input_iterator<const int*>(ib),
				 sentinel<const int*>(ib + s),
				 std::equal_to<int>()));
	CHECK(!equal(random_access_iterator<const int*>(ia),
				 random_access_iterator<const int*>(ia + s),
				 random_access_iterator<const int*>(ib),
				 random_access_iterator<const int*>(ib + s),
				 std::equal_to<int>()));
	CHECK(!equal(random_access_iterator<const int*>(ia),
				 sentinel<const int*>(ia + s),
				 random_access_iterator<const int*>(ib),
				 sentinel<const int*>(ib + s),
				 std::equal_to<int>()));
}

void test_rng_pred()
{
	using namespace nano::ranges;
	using nano::ranges::subrange;

	int ia[] = {0, 1, 2, 3, 4, 5};
	constexpr unsigned s = size(ia);
	int ib[s] = {0, 1, 2, 5, 4, 5};
	CHECK(ranges::equal(subrange(input_iterator<const int*>(ia),
									  sentinel<const int*>(ia + s)),
						input_iterator<const int*>(ia),
						std::equal_to<int>()));
	CHECK(ranges::equal(subrange(input_iterator<const int*>(ia),
									  sentinel<const int*>(ia + s)),
						subrange(input_iterator<const int*>(ia),
									  sentinel<const int*>(ia + s)),
						std::equal_to<int>()));
	CHECK(ranges::equal(subrange(random_access_iterator<const int*>(ia),
									  random_access_iterator<const int*>(
											  ia + s)),
						subrange(random_access_iterator<const int*>(ia),
									  random_access_iterator<const int*>(
											  ia + s)),
						std::equal_to<int>()));
	CHECK(ranges::equal(subrange(random_access_iterator<const int*>(ia),
									  sentinel<const int*>(ia + s)),
						subrange(random_access_iterator<const int*>(ia),
									  sentinel<const int*>(ia + s)),
						std::equal_to<int>()));

	comparison_count = 0;
	CHECK(!ranges::equal(subrange(input_iterator<const int*>(ia),
									   sentinel<const int*>(ia + s)),
						 subrange(input_iterator<const int*>(ia),
									   sentinel<const int*>(ia + s - 1)),
						 counting_equals<int>));
	CHECK(comparison_count > 0);
	comparison_count = 0;
	CHECK(!ranges::equal(subrange(random_access_iterator<const int*>(ia),
									   random_access_iterator<const int*>(
											   ia + s)),
						 subrange(random_access_iterator<const int*>(ia),
									   random_access_iterator<const int*>(
											   ia + s - 1)),
						 counting_equals<int>));
	CHECK(comparison_count == 0);
	comparison_count = 0;
	CHECK(!ranges::equal(subrange(random_access_iterator<const int*>(ia),
									   sentinel<const int*>(ia + s)),
						 subrange(random_access_iterator<const int*>(ia),
									   sentinel<const int*>(ia + s - 1)),
						 counting_equals<int>));
	CHECK(comparison_count > 0);
	CHECK(!ranges::equal(subrange(input_iterator<const int*>(ia),
									   sentinel<const int*>(ia + s)),
						 input_iterator<const int*>(ib),
						 std::equal_to<int>()));
	CHECK(!ranges::equal(subrange(input_iterator<const int*>(ia),
									   sentinel<const int*>(ia + s)),
						 subrange(input_iterator<const int*>(ib),
									   sentinel<const int*>(ib + s)),
						 std::equal_to<int>()));
	CHECK(!ranges::equal(subrange(random_access_iterator<const int*>(ia),
									   random_access_iterator<const int*>(
											   ia + s)),
						 subrange(random_access_iterator<const int*>(ib),
									   random_access_iterator<const int*>(
											   ib + s)),
						 std::equal_to<int>()));
	CHECK(!ranges::equal(subrange(random_access_iterator<const int*>(ia),
									   sentinel<const int*>(ia + s)),
						 subrange(random_access_iterator<const int*>(ib),
									   sentinel<const int*>(ib + s)),
						 std::equal_to<int>()));
}

}

TEST_CASE("alg.equal")
{
	::test();
	::test_rng();
	::test_pred();
	::test_rng_pred();

	{
		int const a[] = {1,2,3};
		int const b[] = {1,2,3};
		CHECK(ranges::equal(a, ranges::begin(b)));
		CHECK(ranges::equal(ranges::begin(a), ranges::end(a), ranges::begin(b)));
	}
}
