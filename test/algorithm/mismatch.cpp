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

#include <nanorange/algorithm/mismatch.hpp>
#include <memory>
#include <algorithm>
#include "../catch.hpp"
#include "../test_iterators.hpp"
#include "../test_utils.hpp"

namespace ranges {
	using namespace nano::ranges;
	using nano::ranges::subrange;
}

namespace {

template <typename I1, typename I2>
bool operator==(const ranges::mismatch_result<I1, I2>& r1,
                const ranges::mismatch_result<I1, I2>& r2)
{
	return r1.in1 == r2.in1 && r1.in2 == r2.in2;
}

template <typename Iter, typename Sent = Iter>
void test_iter()
{
	int ia[] = {0, 1, 2, 2, 0, 1, 2, 3};
	constexpr unsigned sa = ranges::size(ia);
	int ib[] = {0, 1, 2, 3, 0, 1, 2, 3};
	using Pair = ranges::mismatch_result<Iter, Iter>;
	CHECK((ranges::mismatch(Iter(ia), Sent(ia + sa), Iter(ib)) ==
			Pair{Iter(ia + 3), Iter(ib + 3)}));
	CHECK((ranges::mismatch(Iter(ia), Sent(ia + sa), Iter(ib), Sent(ib + sa)) ==
			Pair{Iter(ia + 3), Iter(ib + 3)}));
	CHECK((ranges::mismatch(Iter(ia), Sent(ia + sa), Iter(ib), Sent(ib + 2)) ==
			Pair{Iter(ia + 2), Iter(ib + 2)}));

	CHECK((ranges::mismatch(Iter(ia), Sent(ia + sa), Iter(ib),
							std::equal_to<int>()) ==
			Pair{Iter(ia + 3), Iter(ib + 3)}));
	CHECK((ranges::mismatch(Iter(ia), Sent(ia + sa), Iter(ib), Sent(ib + sa),
							std::equal_to<int>()) ==
			Pair{Iter(ia + 3), Iter(ib + 3)}));
	CHECK((ranges::mismatch(Iter(ia), Sent(ia + sa), Iter(ib), Sent(ib + 2),
							std::equal_to<int>()) ==
			Pair{Iter(ia + 2), Iter(ib + 2)}));
}

template <typename Iter, typename Sent = Iter>
void test_range()
{
	int ia[] = {0, 1, 2, 2, 0, 1, 2, 3};
	constexpr unsigned sa = ranges::size(ia);
	int ib[] = {0, 1, 2, 3, 0, 1, 2, 3};
	using Pair = ranges::mismatch_result<Iter, Iter>;
	auto rng1 = ranges::subrange(Iter(ia), Sent(ia + sa));
	CHECK((ranges::mismatch(rng1, Iter(ib)) ==
			Pair{Iter(ia + 3), Iter(ib + 3)}));
	auto r1 = ranges::mismatch(std::move(rng1), Iter(ib));
	CHECK(r1.in1 == Iter(ia + 3));
	CHECK(r1.in2 == Iter(ib + 3));
	auto rng2 = ranges::subrange(Iter(ia), Sent(ia + sa));
	auto rng3 = ranges::subrange(Iter(ib), Sent(ib + sa));
	CHECK((ranges::mismatch(rng2, rng3) ==
			Pair{Iter(ia + 3), Iter(ib + 3)}));
	auto r2 = ranges::mismatch(std::move(rng2), std::move(rng3));
	CHECK(r2.in1 == Iter(ia + 3));
	CHECK(r2.in2 == Iter(ib + 3));
	auto r3 = ranges::mismatch(rng2, std::move(rng3));
	CHECK(r3.in1 == Iter(ia + 3));
	CHECK(r3.in2 == Iter(ib + 3));
	auto r4 = ranges::mismatch(std::move(rng2), rng3);
	CHECK(r4.in1 == Iter(ia + 3));
	CHECK(r4.in2 == Iter(ib + 3));
	auto rng4 = ranges::subrange(Iter(ia), Sent(ia + sa));
	auto rng5 = ranges::subrange(Iter(ib), Sent(ib + 2));
	CHECK((ranges::mismatch(rng4, rng5) ==
			Pair{Iter(ia + 2), Iter(ib + 2)}));

	auto rng6 = ranges::subrange(Iter(ia), Sent(ia + sa));
	CHECK((ranges::mismatch(rng6, Iter(ib), std::equal_to<int>()) ==
			Pair{Iter(ia + 3), Iter(ib + 3)}));
	auto rng7 = ranges::subrange(Iter(ia), Sent(ia + sa));
	auto rng8 = ranges::subrange(Iter(ib), Sent(ib + sa));
	CHECK((ranges::mismatch(rng7, rng8, std::equal_to<int>()) ==
			Pair{Iter(ia + 3), Iter(ib + 3)}));
	auto rng9 = ranges::subrange(Iter(ia), Sent(ia + sa));
	auto rng10 = ranges::subrange(Iter(ib), Sent(ib + 2));
	CHECK((ranges::mismatch(rng9, rng10, std::equal_to<int>()) ==
			Pair{Iter(ia + 2), Iter(ib + 2)}));
}

struct S {
	int i;
};

}

TEST_CASE("alg.mismatch")
{
	test_iter<input_iterator<const int*>>();
	test_iter<forward_iterator<const int*>>();
	test_iter<bidirectional_iterator<const int*>>();
	test_iter<random_access_iterator<const int*>>();
	test_iter<const int*>();
	test_iter<input_iterator<const int*>, sentinel<const int*>>();
	test_iter<forward_iterator<const int*>, sentinel<const int*>>();
	test_iter<bidirectional_iterator<const int*>, sentinel<const int*>>();
	test_iter<random_access_iterator<const int*>, sentinel<const int*>>();

	test_range<input_iterator<const int*>>();
	test_range<forward_iterator<const int*>>();
	test_range<bidirectional_iterator<const int*>>();
	test_range<random_access_iterator<const int*>>();
	test_range<const int*>();
	test_range<input_iterator<const int*>, sentinel<const int*>>();
	test_range<forward_iterator<const int*>, sentinel<const int*>>();
	test_range<bidirectional_iterator<const int*>, sentinel<const int*>>();
	test_range<random_access_iterator<const int*>, sentinel<const int*>>();

	// Works with projections?
	S s1[] = {S{1},S{2},S{3},S{4},S{-4},S{5},S{6},S{40},S{7},S{8},S{9}};
	int const i1[] = {1,2,3,4,5,6,7,8,9};
	{
		const auto ps1 = ranges::mismatch(s1, i1, std::equal_to<int>(), &S::i);
		CHECK(ps1.in1->i == -4);
		CHECK(*ps1.in2 == 5);
	}
	{
		const auto ps1 = ranges::mismatch(ranges::begin(s1), ranges::end(s1), i1, std::equal_to<int>(), &S::i);
		CHECK(ps1.in1->i == -4);
		CHECK(*ps1.in2 == 5);
	}

	S s2[] = {S{1},S{2},S{3},S{4},S{5},S{6},S{40},S{7},S{8},S{9}};
	{
		const auto ps2 = ranges::mismatch(s1, s2, std::equal_to<int>(), &S::i, &S::i);
		CHECK(ps2.in1->i == -4);
		CHECK(ps2.in2->i == 5);
	}
	{
		const auto ps2 = ranges::mismatch(ranges::begin(s1), ranges::end(s1), s2, std::equal_to<int>(), &S::i, &S::i);
		CHECK(ps2.in1->i == -4);
		CHECK(ps2.in2->i == 5);
	}
}
