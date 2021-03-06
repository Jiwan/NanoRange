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

#include <nanorange/algorithm/min_element.hpp>
#include <memory>
#include <random>
#include <numeric>
#include <algorithm>
#include "../catch.hpp"
#include "../test_utils.hpp"
#include "../test_iterators.hpp"

namespace stl2 = nano;

namespace {

std::mt19937 gen;

template <class Iter, class Sent = Iter>
void
test_iter(Iter first, Sent last)
{
	Iter i = stl2::min_element(first, last);
	if (first != last) {
		for (Iter j = first; j != last; ++j)
			CHECK(!(*j < *i));
	}
	else
		CHECK(i == last);

	auto rng = stl2::subrange(first, last);
	i = stl2::min_element(rng);
	if (first != last) {
		for (Iter j = first; j != last; ++j)
			CHECK(!(*j < *i));
	}
	else
		CHECK(i == last);

	auto res = stl2::min_element(std::move(rng));
	if (first != last) {
		for (Iter j = first; j != last; ++j)
			CHECK(!(*j < *res));
	}
	else
		CHECK(res == last);
}

template <class Iter, class Sent = Iter>
void
test_iter(unsigned N)
{
	std::unique_ptr<int[]> a{new int[N]};
	std::iota(a.get(), a.get() + N, 0);
	std::shuffle(a.get(), a.get() + N, gen);
	test_iter(Iter(a.get()), Sent(a.get() + N));
}

template <class Iter, class Sent = Iter>
void
test_iter()
{
	test_iter<Iter, Sent>(0);
	test_iter<Iter, Sent>(1);
	test_iter<Iter, Sent>(2);
	test_iter<Iter, Sent>(3);
	test_iter<Iter, Sent>(10);
	test_iter<Iter, Sent>(1000);
}

template <class Iter, class Sent = Iter>
void
test_iter_comp(Iter first, Sent last)
{
	Iter i = stl2::min_element(first, last, std::greater<int>());
	if (first != last) {
		for (Iter j = first; j != last; ++j)
			CHECK(!std::greater<int>()(*j, *i));
	}
	else
		CHECK(i == last);

	auto rng = stl2::subrange(first, last);
	i = stl2::min_element(rng, std::greater<int>());
	if (first != last) {
		for (Iter j = first; j != last; ++j)
			CHECK(!std::greater<int>()(*j, *i));
	}
	else
		CHECK(i == last);

	auto res = stl2::min_element(std::move(rng), std::greater<int>());
	if (first != last) {
		for (Iter j = first; j != last; ++j)
			CHECK(!std::greater<int>()(*j, *res));
	}
	else
		CHECK(res == last);
}

template <class Iter, class Sent = Iter>
void
test_iter_comp(unsigned N)
{
	std::unique_ptr<int[]> a{new int[N]};
	std::iota(a.get(), a.get() + N, 0);
	std::shuffle(a.get(), a.get() + N, gen);
	test_iter_comp(Iter(a.get()), Sent(a.get() + N));
}

template <class Iter, class Sent = Iter>
void
test_iter_comp()
{
	test_iter_comp<Iter, Sent>(0);
	test_iter_comp<Iter, Sent>(1);
	test_iter_comp<Iter, Sent>(2);
	test_iter_comp<Iter, Sent>(3);
	test_iter_comp<Iter, Sent>(10);
	test_iter_comp<Iter, Sent>(1000);
}

struct S {
	int i;
};

}

TEST_CASE("alg.min_element")
{
	test_iter<forward_iterator<const int*> >();
	test_iter<bidirectional_iterator<const int*> >();
	test_iter<random_access_iterator<const int*> >();
	test_iter<const int*>();
	test_iter<forward_iterator<const int*>, sentinel<const int*>>();
	test_iter<bidirectional_iterator<const int*>, sentinel<const int*>>();
	test_iter<random_access_iterator<const int*>, sentinel<const int*>>();

	test_iter_comp<forward_iterator<const int*> >();
	test_iter_comp<bidirectional_iterator<const int*> >();
	test_iter_comp<random_access_iterator<const int*> >();
	test_iter_comp<const int*>();
	test_iter_comp<forward_iterator<const int*>, sentinel<const int*>>();
	test_iter_comp<bidirectional_iterator<const int*>, sentinel<const int*>>();
	test_iter_comp<random_access_iterator<const int*>, sentinel<const int*>>();

	// Works with projections?
	S s[] = {S{1},S{2},S{3},S{4},S{-4},S{5},S{6},S{7},S{8},S{9}};
	S const *ps = stl2::min_element(s, std::less<int>{}, &S::i);
	CHECK(ps->i == -4);
}
