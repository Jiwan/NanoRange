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
//

#include "set_symmetric_difference.hpp"
#include <nanorange/algorithm/lexicographical_compare.hpp>

TEST_CASE("alg.set_symmetric_difference6")
{
	// Test projections
	{
		S ia[] = {S{1}, S{2}, S{2}, S{3}, S{3}, S{3}, S{4}, S{4}, S{4}, S{4}};
		T ib[] = {T{2}, T{4}, T{4}, T{6}};
		U ic[20];
		int ir[] = {1, 2, 3, 3, 3, 4, 4, 6};
		const int sr = sizeof(ir)/sizeof(ir[0]);

		stl2::set_symmetric_difference_result<S *, T *, U *> res1 =
			stl2::set_symmetric_difference(ia, ib, ic, std::less<int>(), &S::i, &T::j);
		CHECK((res1.out - ic) == sr);
		CHECK_FALSE(stl2::lexicographical_compare(ic, res1.out, ir, ir+sr, std::less<int>(), &U::k));
		stl2::fill(ic, U{0});

		stl2::set_symmetric_difference_result<T *, S *, U *> res2 =
			stl2::set_symmetric_difference(ib, ia, ic, std::less<int>(), &T::j, &S::i);
		CHECK((res2.out - ic) == sr);
		CHECK_FALSE(stl2::lexicographical_compare(ic, res2.out, ir, ir+sr, std::less<int>(), &U::k));
	}

	// Test rvalue ranges
	{
		S ia[] = {S{1}, S{2}, S{2}, S{3}, S{3}, S{3}, S{4}, S{4}, S{4}, S{4}};
		T ib[] = {T{2}, T{4}, T{4}, T{6}};
		U ic[20];
		int ir[] = {1, 2, 3, 3, 3, 4, 4, 6};
		const int sr = sizeof(ir)/sizeof(ir[0]);

		auto res1 =
			stl2::set_symmetric_difference(std::move(ia), std::move(ib), ic, std::less<int>(), &S::i, &T::j);
		CHECK((res1.out - ic) == sr);
		CHECK_FALSE(stl2::lexicographical_compare(ic, res1.out, ir, ir+sr, std::less<int>(), &U::k));
		stl2::fill(ic, U{0});

		auto res2 =
			stl2::set_symmetric_difference(std::move(ib), std::move(ia), ic, std::less<int>(), &T::j, &S::i);
		CHECK((res2.out - ic) == sr);
		CHECK_FALSE(stl2::lexicographical_compare(ic, res2.out, ir, ir+sr, std::less<int>(), &U::k));
	}
}
