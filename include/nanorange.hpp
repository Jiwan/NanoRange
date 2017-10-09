// nanorange.hpp
//
// Copyright (c) 2017 Tristan Brindle (tcbrindle at gmail dot com)
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef NANORANGE_HPP_INCLUDED
#define NANORANGE_HPP_INCLUDED

#include <algorithm>
#include <functional>
#include <iterator>
#include <numeric>
#include <type_traits>

#ifdef NANORANGE_NO_DEPRECATION_WARNINGS
#define NANORANGE_DEPRECATED
#else
#define NANORANGE_DEPRECATED [[deprecated]]
#endif

namespace nanorange {

namespace detail {
inline namespace begin_end_adl {

using std::begin;
using std::end;

template <typename T>
constexpr auto adl_begin(T&& t) -> decltype(begin(std::forward<T>(t)))
{
    return begin(std::forward<T>(t));
}

template <typename T>
constexpr auto adl_end(T&& t) -> decltype(end(std::forward<T>(t)))
{
    return end(std::forward<T>(t));
}

} // end inline namespace begin_end_adl

inline namespace swap_adl {

using std::swap;

template <typename T, typename U>
constexpr auto adl_swap(T& t, U& u) -> decltype(swap(t, u))
{
    return swap(t, u);
}

} // end inline namespace swap_adl

template <typename T, typename U>
using adl_swap_t = decltype(adl_swap(std::declval<T>(), std::declval<U>()));

/* "Detection idiom" machinery from the Library Fundamentals V2 TS */
template <typename...>
using void_t = void;

struct nonesuch {
    nonesuch() = delete;
    ~nonesuch() = delete;
    nonesuch(nonesuch const&) = delete;
    void operator=(nonesuch const&) = delete;
};

template <class Default, class Void, template <class...> class Op, class... Args>
struct detector {
    using value_t = std::false_type;
    using type = Default;
};

template <class Default, template <class...> class Op, class... Args>
struct detector<Default, void_t<Op<Args...>>, Op, Args...> {
    using value_t = std::true_type;
    using type = Op<Args...>;
};

template <template <class...> class Op, class... Args>
using is_detected = typename detector<nonesuch, void, Op, Args...>::value_t;

template <template <class...> class Op, class... Args>
using detected_t = typename detector<nonesuch, void, Op, Args...>::type;

template <class Default, template <class...> class Op, class... Args>
using detected_or = detector<Default, void, Op, Args...>;

template <template <class...> class Op, class... Args>
constexpr bool is_detected_v = is_detected<Op, Args...>::value;

template <class Default, template <class...> class Op, class... Args>
using detected_or_t = typename detected_or<Default, Op, Args...>::type;

template <class Expected, template <class...> class Op, class... Args>
using is_detected_exact = std::is_same<Expected, detected_t<Op, Args...>>;

template <class Expected, template <class...> class Op, class... Args>
constexpr bool is_detected_exact_v = is_detected_exact<Expected, Op, Args...>::value;

template <class To, template <class...> class Op, class... Args>
using is_detected_convertible = std::is_convertible<detected_t<Op, Args...>, To>;

template <class To, template <class...> class Op, class... Args>
constexpr bool is_detected_convertible_v = is_detected_convertible<To, Op, Args...>::value;

} // namespace detail

/* Core language concepts [7.3] */

#if __cplusplus >= 201700
#define CONCEPT constexpr inline
#else
#define CONCEPT constexpr
#endif

template <typename T, typename U>
CONCEPT bool Same = std::is_same<T, U>::value;

template <typename T, typename U>
CONCEPT bool DerivedFrom =
        std::is_base_of<U, T>::value &&
        std::is_convertible<std::remove_cv_t<T>*, std::remove_cv_t<U>*>::value;

template <typename From, typename To>
CONCEPT bool ConvertibleTo = std::is_convertible<From, To>::value;

// TODO: common_reference_t is a nightmare

template <typename I>
CONCEPT bool Integral = std::is_integral<I>::value;

template <typename I>
CONCEPT bool SignedIntegral = Integral<I> && std::is_signed<I>::value;

template <typename I>
CONCEPT bool UnsignedIntegral = Integral<I> && !SignedIntegral<I>;

template <typename T, typename U>
CONCEPT bool Assignable = std::is_assignable<T, U>::value;

template <typename T>
CONCEPT bool Swappable = detail::is_detected_v<detail::adl_swap_t, T&, T&>;

template <typename T, typename U>
CONCEPT bool SwappableWith =
        detail::is_detected_v<detail::adl_swap_t, T&, U&> &&
        detail::is_detected_v<detail::adl_swap_t, U&, T&>;

template <typename T>
CONCEPT bool Destructible = std::is_nothrow_destructible<T>::value;

template <typename T, typename... Args>
CONCEPT bool Constructible = Destructible<T> &&
        std::is_constructible<T, Args...>::value;

template <typename T>
CONCEPT bool DefaultConstructible = Constructible<T>;

template <typename T>
CONCEPT bool MoveConstructible =
        Constructible<T, T> &&
        ConvertibleTo<T, T>;

template <typename T>
CONCEPT bool CopyConstructible =
        MoveConstructible<T> &&
        Constructible<T, T&> && ConvertibleTo<T&, T> &&
        Constructible<T, const T&> && ConvertibleTo<const T&, T> &&
        Constructible<T, const T> && ConvertibleTo<const T, T>;


/* 7.4 Comparison Concepts */

namespace detail {

template <typename T, typename U>
auto equal_to(T t, U u) -> decltype(t == u);

template <typename T, typename U>
using equal_to_t = decltype(equal_to(std::declval<T>(), std::declval<U>()));

template <typename T, typename U>
auto not_equal_to(T t, U u) -> decltype(t != u);

template <typename T, typename U>
using not_equal_to_t = decltype(not_equal_to(std::declval<T>(), std::declval<U>()));

template <typename T, typename U>
auto less_than(T t, U u) -> decltype(t < u);

template <typename T, typename U>
using less_than_t = decltype(less_than(std::declval<T>(), std::declval<U>()));

template <typename T, typename U>
auto greater_than(T t, U u) -> decltype(t > u);

template <typename T, typename U>
using greater_than_t = decltype(greater_than(std::declval<T>(), std::declval<U>()));

template <typename T, typename U>
auto less_than_eq(T t, U u) -> decltype(t <= u);

template <typename T, typename U>
using less_than_eq_t = decltype(less_than_eq(std::declval<T>(), std::declval<U>()));

template <typename T, typename U>
auto greater_than_eq(T t, U u) -> decltype(t >= u);

template <typename T, typename U>
using greater_than_eq_t = decltype(greater_than_eq(std::declval<T>(), std::declval<U>()));

template <typename T>
auto pre_inc(T t) -> decltype(++t);

template <typename T>
using pre_inc_t = decltype(pre_inc(std::declval<T>()));

template <typename T>
auto post_inc(T t) -> decltype(t++);

template <typename T>
using post_inc_t = decltype(post_inc(std::declval<T>()));

template <typename I>
auto pre_dec(I i) -> decltype(--i);

template <typename T>
using pre_dec_t = decltype(pre_dec(std::declval<T>()));

template <typename I>
auto post_dec(I i) -> decltype(i--);

template <typename T>
using post_dec_t = decltype(post_dec(std::declval<T>()));

template <typename T>
auto deref(T t) -> decltype(*t);

template <typename T>
using deref_t = decltype(deref(std::declval<T>()));

}


// TODO: Do this properly
template <typename B>
CONCEPT bool Boolean =
        ConvertibleTo<B, bool>;

template <typename T, typename U>
CONCEPT bool WeaklyComparibleWith =
        Boolean<detail::detected_t<detail::equal_to_t, T, U>> &&
        Boolean<detail::detected_t<detail::not_equal_to_t, T, U>> &&
        Boolean<detail::detected_t<detail::equal_to_t, U, T>> &&
        Boolean<detail::detected_t<detail::not_equal_to_t, U, T>>;

template <typename T>
CONCEPT bool EqualityComparible = WeaklyComparibleWith<T, T>;

template <typename T, typename U>
CONCEPT bool EqualityComparibleWith =
        EqualityComparible<T> &&
        EqualityComparible<U> &&
        WeaklyComparibleWith<T, U>;

template <typename T>
CONCEPT bool StrictTotallyOrdered =
        EqualityComparible<T> &&
        Boolean<detail::detected_t<detail::less_than_t, T, T>> &&
        Boolean<detail::detected_t<detail::less_than_eq_t, T, T>> &&
        Boolean<detail::detected_t<detail::greater_than_t, T, T>> &&
        Boolean<detail::detected_t<detail::greater_than_eq_t, T, T>>;

template <typename T, typename U>
CONCEPT bool StrictTotallyOrderedWith =
        StrictTotallyOrdered<T> &&
        StrictTotallyOrdered<U> &&
        EqualityComparibleWith<T, U> &&
        Boolean<detail::detected_t<detail::less_than_t, T, U>> &&
        Boolean<detail::detected_t<detail::less_than_eq_t, T, U>> &&
        Boolean<detail::detected_t<detail::greater_than_t, T, U>> &&
        Boolean<detail::detected_t<detail::greater_than_eq_t, T, U>> &&
        Boolean<detail::detected_t<detail::less_than_t, U, T>> &&
        Boolean<detail::detected_t<detail::less_than_eq_t, U, T>> &&
        Boolean<detail::detected_t<detail::greater_than_t, U, T>> &&
        Boolean<detail::detected_t<detail::greater_than_eq_t, U, T>>;

/* 7.5 Object Concepts */

template <typename T>
CONCEPT bool Movable =
        std::is_object<T>::value &&
        MoveConstructible<T> &&
        Assignable<T&, T> &&
        Swappable<T>;

template <typename T>
CONCEPT bool Copyable =
        CopyConstructible<T> &&
        Movable<T> &&
        Assignable<T&, const T&>;

template <typename T>
CONCEPT bool Semiregular =
        Copyable<T> &&
        DefaultConstructible<T>;

template <typename T>
CONCEPT bool Regular =
        Semiregular<T> &&
        EqualityComparible<T>;

/* 7.6 Callable Concepts */

namespace detail {

template <typename, typename = void>
struct is_callable : std::false_type {};

template <typename F, typename... Args>
struct is_callable<F(Args...), detail::void_t<std::result_of_t<F&&(
        Args&& ...)>>>
        : std::true_type {};

}

template <typename F, typename... Args>
CONCEPT bool Callable = detail::is_callable<F(Args...)>::value;

template <typename F, typename... Args>
CONCEPT bool RegularCallable = Callable<F, Args...>;

namespace detail {

template <typename, typename = void>
struct is_predicate : std::false_type {};

template <typename F, typename... Args>
struct is_predicate<F(Args...), std::enable_if_t<
                Callable<F, Args...> &&
                Boolean<std::result_of_t<F&&(Args&& ...)>>>>
        : std::true_type {};

}

template <typename F, typename... Args>
CONCEPT bool Predicate = detail::is_predicate<F(Args...)>::value;

template <typename R, typename T, typename U>
CONCEPT bool Relation =
        Predicate<R, T, T> &&
                Predicate<R, U, U> &&
                Predicate<R, T, U> &&
                Predicate<R, U, T>;

template <typename R, typename T, typename U>
CONCEPT bool StrictWeakOrder = Relation<R, T, U>;

/* 9.1 Iterators library */

// FIXME: This is not correct
template <typename E>
auto iter_move(E&& e) -> decltype(std::move(*e))
{
    return std::move(*e);
}

template <typename, typename = void>
struct difference_type {};

template <typename T>
struct difference_type<T*>
    : std::enable_if<std::is_object<T>::value, std::ptrdiff_t> {};

template <typename I>
struct difference_type<const I> : difference_type<std::decay_t<I>> {};

namespace detail {

template <typename T>
using member_difference_type_t = typename T::difference_type;

}

template <typename T>
struct difference_type<T, detail::void_t<typename T::difference_type>> {
    using type = typename T::difference_type;
};

template <typename T>
struct difference_type<T, std::enable_if_t<
        !std::is_pointer<T>::value &&
        !detail::is_detected_v<detail::member_difference_type_t, T> &&
        Integral<decltype(std::declval<const T&>() - std::declval<const T&>())>>>
    : std::make_signed<decltype(std::declval<T>() - std::declval<T>())> {};

template <typename T>
using difference_type_t = typename difference_type<T>::type;

template <typename, typename = void>
struct value_type {};

template <typename T>
struct value_type<T*>
    : std::enable_if<std::is_object<T>::value, std::remove_cv_t<T>> { };

template <typename I>
struct value_type<I, std::enable_if_t<std::is_array<I>::value>>
    : value_type<std::decay_t<I>> {};

template <typename I>
struct value_type<const I> : value_type<std::decay_t<I>> {};

namespace detail {

template <typename T>
using member_value_type_t = typename T::value_type;

template <typename T>
using member_element_type_t = typename T::element_type;

}

template <typename T>
struct value_type<T, std::enable_if_t<detail::is_detected_v<detail::member_value_type_t, T>>>
    : std::enable_if<std::is_object<typename T::value_type>::value, typename T::value_type> {};

template <typename T>
struct value_type<T, std::enable_if_t<detail::is_detected_v<detail::member_element_type_t, T>>>
    : std::enable_if<std::is_object<typename T::element_type>::value,
                     std::remove_cv_t<typename T::element_type>> {};

template <typename T>
using value_type_t = typename value_type<T>::type;

// N.B. we use std::iterator_traits here, rather than a nested type
template <typename T>
using iterator_category_t = typename std::iterator_traits<T>::iterator_category;

template <typename T>
using reference_t = decltype(*std::declval<T&>());

template <typename T>
using rvalue_reference_t = decltype(nanorange::iter_move(std::declval<T&>()));

template <typename In>
CONCEPT bool Readable =
        detail::is_detected_v<value_type_t, In> &&
        detail::is_detected_v<reference_t, In> &&
        detail::is_detected_v<rvalue_reference_t, In>;


namespace detail {

template <typename Out, typename T>
auto write_ops_test(Out&& o, T&& t)
        -> decltype(*o = std::forward<T>(t),
                    const_cast<const reference_t<Out>&&>(*o) = std::forward<T>(t),
                    const_cast<const reference_t<Out>&&>(*std::forward<Out>(o)) = std::forward<T>(t));

template <typename Out, typename T>
using write_ops_t = decltype(write_ops_test(std::declval<Out>(), std::declval<T>()));

}

template <typename Out, typename T>
CONCEPT bool Writable = detail::is_detected_v<detail::write_ops_t, Out, T>;

template <typename I>
CONCEPT bool WeaklyIncrementable =
        Semiregular<I> &&
        detail::is_detected_v<difference_type_t, I> &&
        SignedIntegral<detail::detected_t<difference_type_t, I>> &&
        detail::is_detected_exact_v<I&, detail::pre_inc_t, I> &&
        detail::is_detected_v<detail::post_inc_t, I&>;

template <typename I>
CONCEPT bool Incrementable =
        Regular<I> &&
        WeaklyIncrementable<I> &&
        detail::is_detected_exact_v<I, detail::post_inc_t, I>;


namespace detail {

template <typename T>
struct legacy_iterator_traits {
    using value_type_t = typename std::iterator_traits<T>::value_type;
    using difference_t = typename std::iterator_traits<T>::difference_type;
    using reference_t = typename std::iterator_traits<T>::reference;
    using pointer = typename std::iterator_traits<T>::pointer;
    using iterator_category = typename std::iterator_traits<T>::iterator_category;
};

}

template <typename I>
CONCEPT bool Iterator =
        detail::is_detected_v<detail::deref_t, I> &&
        !std::is_void<detail::detected_t<detail::deref_t, I>>::value &&
        WeaklyIncrementable<I> &&
        detail::is_detected_v<detail::legacy_iterator_traits, I>;

template <typename I>
CONCEPT bool InputIterator =
        Iterator<I> &&
        Readable<I> &&
        DerivedFrom<detail::detected_t<iterator_category_t, I>, std::input_iterator_tag> &&
        EqualityComparible<I>;

namespace detail {

template <typename I, typename T>
auto output_iter_ops(I i, T&& t) -> decltype(*i++ = std::forward<T>(t));

template <typename I, typename T>
using output_iter_ops_t = decltype(output_iter_ops(std::declval<I>(), std::declval<T>()));

}

template <typename I, typename T>
CONCEPT bool OutputIterator =
        Iterator<I> &&
        Writable<I, T> &&
        detail::is_detected_v<detail::output_iter_ops_t, I, T>;


template <typename I>
CONCEPT bool ForwardIterator =
        InputIterator<I> &&
        ConvertibleTo<detail::detected_t<iterator_category_t, I>, std::forward_iterator_tag> &&
        Incrementable<I>;

template <typename I>
CONCEPT bool BidirectionalIterator =
    ForwardIterator<I> &&
            DerivedFrom<detail::detected_t<iterator_category_t, I>, std::bidirectional_iterator_tag> &&
            detail::is_detected_exact_v<I&, detail::pre_dec_t, I> &&
            detail::is_detected_exact_v<I, detail::post_dec_t, I>;

namespace detail {

template <typename I>
auto random_access_iter_ops(I i, const I j, const difference_type_t<I> n) -> decltype(
    i += n,
    j + n,
    n + j,
    i -= n,
    j - n,
    j[n],
    Same<decltype(j[n]), reference_t<I>>
);

template <typename I>
using random_access_iter = decltype(random_access_iter_ops(
        std::declval<I>(),
        std::declval<I>(),
        std::declval<difference_type_t<I>>()));

}

template <typename I>
CONCEPT bool RandomAccessIterator =
        BidirectionalIterator<I> &&
        DerivedFrom<detail::detected_t<iterator_category_t, I>, std::random_access_iterator_tag> &&
        StrictTotallyOrdered<I> &&
        detail::is_detected_v<detail::random_access_iter, I>;


/* 9.4 Indirect callable concepts */

template <typename F, typename I>
CONCEPT bool IndirectUnaryCallable =
        Readable<I> &&
        CopyConstructible<F> &&
        Callable<F&, value_type_t<I>> &&
        Callable<F&, reference_t<I>>;

template <typename F, typename I>
CONCEPT bool IndirectRegularUnaryCallable =
        Readable<I> &&
        CopyConstructible<F> &&
        RegularCallable<F&, value_type_t<I>> &&
        RegularCallable<F&, reference_t<I>>;

template <typename F, typename I>
CONCEPT bool IndirectUnaryPredicate =
        Readable<I> &&
        CopyConstructible<F> &&
        Predicate<F&, value_type_t<I>&> &&
        Predicate<F&, reference_t<I>>;

template <typename F, typename I1, typename I2 = I1>
CONCEPT bool IndirectRelation =
        Readable<I1> && Readable<I2> &&
        CopyConstructible<F> &&
        Relation<F&, value_type_t<I1>&, value_type_t<I2>&> &&
        Relation<F&, value_type_t<I1>&, reference_t<I2>> &&
        Relation<F&, reference_t<I1>, value_type_t<I2>&> &&
        Relation<F&, reference_t<I1>, reference_t<I2>>;

template <typename F, typename I1, typename I2 = I1>
CONCEPT bool IndirectStrictWeakOrder =
        Readable<I1> && Readable<I2> &&
        CopyConstructible<F> &&
        StrictWeakOrder<F&, value_type_t<I1>&, value_type_t<I2>&> &&
        StrictWeakOrder<F&, value_type_t<I1>&, reference_t<I2>> &&
        StrictWeakOrder<F&, reference_t<I1>, value_type_t<I2>&> &&
        StrictWeakOrder<F&, reference_t<I1>, reference_t<I2>>;

template <typename, typename = void> struct indirect_result_of {};

template <typename F, typename... Is>
struct indirect_result_of<F(Is...), std::enable_if_t<Callable<F, reference_t<Is>...>>>
    : std::result_of<F(reference_t<Is>&&...)> {};

template <typename F>
using indirect_result_of_t = typename indirect_result_of<F>::type;


/* 9.5 Common algorithm requirements */

template <typename In, typename Out>
CONCEPT bool IndirectlyMovable =
        Readable<In> &&
        Writable<Out, rvalue_reference_t<In>>;

template <typename In, typename Out>
CONCEPT bool IndirectlyMovableStorable =
        IndirectlyMovable<In, Out> &&
        Writable<Out, value_type_t<In>> &&
        Movable<value_type_t<In>> &&
        Constructible<value_type_t<In>, rvalue_reference_t<In>> &&
        Assignable<value_type_t<In>&, rvalue_reference_t<In>>;

template <typename In, typename Out>
CONCEPT bool IndirectlyCopyable =
        Readable<In> &&
        Writable<Out, reference_t<In>>;

template <typename In, typename Out>
CONCEPT bool IndirectlyCopyableStorable =
        IndirectlyCopyable<In, Out> &&
        Writable<Out, const value_type_t<In>&> &&
        Copyable<value_type_t<In>> &&
        Constructible<value_type_t<In>, reference_t<In>> &&
        Assignable<value_type_t<In>&, reference_t<In>>;


template <typename I1, typename I2 = I1>
CONCEPT bool IndirectlySwappable =
        Readable<I1> && Readable<I2>; // FIXME: add iter_swap requirements

template <typename I1, typename I2, typename R = std::equal_to<>>
CONCEPT bool IndirectlyComparable =
        IndirectRelation<R, I1, I2>;

template <typename I>
CONCEPT bool Permutable =
        ForwardIterator<I> &&
        IndirectlyMovableStorable<I, I> &&
        IndirectlySwappable<I, I>;

template <typename I1, typename I2, typename Out, typename R = std::less<>>
CONCEPT bool Mergeable =
        InputIterator<I1> &&
        InputIterator<I2> &&
        WeaklyIncrementable<Out> &&
        IndirectlyCopyable<I1, Out> &&
        IndirectlyCopyable<I2, Out> &&
        IndirectStrictWeakOrder<R, I1, I2>;

template <typename I, typename R = std::less<>>
CONCEPT bool Sortable =
        Permutable<I> &&
        IndirectStrictWeakOrder<R, I>;

/* 10.0 Ranges Library */

template <typename Rng>
using iterator_t = decltype(detail::adl_begin(std::declval<Rng&>()));

template <typename Rng>
using sentinel_t = decltype(detail::adl_end(std::declval<Rng&>()));

template <typename Rng>
using range_value_type_t = value_type_t<iterator_t<Rng>>;

template <typename Rng>
using range_difference_type_t = difference_type_t<iterator_t<Rng>>;

// For our purposes, a type is a range if the result of begin() is an
// iterator, and begin() and end() return the same type
template <typename T>
CONCEPT bool Range =
        Iterator<detail::detected_t<iterator_t, T>> &&
        Same<detail::detected_t<iterator_t, T>, detail::detected_t<sentinel_t, T>>;


template <typename T>
CONCEPT bool BoundedRange = Range<T>;

template <typename T>
CONCEPT bool InputRange =
        Range<T> &&
        InputIterator<detail::detected_t<iterator_t, T>>;

template <typename R, typename T>
CONCEPT bool OutputRange =
        Range<R> &&
        OutputIterator<detail::detected_t<iterator_t, R>, T>;

template <typename T>
CONCEPT bool ForwardRange =
        Range<T> &&
        ForwardIterator<detail::detected_t<iterator_t, T>>;

template <typename T>
CONCEPT bool BidirectionalRange =
        Range<T> &&
        BidirectionalIterator<detail::detected_t<iterator_t, T>>;

template <typename T>
CONCEPT bool RandomAccessRange =
        Range<T> &&
        RandomAccessIterator<detail::detected_t<iterator_t, T>>;


#define REQUIRES(...) std::enable_if_t<__VA_ARGS__, int> = 0

template <typename Container>
struct back_insert_iterator
{
    using container_type = Container;
    using difference_type = std::ptrdiff_t;

    /* Backwards compatibility typedefs */
    using value_type = void;
    using reference = void;
    using pointer = void;
    using iterator_category = std::output_iterator_tag;

    constexpr back_insert_iterator() = default;

    explicit back_insert_iterator(Container& x)
        : cont_(std::addressof(x))
    {}

    back_insert_iterator& operator=(const value_type_t<Container>& value)
    {
        cont_->push_back(value);
        return *this;
    }

    back_insert_iterator& operator=(value_type_t<Container>&& value)
    {
        cont_->push_back(std::move(value));
        return *this;
    }

    back_insert_iterator& operator*() { return *this; }
    back_insert_iterator& operator++() { return *this; }
    back_insert_iterator& operator++(int) { return *this; }

private:
    container_type* cont_ = nullptr;
};

template <typename Container>
back_insert_iterator<Container> back_inserter(Container& x)
{
    return back_insert_iterator<Container>(x);
}

template <typename Container>
struct front_insert_iterator
{
    using container_type = Container;
    using difference_type = std::ptrdiff_t;

    /* Backwards compatibility typedefs */
    using value_type = void;
    using reference = void;
    using pointer = void;
    using iterator_category = std::output_iterator_tag;

    constexpr front_insert_iterator() = default;

    explicit front_insert_iterator(Container& x)
            : cont_(std::addressof(x))
    {}

    front_insert_iterator& operator=(const value_type_t<Container>& value)
    {
        cont_->push_front(value);
        return *this;
    }

    front_insert_iterator& operator=(value_type_t<Container>&& value)
    {
        cont_->front_back(std::move(value));
        return *this;
    }

    front_insert_iterator& operator*() { return *this; }
    front_insert_iterator& operator++() { return *this; }
    front_insert_iterator& operator++(int) { return *this; }

private:
    container_type* cont_ = nullptr;
};

template <typename Container>
front_insert_iterator<Container> front_inserter(Container& x)
{
    return front_insert_iterator<Container>(x);
}

template <typename Container>
struct insert_iterator
{
    using container_type = Container;
    using difference_type = std::ptrdiff_t;

    /* Backwards compatibility typedefs */
    using value_type = void;
    using reference = void;
    using pointer = void;
    using iterator_category = std::output_iterator_tag;

    constexpr insert_iterator() = default;

    explicit insert_iterator(Container& x, iterator_t<Container> i)
            : cont_(std::addressof(x)), it_(i)
    {}

    insert_iterator& operator=(const value_type_t<Container>& value)
    {
        cont_->insert(it_, value);
        ++it_;
        return *this;
    }

    insert_iterator& operator=(value_type_t<Container>&& value)
    {
        cont_->push_back(it_, std::move(value));
        ++it_;
        return *this;
    }

    insert_iterator& operator*() { return *this; }
    insert_iterator& operator++() { return *this; }
    insert_iterator& operator++(int) { return *this; }

private:
    container_type* cont_ = nullptr;
    iterator_t<container_type> it_{};
};

template <typename Container>
insert_iterator<Container> inserter(Container& x)
{
    return back_insert_iterator<Container>(x);
}

template <typename T, typename CharT = char, typename Traits = std::char_traits<CharT>>
struct ostream_iterator
{
    using char_type = CharT;
    using traits_type = Traits;
    using ostream_type = std::basic_ostream<CharT, Traits>;
    using difference_type = std::ptrdiff_t;

    // Backwards compatibility traits
    using value_type = void;
    using reference = void;
    using pointer = void;
    using iterator_category = std::output_iterator_tag;

    constexpr ostream_iterator() noexcept = default;

    ostream_iterator(ostream_type& os, const CharT* delim = nullptr) noexcept
            : os_(std::addressof(os)), delim_(delim)
    {}

    ostream_iterator& operator=(const T& value)
    {
        *os_ << value;
        if (delim_) {
            *os_ << delim_;
        }
        return *this;
    }

    ostream_iterator& operator*() { return *this; }
    ostream_iterator& operator++() { return *this; }
    ostream_iterator& operator++(int) { return *this; }

private:
    ostream_type* os_ = nullptr;
    const char_type* delim_ = nullptr;
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
struct ostreambuf_iterator {

    using char_type = CharT;
    using traits = Traits;
    using difference_type = std::ptrdiff_t;
    using streambuf_type = std::basic_streambuf<CharT, Traits>;
    using ostream_type = std::basic_ostream<CharT, Traits>;

    // backwards compatibility traits
    using value_type = void;
    using reference = void;
    using pointer = void;
    using iterator_category = std::output_iterator_tag;

    constexpr ostreambuf_iterator() = default;

    ostreambuf_iterator(ostream_type& s) noexcept
        : sbuf_(s.rdbuf())
    {}

    ostreambuf_iterator(streambuf_type* s) noexcept
        : sbuf_(s)
    {}

    ostreambuf_iterator& operator=(char_type c)
    {
        if (!failed()) {
            failed_ = (sbuf_->sputc(c) == traits::eof());
        }
        return *this;
    }

    ostreambuf_iterator& operator*() { return *this; }
    ostreambuf_iterator& operator++() { return *this; }
    ostreambuf_iterator& operator++(int) { return *this; }

    bool failed() const noexcept { return failed_; }

private:
    streambuf_type* sbuf_ = nullptr;
    bool failed_ = false;
};


template <typename T>
struct dangling {

    template <typename U = T,
              REQUIRES(std::is_default_constructible<U>::value)>
    dangling() : value_{} {}

    dangling(T t) : value_(t) {}

    T get_unsafe() { return value_; }

private:
    T value_;
};

template <typename Range>
using safe_iterator_t = std::conditional_t<
        std::is_lvalue_reference<Range>::value,
        iterator_t<Range>, dangling<iterator_t<Range>>>;

// Non-modifying sequence operations
template <typename Iter, typename UnaryPredicate,
          REQUIRES(InputIterator<Iter> &&
                   IndirectUnaryPredicate<UnaryPredicate, Iter>)>
bool all_of(Iter first, Iter last, UnaryPredicate pred)
{
    return std::all_of(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename UnaryPredicate,
          REQUIRES(InputRange<Range> &&
                   IndirectUnaryPredicate<UnaryPredicate, iterator_t<Range>>)>
bool all_of(Range&& range, UnaryPredicate pred)
{
    return std::all_of(detail::adl_begin(range), detail::adl_end(range), std::move(pred));
}

template <typename Iter, typename UnaryPredicate,
          REQUIRES(InputIterator<Iter> &&
                   IndirectUnaryPredicate<UnaryPredicate, Iter>)>
bool any_of(Iter first, Iter last, UnaryPredicate pred)
{
    return std::any_of(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename UnaryPredicate,
          REQUIRES(InputRange<Range> &&
                   IndirectUnaryPredicate<UnaryPredicate, iterator_t<Range>>)>
bool any_of(Range&& range, UnaryPredicate pred)
{
    return std::any_of(detail::adl_begin(range), detail::adl_end(range), std::move(pred));
}

template <typename Iter, typename UnaryPredicate,
          REQUIRES(InputIterator<Iter> &&
                   IndirectUnaryPredicate<UnaryPredicate, Iter>)>
bool none_of(Iter first, Iter last, UnaryPredicate pred)
{
    return std::none_of(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename UnaryPredicate,
          REQUIRES(InputRange<Range> &&
                   IndirectUnaryPredicate<UnaryPredicate, iterator_t<Range>>)>
bool none_of(Range&& range, UnaryPredicate pred)
{
    return std::none_of(detail::adl_begin(range), detail::adl_end(range), std::move(pred));
}

template <typename Iter, typename UnaryFunction,
          REQUIRES(InputIterator<Iter> &&
                   IndirectUnaryCallable<UnaryFunction, Iter>)>
UnaryFunction for_each(Iter first, Iter last, UnaryFunction func)
{
    return std::for_each(std::move(first), std::move(last), std::move(func));
}

template <typename Range, typename UnaryFunction,
          REQUIRES(InputRange<Range> &&
                   IndirectUnaryCallable<UnaryFunction, iterator_t<Range>>)>
UnaryFunction for_each(Range&& range, UnaryFunction func)
{
    return std::for_each(detail::adl_begin(range), detail::adl_end(range),
                         std::move(func));
}

template <typename Iter, typename T,
          REQUIRES(InputIterator<Iter> &&
                   IndirectRelation<std::equal_to<>, Iter, const T*>)>
difference_type_t<Iter>
count(Iter first, Iter last, const T& value)
{
    return std::count(std::move(first), std::move(last), value);
}

template <typename Range, typename T,
          REQUIRES(InputRange<Range> &&
                   IndirectRelation<std::equal_to<>, iterator_t<Range>, const T*>)>
range_difference_type_t<Range>
count(Range&& rng, const T& value)
{
    return std::count(detail::adl_begin(rng), detail::adl_end(rng), value);
}

template <typename Iter, typename UnaryPredicate,
          REQUIRES(InputIterator<Iter> &&
                   IndirectUnaryPredicate<UnaryPredicate, Iter>)>
difference_type_t<Iter>
count_if(Iter first, Iter last, UnaryPredicate pred)
{
    return std::count_if(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename UnaryPredicate,
          REQUIRES(InputRange<Range> &&
                   IndirectUnaryPredicate<UnaryPredicate, iterator_t<Range>>)>
range_difference_type_t<Range>
count_if(Range&& range, UnaryPredicate pred)
{
    return std::count_if(detail::adl_begin(range), detail::adl_end(range), std::move(pred));
}

// N.B. The "three-legged" forms of mismatch() are dangerous and should be
// avoided, so we mark them as deprecated
template <typename Iter1, typename Iter2, typename BinaryPredicate = std::equal_to<>,
          REQUIRES(InputIterator<Iter1> &&
                   InputIterator<Iter2> &&
                   IndirectRelation<BinaryPredicate, Iter1, Iter2>)>
NANORANGE_DEPRECATED
std::pair<Iter1, Iter2>
mismatch(Iter1 first1, Iter2 last1, Iter2 first2, BinaryPredicate pred = {})
{
    return std::mismatch(std::move(first1), std::move(last1), std::move(first2), std::move(pred));
}

template <typename Iter1, typename Iter2, typename BinaryPredicate = std::equal_to<>,
        REQUIRES(InputIterator<Iter1> &&
                 InputIterator<Iter2> &&
                 IndirectRelation<BinaryPredicate, Iter1, Iter2>)>
std::pair<Iter1, Iter2>
mismatch(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2, BinaryPredicate pred = {})
{
    return std::mismatch(std::move(first1), std::move(last1),
                         std::move(first2), std::move(last2),
                         std::move(pred));
}

template <typename Range1, typename Range2, typename BinaryPredicate = std::equal_to<>,
        REQUIRES(InputRange<Range1> &&
                 InputRange<Range2> &&
                 IndirectRelation<BinaryPredicate, iterator_t<Range1>, iterator_t<Range2>>)>
std::pair<safe_iterator_t<Range1>, safe_iterator_t<Range2>>
mismatch(Range1&& range1, Range2&& range2, BinaryPredicate pred = {})
{
    return std::mismatch(detail::adl_begin(range1), detail::adl_end(range1),
                         detail::adl_begin(range2), detail::adl_end(range2),
                         std::move(pred));
}

// Again, the three-legged form of equal() is discouraged
template <typename Iter1, typename Iter2, typename BinaryPredicate = std::equal_to<>,
        REQUIRES(InputIterator<Iter1> &&
                 InputIterator<Iter2> &&
                 IndirectlyComparable<Iter1, Iter2, BinaryPredicate>)>
NANORANGE_DEPRECATED
bool equal(Iter1 first1, Iter1 last1, Iter2 first2, BinaryPredicate pred = {})
{
    return std::equal(std::move(first1), std::move(last1),
                      std::move(first2), std::move(pred));
}

template <typename Iter1, typename Iter2, typename BinaryPredicate = std::equal_to<>,
        REQUIRES(InputIterator<Iter1> &&
                 InputIterator<Iter2> &&
                 IndirectlyComparable<Iter1, Iter2, BinaryPredicate>)>
bool equal(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2, BinaryPredicate pred = {})
{
    return std::equal(std::move(first1), std::move(last1),
                      std::move(first2), std::move(last2),
                      std::move(pred));
}

template <typename Range1, typename Range2, typename BinaryPredicate = std::equal_to<>,
        REQUIRES(InputRange<Range1> &&
                 InputRange<Range2> &&
                 IndirectlyComparable<iterator_t<Range1>, iterator_t<Range2>, BinaryPredicate>)>
bool equal(Range1&& range1, Range2&& range2, BinaryPredicate pred = {})
{
    return std::equal(detail::adl_begin(range1), detail::adl_end(range1),
                      detail::adl_begin(range2), detail::adl_end(range2),
                      std::move(pred));
}

template <typename Iter, typename T,
          REQUIRES(InputIterator<Iter> &&
                   IndirectRelation<std::equal_to<>, Iter, const T*>)>
Iter find(Iter first, Iter last, const T& value)
{
    return std::find(std::move(first), std::move(last), value);
}

template <typename Range, typename T,
          REQUIRES(InputRange<Range> &&
                   IndirectRelation<std::equal_to<>, iterator_t<Range>, const T*>)>
safe_iterator_t<Range>
find(Range&& range, const T& value)
{
    return std::find(detail::adl_begin(range), detail::adl_end(range), value);
}

template <typename Iter, typename UnaryPredicate,
          REQUIRES(InputIterator<Iter> &&
                   IndirectUnaryPredicate<UnaryPredicate, Iter>)>
Iter find_if(Iter first, Iter last, UnaryPredicate pred)
{
    return std::find_if(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename UnaryPredicate,
          REQUIRES(InputRange<Range> &&
                   IndirectUnaryPredicate<UnaryPredicate, iterator_t<Range>>)>
safe_iterator_t<Range>
find_if(Range&& range, UnaryPredicate pred)
{
    return std::find_if(detail::adl_begin(range), detail::adl_end(range),
                        std::move(pred));
}

template <typename Iter, typename UnaryPredicate,
          REQUIRES(InputIterator<Iter> &&
                   IndirectUnaryPredicate<UnaryPredicate, Iter>)>
Iter find_if_not(Iter first, Iter last, UnaryPredicate&& pred)
{
    return std::find_if_not(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename UnaryPredicate,
          REQUIRES(InputRange<Range> &&
                   IndirectUnaryPredicate<UnaryPredicate, iterator_t<Range>>)>
safe_iterator_t<Range>
find_if_not(Range&& range, UnaryPredicate pred)
{
    return std::find_if_not(detail::adl_begin(range), detail::adl_end(range),
                            std::move(pred));
}

template <typename Iter1, typename Iter2, typename BinaryPredicate = std::equal_to<>,
        REQUIRES(ForwardIterator<Iter1> &&
                 ForwardIterator<Iter2> &&
                 IndirectRelation<BinaryPredicate, Iter1, Iter2>)>
Iter1 find_end(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2, BinaryPredicate pred = {})
{
    return std::find_end(std::move(first1), std::move(last1),
                         std::move(first2), std::move(last2),
                         std::move(pred));
}

template <typename Range1, typename Range2, typename BinaryPredicate = std::equal_to<>,
        REQUIRES(ForwardRange<Range1> &&
                 ForwardRange<Range2> &&
                 IndirectRelation<BinaryPredicate, iterator_t<Range1>, iterator_t<Range2>>)>
safe_iterator_t<Range1>
find_end(Range1&& range1, Range2&& range2, BinaryPredicate pred = {})
{
    return std::find_end(detail::adl_begin(range1), detail::adl_end(range1),
                         detail::adl_begin(range2), detail::adl_end(range2),
                         std::move(pred));
}

template <typename Iter1, typename Iter2, typename Pred = std::equal_to<>,
        REQUIRES(InputIterator<Iter1> &&
                 ForwardIterator<Iter2> &&
                 IndirectRelation<Pred, Iter1, Iter2>)>
Iter1 find_first_of(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2, Pred pred = {})
{
    return std::find_first_of(std::move(first1), std::move(last1),
                              std::move(first2), std::move(last2),
                              std::move(pred));
}

template <typename Range1, typename Range2, typename Pred = std::equal_to<>,
        REQUIRES(InputRange<Range1> &&
                         InputRange<Range2> &&
                 IndirectRelation<Pred, iterator_t<Range1>, iterator_t<Range2>>)>
safe_iterator_t<Range1>
find_first_of(Range1&& range1, Range2&& range2, Pred pred = {})
{
    return std::find_first_of(detail::adl_begin(range1), detail::adl_end(range1),
                              detail::adl_begin(range2), detail::adl_end(range2),
                              std::move(pred));
}

template <typename Iter, typename Pred = std::equal_to<>,
        REQUIRES(ForwardIterator<Iter> &&
                 IndirectRelation<Pred, Iter, Iter>)>
Iter adjacent_find(Iter first, Iter last, Pred pred = {})
{
    return std::adjacent_find(std::move(first), std::move(last), std::forward<Pred>(pred));
}

template <typename Range, typename Pred = std::equal_to<>,
        REQUIRES(ForwardRange<Range> &&
                 IndirectRelation<Pred, iterator_t<Range>, iterator_t<Range>>)>
safe_iterator_t<Range>
adjacent_find(Range&& range, Pred pred = {})
{
    return std::adjacent_find(detail::adl_begin(range), detail::adl_end(range),
                              std::move(pred));
};

template <typename Iter1, typename Iter2, typename Pred = std::equal_to<>,
        REQUIRES(ForwardIterator<Iter1> &&
                 ForwardIterator<Iter2> &&
                 IndirectlyComparable<Iter1, Iter2, Pred>)>
Iter1 search(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2, Pred pred = {})
{
    return std::search(std::move(first1), std::move(last1),
                       std::move(first2), std::move(last2), std::move(pred));
}

template <typename Range1, typename Range2, typename Pred = std::equal_to<>,
        REQUIRES(ForwardRange<Range1> &&
                 ForwardRange<Range2> &&
                 IndirectlyComparable<iterator_t<Range1>, iterator_t<Range2>, Pred>)>
safe_iterator_t<Range1>
search(Range1&& range1, Range2&& range2, Pred pred = {})
{
    return std::search(detail::adl_begin(range1), detail::adl_end(range1),
                       detail::adl_begin(range2), detail::adl_end(range2),
                       std::move(pred));
}

template <typename Iter, typename T, typename Pred = std::equal_to<>,
        REQUIRES(ForwardIterator<Iter> &&
                 IndirectlyComparable<Iter, const T*, Pred>)>
Iter search_n(Iter first, Iter last, difference_type_t<Iter> count, const T& value, Pred pred = {})
{
    return std::search_n(std::move(first), std::move(last), count, value, std::move(pred));
}

template <typename Range, typename T, typename Pred = std::equal_to<>,
        REQUIRES(ForwardRange<Range> &&
                 IndirectlyComparable<iterator_t<Range>, const T*, Pred>)>
safe_iterator_t<Range>
search_n(Range&& range, difference_type_t<iterator_t<Range>> count, const T& value, Pred pred = {})
{
    return std::search_n(detail::adl_begin(range), detail::adl_end(range), count, value, std::move(pred));
}


/*
 * Modifying sequence algorithms
 */

template <typename Iter1, typename Iter2,
          REQUIRES(InputIterator<Iter1> &&
                   OutputIterator<Iter2, value_type_t<Iter1>>)>
Iter2 copy(Iter1 first, Iter1 last, Iter2 ofirst)
{
    return std::copy(std::move(first), std::move(last), std::move(ofirst));
}

template <typename Range1, typename Iter2,
          REQUIRES(InputRange<Range1> &&
                   OutputIterator<Iter2, range_value_type_t<Range1>>)>
Iter2 copy(Range1&& range, Iter2 ofirst)
{
    return std::copy(detail::adl_begin(range), detail::adl_end(range), std::move(ofirst));
}

template <typename Iter1, typename Iter2, typename Pred,
          REQUIRES(InputIterator<Iter1> &&
                   OutputIterator<Iter2, value_type_t<Iter1>> &&
                   IndirectUnaryPredicate<Pred, Iter1>)>
Iter2 copy_if(Iter1 first, Iter1 last, Iter2 ofirst, Pred pred)
{
    return std::copy_if(std::move(first), std::move(last), std::move(ofirst), std::move(pred));
}

template <typename Range1, typename Iter2, typename Pred,
          REQUIRES(InputRange<Range1> &&
                   OutputIterator<Iter2, range_value_type_t<Range1>> &&
                   IndirectUnaryPredicate<Pred, iterator_t<Range1>>)>
Iter2 copy_if(Range1&& range, Iter2 ofirst, Pred&& pred)
{
    return std::copy_if(detail::adl_begin(range), detail::adl_end(range), std::move(ofirst),
                        std::move(pred));
}

template <typename Iter1, typename Iter2,
          REQUIRES(InputIterator<Iter1> &&
                   WeaklyIncrementable<Iter2> &&
                   IndirectlyCopyable<Iter1, Iter2>)>
Iter2 copy_n(Iter1 first, difference_type_t<Iter1> count, Iter2 ofirst)
{
    return std::copy_n(std::move(first), count, std::move(ofirst));
}

template <typename Iter1, typename Iter2,
          REQUIRES(BidirectionalIterator<Iter1> &&
                   BidirectionalIterator<Iter2> &&
                   IndirectlyCopyable<Iter1, Iter2>)>
Iter2 copy_backward(Iter1 first, Iter1 last, Iter2 olast)
{
    return std::copy_backward(std::move(first), std::move(last), std::move(olast));
}

template <typename Range1, typename Iter2,
          REQUIRES(BidirectionalRange<Range1> &&
                   BidirectionalIterator<Iter2> &&
                   IndirectlyCopyable<iterator_t<Range1>, Iter2>)>
Iter2 copy_backward(Range1&& range, Iter2 olast)
{
    return std::copy_backward(detail::adl_begin(range), detail::adl_end(range), std::move(olast));
}

template <typename Iter1, typename Iter2,
          REQUIRES(InputIterator<Iter1> &&
                   WeaklyIncrementable<Iter2> &&
                   IndirectlyMovable<Iter1, Iter2>)>
Iter2 move(Iter1 first, Iter1 last, Iter2 ofirst)
{
    return std::move(std::move(first), std::move(last), std::move(ofirst));
}

template <typename Range1, typename Iter2,
           REQUIRES(InputRange<Range1> &&
                    WeaklyIncrementable<Iter2> &&
                    IndirectlyMovable<iterator_t<Range1>, Iter2>)>
Iter2 move(Range1&& range, Iter2 ofirst)
{
    return std::move(detail::adl_begin(range), detail::adl_end(range), std::move(ofirst));
}

template <typename Iter1, typename Iter2,
          REQUIRES(BidirectionalIterator<Iter1> &&
                   BidirectionalIterator<Iter2> &&
                   IndirectlyMovable<Iter1, Iter2>)>
Iter2 move_backward(Iter1 first, Iter1 last, Iter2 olast)
{
    return std::move_backward(std::move(first), std::move(last), std::move(olast));
}

template <typename Range1, typename Iter2,
          REQUIRES(BidirectionalRange<Range1> &&
                   BidirectionalIterator<Iter2> &&
                   IndirectlyMovable<iterator_t<Range1>, Iter2>)>
Iter2 move_backward(Range1&& range, Iter2 olast)
{
    return std::move_backward(detail::adl_begin(range), detail::adl_end(range), std::move(olast));
}

// N.B. TS says Iter only needs to be OutputIterator, but
// cppreference says std::fill() requires ForwardIterator
template <typename Iter, typename T,
          REQUIRES(ForwardIterator<Iter> &&
                   Writable<Iter, const T&>)>
void fill(Iter first, Iter last, const T& value)
{
    std::fill(std::move(first), std::move(last), value);
}

template <typename Range, typename T,
          REQUIRES(ForwardRange<Range> &&
                   Writable<iterator_t<Range>, const T&>)>
void fill(Range&& range, const T& value)
{
    std::fill(detail::adl_begin(range), detail::adl_end(range), value);
}

template <typename Iter, typename T,
          REQUIRES(OutputIterator<Iter, const T&>)>
Iter fill_n(Iter first, difference_type_t<Iter> count, const T& value)
{
    return std::fill_n(std::move(first), count, value);
}

template <typename Iter1, typename Iter2, typename UnaryOp,
          REQUIRES(InputIterator<Iter1> &&
                   WeaklyIncrementable<Iter2> &&
                   CopyConstructible<UnaryOp> &&
                   Writable<Iter2, indirect_result_of_t<UnaryOp&(Iter1)>>)>
Iter2 transform(Iter1 first, Iter1 last, Iter2 ofirst, UnaryOp op)
{
    return std::transform(std::move(first), std::move(last), std::move(ofirst), std::move(op));
}

template <typename Range1, typename Iter2, typename UnaryOp,
          REQUIRES(InputRange<Range1> &&
                   WeaklyIncrementable<Iter2> &&
                   CopyConstructible<UnaryOp> &&
                   Writable<Iter2, indirect_result_of_t<UnaryOp&(iterator_t<Range1>)>>)>
Iter2 transform(Range1&& range, Iter2 ofirst, UnaryOp op)
{
    return std::transform(detail::adl_begin(range), detail::adl_end(range),
                          std::move(ofirst), std::move(op));
}

template <typename Iter1, typename Iter2, typename Iter3, typename BinOp,
          REQUIRES(InputIterator<Iter1> &&
                   InputIterator<Iter2> &&
                   WeaklyIncrementable<Iter3> &&
                   CopyConstructible<BinOp> &&
                   Writable<Iter3, indirect_result_of_t<BinOp&(Iter1, Iter2)>>)>
NANORANGE_DEPRECATED
Iter3 transform(Iter1 first1, Iter1 last1, Iter2 first2, Iter3 ofirst, BinOp op)
{
    return std::transform(std::move(first1), std::move(last1),
                          std::move(first2), std::move(ofirst), std::move(op));
}

// Just for the hell of it, let's do a 6-parameter overload too
template <typename Iter1, typename Iter2, typename Iter3, typename BinOp,
        REQUIRES(InputIterator<Iter1> &&
                 InputIterator<Iter2> &&
                 WeaklyIncrementable<Iter3> &&
                 CopyConstructible<BinOp> &&
                 Writable<Iter3, indirect_result_of_t<BinOp&(Iter1, Iter2)>>)>
Iter3 transform(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2, Iter3 ofirst, BinOp op)
{
    while (first1 != last1 && first2 != last2) {
        *ofirst = op(*first1, *first2);
        ++ofirst; ++first1; ++first2;
    }
    return std::move(ofirst);
}

template <typename Range1, typename Range2, typename Iter3, typename BinOp,
        REQUIRES(InputRange<Range1> &&
                 InputRange<Range2> &&
                 WeaklyIncrementable<Iter3> &&
                 CopyConstructible<BinOp> &&
                 Writable<Iter3, indirect_result_of_t<BinOp&(iterator_t<Range1>, iterator_t<Range2>)>>)>
Iter3 transform(Range1&& range1, Range2&& range2, Iter3 ofirst, BinOp op)
{
    return nanorange::transform(detail::adl_begin(range1), detail::adl_end(range1),
                                  detail::adl_begin(range2), detail::adl_end(range2),
                                  std::move(ofirst), std::move(op));
}

// N.B Ranges TS only requires Iterator, std::generate() needs ForwardIterator
template <typename Iter, typename Generator,
          REQUIRES(ForwardIterator<Iter> &&
                   CopyConstructible<Generator> &&
                   Callable<Generator&> &&
                   Writable<Iter, std::result_of_t<Generator&()>>)>
void generate(Iter first, Iter last, Generator gen)
{
    std::generate(std::move(first), std::move(last), std::move(gen));
}

template <typename Range, typename Generator,
          REQUIRES(ForwardRange<Range> &&
                   CopyConstructible<Generator> &&
                   Callable<Generator&> &&
                   Writable<iterator_t<Range>, std::result_of_t<Generator&()>>)>
void generate(Range&& range, Generator gen)
{
    std::generate(detail::adl_begin(range), detail::adl_end(range), std::move(gen));
}

template <typename Iter, typename Generator,
          REQUIRES(ForwardIterator<Iter> &&
                   CopyConstructible<Generator> &&
                   Callable<Generator&> &&
                   Writable<Iter, std::result_of_t<Generator&()>>)>
Iter generate_n(Iter first, difference_type_t<Iter> count, Generator generator)
{
    return std::generate_n(std::move(first), count, std::move(generator));
}

template <typename Iter, typename T,
          REQUIRES(ForwardIterator<Iter> &&
                   Permutable<Iter> &&
                   IndirectRelation<std::equal_to<>, Iter, const T*>)>
Iter remove(Iter first, Iter last, const T& value)
{
    return std::remove(std::move(first), std::move(last), value);
}

template <typename Range, typename T,
          REQUIRES(ForwardRange<Range> &&
                   Permutable<iterator_t<Range>> &&
                   IndirectRelation<std::equal_to<>, iterator_t<Range>, const T*>)>
safe_iterator_t<Range> remove(Range&& range, const T& value)
{
    return std::remove(detail::adl_begin(range), detail::adl_end(range), value);
}

template <typename Iter, typename Pred,
          REQUIRES(ForwardIterator<Iter> &&
                   IndirectUnaryPredicate<Pred, Iter> &&
                   Permutable<Iter>)>
Iter remove_if(Iter first, Iter last, Pred pred)
{
    return std::remove_if(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename Pred,
          REQUIRES(ForwardRange<Range> &&
                   IndirectUnaryPredicate<Pred, iterator_t<Range>> &&
                   Permutable<iterator_t<Range>>)>
safe_iterator_t<Range> remove_if(Range&& range, Pred pred)
{
    return std::remove_if(detail::adl_begin(range), detail::adl_end(range), std::move(pred));
}

template <typename Iter1, typename Iter2, typename T,
          REQUIRES(InputIterator<Iter1> &&
                   WeaklyIncrementable<Iter2> &&
                   IndirectlyCopyable<Iter1, Iter2> &&
                   IndirectRelation<std::equal_to<>, Iter1, const T*>)>
Iter2 remove_copy(Iter1 first, Iter1 last, Iter2 ofirst, const T& value)
{
    return std::remove_copy(std::move(first), std::move(last),
                            std::move(ofirst), value);
}

template <typename Range1, typename Iter2, typename T,
        REQUIRES(InputRange<Range1> &&
                 WeaklyIncrementable<Iter2> &&
                 IndirectlyCopyable<iterator_t<Range1>, Iter2> &&
                 IndirectRelation<std::equal_to<>, iterator_t<Range1>, const T*>)>
Iter2 remove_copy(Range1&& range, Iter2 ofirst, const T& value)
{
    return std::remove_copy(detail::adl_begin(range), detail::adl_end(range),
                            std::move(ofirst), value);
}

template <typename Iter1, typename Iter2, typename Pred,
          REQUIRES(ForwardIterator<Iter1> &&
                   WeaklyIncrementable<Iter2> &&
                   IndirectUnaryPredicate<Pred, Iter1> &&
                   IndirectlyCopyable<Iter1, Iter2>)>
Iter2 remove_copy_if(Iter1 first, Iter1 last, Iter2 ofirst, Pred pred)
{
    return std::remove_copy_if(std::move(first), std::move(last),
                               std::move(ofirst), std::move(pred));
}

template <typename Range1, typename Iter2, typename Pred,
          REQUIRES(InputRange<Range1> &&
                   WeaklyIncrementable<Iter2> &&
                   IndirectUnaryPredicate<Pred, iterator_t<Range1>> &&
                   IndirectlyCopyable<iterator_t<Range1>, Iter2>)>
Iter2 remove_copy_if(Range1&& range, Iter2 ofirst, Pred pred)
{
    return std::remove_copy_if(detail::adl_begin(range), detail::adl_end(range),
                               std::move(ofirst), std::move(pred));
}

template <typename Iter, typename T,
          REQUIRES(ForwardIterator<Iter> &&
                   Writable<Iter, const T&> &&
                   IndirectRelation<std::equal_to<>, Iter, const T*>)>
void replace(Iter first, Iter last, const T& old_value, const T& new_value)
{
    std::replace(std::move(first), std::move(last), old_value, new_value);
}

template <typename Range, typename T,
          REQUIRES(ForwardRange<Range> &&
                   Writable<iterator_t<Range>, const T&> &&
                           IndirectRelation<std::equal_to<>, iterator_t<Range>, const T*>)>
void replace(Range&& range, const T& old_value, const T& new_value)
{
    std::replace(detail::adl_begin(range), detail::adl_end(range),
                 old_value, new_value);
}

template <typename Iter, typename Pred, typename T,
          REQUIRES(ForwardIterator<Iter> &&
                   Writable<Iter, const T&> &&
                   IndirectUnaryPredicate<Pred, Iter>)>
void replace_if(Iter first, Iter last, Pred pred, const T& new_value)
{
    std::replace_if(std::move(first), std::move(last), std::move(pred), new_value);
}

template <typename Range, typename Pred, typename T,
          REQUIRES(ForwardRange<Range> &&
                   Writable<iterator_t<Range>, const T&> &&
                   IndirectUnaryPredicate<Pred, iterator_t<Range>>)>
void replace_if(Range&& range, Pred pred, const T& new_value)
{
    std::replace_if(detail::adl_begin(range), detail::adl_end(range),
                    std::move(pred), new_value);
}

template <typename Iter1, typename Iter2, typename T,
          REQUIRES(InputIterator<Iter1> &&
                   OutputIterator<Iter2, const T&>)>
Iter2 replace_copy(Iter1 first, Iter1 last, Iter2 ofirst, const T& old_value, const T& new_value)
{
    return std::replace_copy(std::move(first), std::move(last), std::move(ofirst),
                             old_value, new_value);
}

template <typename Range1, typename Iter2, typename T,
          REQUIRES(InputRange<Range1> &&
                   OutputIterator<Iter2, const T&>)>
Iter2 replace_copy(Range1&& range, Iter2 ofirst, const T& old_value, const T& new_value)
{
    return std::replace_copy(detail::adl_begin(range), detail::adl_end(range),
                             std::move(ofirst), old_value, new_value);
}

template <typename Iter1, typename Iter2, typename Pred, typename T,
          REQUIRES(InputIterator<Iter1> &&
                   OutputIterator<Iter2, const T&> &&
                   IndirectUnaryPredicate<Pred, Iter1>)>
Iter2 replace_copy_if(Iter1 first, Iter1 last, Iter2 ofirst, Pred pred, const T& new_value)
{
    return std::replace_copy_if(std::move(first), std::move(last), std::move(ofirst),
                                std::move(pred), new_value);
}

template <typename Range1, typename Iter2, typename Pred, typename T,
          REQUIRES(InputRange<Range1> &&
                   OutputIterator<Iter2, const T&> &&
                   IndirectUnaryPredicate<Pred, iterator_t<Range1>>)>
Iter2 replace_copy_if(Range1&& range, Iter2 ofirst, Pred pred, const T& new_value)
{
    return std::replace_copy_if(detail::adl_begin(range), detail::adl_end(range),
                                std::move(ofirst), std::move(pred), new_value);
}

template <typename Iter1, typename Iter2,
          REQUIRES(ForwardIterator<Iter1> &&
                   ForwardIterator<Iter2>)>
NANORANGE_DEPRECATED
Iter2 swap_ranges(Iter1 first1, Iter1 last1, Iter2 first2)
{
    return std::swap_ranges(std::move(first1), std::move(last1), std::move(first2));
}

template <typename Iter1, typename Iter2,
        REQUIRES(ForwardIterator<Iter1> &&
                 ForwardIterator<Iter2>)>
std::pair<Iter1, Iter2>
swap_ranges(Iter1 first1, Iter1 last1, Iter2 first2, Iter2 last2)
{
    while (first1 != last1 && first2 != last2) {
        std::iter_swap(first1, first2);
        ++first1; ++first2;
    }
    return std::make_pair(std::move(first1), std::move(first2));
}

template <typename Range1, typename Range2,
          REQUIRES(ForwardRange<Range1> &&
                   ForwardRange<Range2>)>
std::pair<safe_iterator_t<Range1>, safe_iterator_t<Range2>>
swap_ranges(Range1&& range1, Range2&& range2)
{
    return nanorange::swap_ranges(detail::adl_begin(range1), detail::adl_end(range1),
                                  detail::adl_begin(range2), detail::adl_end(range2));
}

template <typename Iter,
          REQUIRES(BidirectionalIterator<Iter>)>
void reverse(Iter first, Iter last)
{
    std::reverse(std::move(first), std::move(last));
}

template <typename Range,
          REQUIRES(BidirectionalRange<Range>)>
void reverse(Range&& range)
{
    std::reverse(detail::adl_begin(range), detail::adl_end(range));
}

template <typename Iter1, typename Iter2,
          REQUIRES(BidirectionalIterator<Iter1> &&
                   OutputIterator<Iter2, value_type_t<Iter1>>)>
Iter2 reverse_copy(Iter1 first, Iter1 last, Iter2 ofirst)
{
    return std::reverse_copy(std::move(first), std::move(last), std::move(ofirst));
}

template <typename Range1, typename Iter2,
          REQUIRES(BidirectionalRange<Range1> &&
                   OutputIterator<Iter2, range_value_type_t<Range1>>)>
Iter2 reverse_copy(Range1&& range, Iter2 ofirst)
{
    return std::reverse_copy(detail::adl_begin(range), detail::adl_end(range),
                             std::move(ofirst));
}

template <typename Iter,
          REQUIRES(ForwardIterator<Iter>)>
void rotate(Iter first, Iter middle, Iter last)
{
    std::rotate(std::move(first), std::move(middle), std::move(last));
}

template <typename Range,
          REQUIRES(ForwardRange<Range>)>
void rotate(Range&& range, iterator_t<Range> middle)
{
    std::rotate(detail::adl_begin(range), std::move(middle), detail::adl_end(range));
}

template <typename Iter1, typename Iter2,
          REQUIRES(ForwardIterator<Iter1> &&
                   OutputIterator<Iter2, value_type_t<Iter1>>)>
Iter2 rotate_copy(Iter1 first, Iter1 middle, Iter1 last, Iter2 ofirst)
{
    return std::rotate_copy(std::move(first), std::move(middle),
                            std::move(last), std::move(ofirst));
}

template <typename Range1, typename Iter2,
        REQUIRES(ForwardRange<Range1> &&
                 OutputIterator<Iter2, range_value_type_t<Range1>>)>
Iter2 rotate_copy(Range1&& range, iterator_t<Range1> middle, Iter2 ofirst)
{
    return std::rotate_copy(detail::adl_begin(range), std::move(middle),
                            detail::adl_end(range), std::move(ofirst));
}

template <typename Iter, typename URBG,
          REQUIRES(RandomAccessIterator<Iter>)>
void shuffle(Iter first, Iter last, URBG&& generator)
{
    std::shuffle(std::move(first), std::move(last), std::forward<URBG>(generator));
}

template <typename Range, typename URBG,
          REQUIRES(RandomAccessRange<Range>)>
void shuffle(Range&& range, URBG&& generator)
{
    std::shuffle(detail::adl_begin(range), detail::adl_end(range), std::forward<URBG>(generator));
}

template <typename Iter, typename Pred = std::equal_to<>,
          REQUIRES(ForwardIterator<Iter> &&
                   IndirectRelation<Pred, Iter, Iter>)>
Iter unique(Iter first, Iter last, Pred pred = {})
{
    return std::unique(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename Pred = std::equal_to<>,
          REQUIRES(ForwardRange<Range> &&
                   IndirectRelation<Pred, iterator_t<Range>, iterator_t<Range>>)>
safe_iterator_t<Range> unique(Range&& range, Pred pred = {})
{
    return std::unique(detail::adl_begin(range), detail::adl_end(range), std::move(pred));
}

template <typename Iter1, typename Iter2, typename Pred = std::equal_to<>,
        REQUIRES(InputIterator<Iter1> &&
                 OutputIterator<Iter2, value_type_t<Iter1>> &&
                 IndirectRelation<Pred, Iter1, Iter1>)>
Iter2 unique_copy(Iter1 first, Iter1 last, Iter2 ofirst, Pred pred = {})
{
    return std::unique_copy(std::move(first), std::move(last), std::move(ofirst), std::move(pred));
}

template <typename Range1, typename Iter2, typename Pred = std::equal_to<>,
        REQUIRES(InputRange<Range1> &&
                 OutputIterator<Iter2, range_value_type_t<Range1>> &&
                 IndirectRelation<Pred, iterator_t<Range1>, iterator_t<Range1>>)>
Iter2 unique_copy(Range1&& range, Iter2 ofirst, Pred pred = {})
{
    return std::unique_copy(detail::adl_begin(range), detail::adl_end(range), std::move(ofirst), std::move(pred));
}

/*
 * Partitioning Operations
 */

template <typename Iter, typename Pred,
          REQUIRES(InputIterator<Iter> &&
                   IndirectUnaryPredicate<Pred, Iter>)>
bool is_partitioned(Iter first, Iter last, Pred pred)
{
    return std::is_partitioned(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename Pred,
          REQUIRES(InputRange<Range> &&
                   IndirectUnaryPredicate<Pred, iterator_t<Range>>)>
bool is_partitioned(Range&& range, Pred pred)
{
    return std::is_partitioned(detail::adl_begin(range), detail::adl_end(range), std::move(pred));
}

template <typename Iter, typename Pred,
          REQUIRES(ForwardIterator<Iter> &&
                   IndirectUnaryPredicate<Pred, Iter>)>
Iter partition(Iter first, Iter last, Pred pred)
{
    return std::partition(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename Pred,
          REQUIRES(ForwardRange<Range> &&
                   IndirectUnaryPredicate<Pred, iterator_t<Range>>)>
safe_iterator_t<Range> partition(Range&& range, Pred pred)
{
    return std::partition(detail::adl_begin(range), detail::adl_end(range), std::move(pred));
}


template <typename Iter1, typename Iter2, typename Iter3, typename Pred,
          REQUIRES(InputIterator<Iter1> &&
                   OutputIterator<Iter2, value_type_t<Iter1>> &&
                   OutputIterator<Iter3, value_type_t<Iter1>> &&
                   IndirectUnaryPredicate<Pred, Iter1>)>
std::pair<Iter2, Iter3>
partition_copy(Iter1 first, Iter1 last, Iter2 otrue, Iter3 ofalse, Pred pred)
{
    return std::partition_copy(std::move(first), std::move(last),
                               std::move(otrue), std::move(ofalse), std::move(pred));
}

template <typename Range1, typename Iter2, typename Iter3, typename Pred,
        REQUIRES(InputRange<Range1> &&
                         OutputIterator<Iter2, range_value_type_t<Range1>> &&
                         OutputIterator<Iter3, range_value_type_t<Range1>> &&
                         IndirectUnaryPredicate<Pred, iterator_t<Range1>>)>
std::pair<Iter2, Iter3>
partition_copy(Range1&& range, Iter2 otrue, Iter3 ofalse, Pred pred)
{
    return std::partition_copy(detail::adl_begin(range), detail::adl_end(range),
                               std::move(otrue), std::move(ofalse), std::move(pred));
}


template <typename Iter, typename Pred,
          REQUIRES(BidirectionalIterator<Iter> &&
                   IndirectUnaryPredicate<Pred, Iter>)>
Iter stable_partition(Iter first, Iter last, Pred pred)
{
    return std::stable_partition(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename Pred,
        REQUIRES(BidirectionalRange<Range> &&
                 IndirectUnaryPredicate<Pred, iterator_t<Range>>)>
safe_iterator_t<Range> stable_partition(Range&& range, Pred pred)
{
    return std::stable_partition(detail::adl_begin(range), detail::adl_end(range), std::move(pred));
}

template <typename Iter, typename Pred,
          REQUIRES(ForwardIterator<Iter> &&
                   IndirectUnaryPredicate<Pred, Iter>)>
Iter partition_point(Iter first, Iter last, Pred pred)
{
    return std::partition_point(std::move(first), std::move(last), std::move(pred));
}

template <typename Range, typename Pred,
        REQUIRES(ForwardRange<Range> &&
                 IndirectUnaryPredicate<Pred, iterator_t<Range>>)>
safe_iterator_t<Range> partition_point(Range&& range, Pred pred)
{
    return std::partition_point(detail::adl_begin(range), detail::adl_end(range), std::move(pred));
}

/*
 * Sorting operations
 */

template <typename Iter, typename Comp = std::less<>,
        REQUIRES(ForwardIterator<Iter> &&
                 IndirectStrictWeakOrder<Comp, Iter>)>
bool is_sorted(Iter first, Iter last, Comp comp = {})
{
    return std::is_sorted(std::move(first), std::move(last), std::move(comp));
}

template <typename Range, typename Comp = std::less<>,
        REQUIRES(ForwardRange<Range> &&
                 IndirectStrictWeakOrder<Comp, iterator_t<Range>>)>
bool is_sorted(Range&& range, Comp comp = {})
{
    return std::is_sorted(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename Iter, typename Comp = std::less<>,
        REQUIRES(ForwardIterator<Iter> &&
                 IndirectStrictWeakOrder<Comp, Iter>)>
Iter is_sorted_until(Iter first, Iter last, Comp comp = {})
{
    return std::is_sorted_until(std::move(first), std::move(last), std::move(comp));
}

template <typename Range, typename Comp = std::less<>,
        REQUIRES(ForwardRange<Range> &&
                 IndirectStrictWeakOrder<Comp, iterator_t<Range>>)>
safe_iterator_t<Range> is_sorted_until(Range&& range, Comp comp = {})
{
    return std::is_sorted_until(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename Iter, typename Comp = std::less<>,
        REQUIRES(RandomAccessIterator<Iter> &&
                 Sortable<Iter, Comp>)>
void sort(Iter first, Iter last, Comp comp = {})
{
    std::sort(std::move(first), std::move(last), std::move(comp));
}

template <typename Range, typename Comp = std::less<>,
        REQUIRES(RandomAccessRange<Range> &&
                 Sortable<iterator_t<Range>, Comp>)>
void sort(Range&& range, Comp comp = {})
{
    std::sort(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename Iter, typename Comp = std::less<>,
        REQUIRES(RandomAccessIterator<Iter> &&
                 Sortable<Iter, Comp>)>
void partial_sort(Iter first, Iter middle, Iter last, Comp comp = {})
{
    std::partial_sort(std::move(first), std::move(middle), std::move(last), std::move(comp));
}

template <typename Range, typename Comp = std::less<>,
        REQUIRES(RandomAccessRange<Range> &&
                 Sortable<iterator_t<Range>, Comp>)>
void partial_sort(Range&& range, iterator_t<Range> middle, Comp comp = {})
{
    std::partial_sort(detail::adl_begin(range), std::move(middle), detail::adl_end(range), std::move(comp));
}

template <typename Iter1, typename Iter2, typename Comp = std::less<>,
        REQUIRES(InputIterator<Iter1> &&
                 RandomAccessIterator<Iter2> &&
                 IndirectlyCopyable<Iter1, Iter2> &&
                 Sortable<Iter2, Comp> &&
                 IndirectStrictWeakOrder<Comp, Iter1, Iter2>)>
Iter2 partial_sort_copy(Iter1 first, Iter1 last, Iter2 rfirst, Iter2 rlast, Comp comp = {})
{
    return std::partial_sort(std::move(first), std::move(last), std::move(rfirst), std::move(rlast), std::move(comp));
}

template <typename Range1, typename Range2, typename Comp = std::less<>,
        REQUIRES(InputRange<Range1> &&
                 RandomAccessRange<Range2> &&
                 IndirectlyCopyable<iterator_t<Range1>, iterator_t<Range2>> &&
                 Sortable<iterator_t<Range2>, Comp> &&
                 IndirectStrictWeakOrder<Comp, iterator_t<Range1>, iterator_t<Range2>>)>
safe_iterator_t<Range2> partial_sort_copy(Range1&& input, Range2&& result, Comp comp = {})
{
    return std::partial_sort_copy(detail::adl_begin(input), detail::adl_end(input),
                                  detail::adl_begin(result), detail::adl_end(result),
                                  std::move(comp));
}

template <typename Iter, typename Comp = std::less<>,
        REQUIRES(RandomAccessIterator<Iter> &&
                 Sortable<Iter, Comp>)>
void stable_sort(Iter first, Iter last, Comp comp = {})
{
    std::stable_sort(std::move(first), std::move(last), std::move(comp));
}

template <typename Range, typename Comp = std::less<>,
        REQUIRES(RandomAccessRange<Range> &&
                 Sortable<iterator_t<Range>, Comp>)>
void stable_sort(Range&& range, Comp comp = {})
{
    std::stable_sort(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename Iter, typename Comp = std::less<>,
        REQUIRES(RandomAccessIterator<Iter> &&
                 Sortable<Iter, Comp>)>
void nth_element(Iter first, Iter nth, Iter last, Comp comp = {})
{
    std::nth_element(std::move(first), std::move(nth), std::move(last), std::move(comp));
}

template <typename Range, typename Comp = std::less<>,
        REQUIRES(RandomAccessRange<Range> &&
                 Sortable<iterator_t<Range>, Comp>)>
void nth_element(Range&& range, iterator_t<Range> nth, Comp comp = {})
{
    std::nth_element(detail::adl_begin(range), detail::adl_end(range), std::move(nth), std::move(comp));
}

/*
 * Binary search operations
 */

template <typename Iter, typename T, typename Comp = std::less<>,
        REQUIRES(ForwardIterator<Iter> &&
                 IndirectStrictWeakOrder<Comp, const T*, Iter>)>
Iter lower_bound(Iter first, Iter last, const T& value, Comp comp = {})
{
    return std::lower_bound(std::move(first), std::move(last), value, std::move(comp));
}

template <typename Range, typename T, typename Comp = std::less<>,
        REQUIRES(ForwardRange<Range> &&
                 IndirectStrictWeakOrder<Comp, const T*, iterator_t<Range>>)>
safe_iterator_t<Range>
lower_bound(Range&& range, const T& value, Comp comp = {})
{
    return std::lower_bound(detail::adl_begin(range), detail::adl_end(range), value, std::move(comp));
}

template <typename Iter, typename T, typename Comp = std::less<>,
        REQUIRES(ForwardIterator<Iter> &&
                 IndirectStrictWeakOrder<Comp, const T*, Iter>)>
Iter upper_bound(Iter first, Iter last, const T& value, Comp comp = {})
{
    return std::upper_bound(std::move(first), std::move(last), value, std::move(comp));
}

template <typename Range, typename T, typename Comp = std::less<>,
        REQUIRES(ForwardRange<Range> &&
                 IndirectStrictWeakOrder<Comp, const T*, iterator_t<Range>>)>
safe_iterator_t<Range>
upper_bound(Range&& range, const T& value, Comp comp = {})
{
    return std::upper_bound(detail::adl_begin(range), detail::adl_end(range), value, std::move(comp));
}

template <typename Iter, typename T, typename Comp = std::less<>,
        REQUIRES(ForwardIterator<Iter> &&
                 IndirectStrictWeakOrder<Comp, const T*, Iter>)>
bool binary_search(Iter first, Iter last, const T& value, Comp comp = {})
{
    return std::binary_search(std::move(first), std::move(last), value, std::move(comp));
}

template <typename Range, typename T, typename Comp = std::less<>,
        REQUIRES(ForwardRange<Range> &&
                 IndirectStrictWeakOrder<Comp, const T*, iterator_t<Range>>)>
bool binary_search(Range&& range, const T& value, Comp comp = {})
{
    return std::binary_search(detail::adl_begin(range), detail::adl_end(range), value, std::move(comp));
}

template <typename Iter, typename T, typename Comp = std::less<>,
          REQUIRES(ForwardIterator<Iter> &&
                   IndirectStrictWeakOrder<Comp, const T*, Iter>)>
std::pair<Iter, Iter>
equal_range(Iter first, Iter last, const T& value, Comp comp = {})
{
    return std::equal_range(std::move(first), std::move(last), value, std::move(comp));
}

template <typename Range, typename T, typename Comp = std::less<>,
          REQUIRES(ForwardRange<Range> &&
                   IndirectStrictWeakOrder<Comp, const T*, iterator_t<Range>>)>
std::pair<safe_iterator_t<Range>, safe_iterator_t<Range>>
equal_range(Range&& range, const T& value, Comp comp = {})
{
    return std::equal_range(detail::adl_begin(range), detail::adl_end(range), value, std::move(comp));
}

/*
 * Set operations
 */

template <typename InputIt1, typename InputIt2, typename OutputIt, typename Comp = std::less<>,
        REQUIRES(InputIterator<InputIt1> &&
                 InputIterator<InputIt2> &&
                 WeaklyIncrementable<OutputIt> &&
                 Mergeable<InputIt1, InputIt2, OutputIt, Comp>)>
OutputIt merge(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2,  OutputIt ofirst, Comp comp = {})
{
    return std::merge(std::move(first1), std::move(last1),
                      std::move(first2), std::move(last2),
                      std::move(ofirst), std::move(comp));
}

template <typename InputRng1, typename InputRng2, typename OutputIt, typename Comp = std::less<>,
        REQUIRES(InputRange<InputRng1> &&
                 InputRange<InputRng2> &&
                 WeaklyIncrementable<OutputIt> &&
                 Mergeable<iterator_t<InputRng1>, iterator_t<InputRng2>, OutputIt, Comp>)>
OutputIt merge(InputRng1&& range1, InputRng2&& range2, OutputIt ofirst, Comp comp = {})
{
    return std::merge(detail::adl_begin(range1), detail::adl_end(range1),
                      detail::adl_begin(range2), detail::adl_end(range2),
                      std::move(ofirst), std::move(comp));
}

template <typename BidirIt, typename Comp = std::less<>,
        REQUIRES(BidirectionalIterator<BidirIt> &&
                 Sortable<BidirIt, Comp>)>
void inplace_merge(BidirIt first, BidirIt middle, BidirIt last, Comp comp = {})
{
    std::inplace_merge(std::move(first), std::move(middle), std::move(last), std::move(comp));
}

template <typename BidirRng, typename Comp = std::less<>,
        REQUIRES(BidirectionalRange<BidirRng> &&
                 Sortable<iterator_t<BidirRng>, Comp>)>
void inplace_merge(BidirRng&& range, iterator_t<BidirRng> middle, Comp comp = {})
{
    std::inplace_merge(detail::adl_begin(range), std::move(middle), detail::adl_end(range), std::move(comp));
}

template <typename InputIt1, typename InputIt2, typename Comp = std::less<>,
        REQUIRES(InputIterator<InputIt1> &&
                 InputIterator<InputIt2> &&
                 IndirectStrictWeakOrder<Comp, InputIt1, InputIt2>)>
bool includes(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2, Comp comp = {})
{
    return std::includes(std::move(first1), std::move(last1),
                         std::move(first2), std::move(last2),
                         std::move(comp));
}

template <typename InputRng1, typename InputRng2, typename Comp = std::less<>,
        REQUIRES(InputRange<InputRng1> &&
                 InputRange<InputRng2> &&
                 IndirectStrictWeakOrder<Comp, iterator_t<InputRng1>, iterator_t<InputRng2>>)>
bool includes(InputRng1&& range1, InputRng2&& range2, Comp comp = {})
{
    return std::includes(detail::adl_begin(range1), detail::adl_end(range1),
                         detail::adl_begin(range2), detail::adl_end(range2),
                         std::move(comp));
}

template <typename InputIt1, typename InputIt2, typename OutputIt, typename Comp = std::less<>,
          REQUIRES(InputIterator<InputIt1> &&
                   InputIterator<InputIt2> &&
                   WeaklyIncrementable<OutputIt> &&
                   Mergeable<InputIt1, InputIt2, OutputIt, Comp>)>
OutputIt set_difference(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2,
                        OutputIt ofirst, Comp comp = {})
{
    return std::set_difference(std::move(first1), std::move(last1),
                               std::move(first2), std::move(last2),
                               std::move(ofirst), std::move(comp));
}

template <typename InputRng1, typename InputRng2, typename OutputIt, typename Comp = std::less<>,
        REQUIRES(InputRange<InputRng1> &&
                         InputRange<InputRng2> &&
                         WeaklyIncrementable<OutputIt> &&
                         Mergeable<iterator_t<InputRng1>, iterator_t<InputRng2>, OutputIt, Comp>)>
OutputIt set_difference(InputRng1&& range1, InputRng2&& range2, OutputIt ofirst, Comp comp = {})
{
    return std::set_difference(detail::adl_begin(range1), detail::adl_end(range1),
                               detail::adl_begin(range2), detail::adl_end(range2),
                               std::move(ofirst), std::move(comp));
}

template <typename InputIt1, typename InputIt2, typename OutputIt, typename Comp = std::less<>,
        REQUIRES(InputIterator<InputIt1> &&
                         InputIterator<InputIt2> &&
                         WeaklyIncrementable<OutputIt> &&
                         Mergeable<InputIt1, InputIt2, OutputIt, Comp>)>
OutputIt set_intersection(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2,
                        OutputIt ofirst, Comp comp = {})
{
    return std::set_intersection(std::move(first1), std::move(last1),
                                 std::move(first2), std::move(last2),
                                 std::move(ofirst), std::move(comp));
}

template <typename InputRng1, typename InputRng2, typename OutputIt, typename Comp = std::less<>,
        REQUIRES(InputRange<InputRng1> &&
                         InputRange<InputRng2> &&
                         WeaklyIncrementable<OutputIt> &&
                         Mergeable<iterator_t<InputRng1>, iterator_t<InputRng2>, OutputIt, Comp>)>
OutputIt set_intersection(InputRng1&& range1, InputRng2&& range2, OutputIt ofirst, Comp comp = {})
{
    return std::set_intersection(detail::adl_begin(range1), detail::adl_end(range1),
                                 detail::adl_begin(range2), detail::adl_end(range2),
                                 std::move(ofirst), std::move(comp));
}

template <typename InputIt1, typename InputIt2, typename OutputIt, typename Comp = std::less<>,
        REQUIRES(InputIterator<InputIt1> &&
                         InputIterator<InputIt2> &&
                         WeaklyIncrementable<OutputIt> &&
                         Mergeable<InputIt1, InputIt2, OutputIt, Comp>)>
OutputIt set_symmetric_difference(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2,
                        OutputIt ofirst, Comp comp = {})
{
    return std::set_symmetric_difference(std::move(first1), std::move(last1),
                                         std::move(first2), std::move(last2),
                                         std::move(ofirst), std::move(comp));
}

template <typename InputRng1, typename InputRng2, typename OutputIt, typename Comp = std::less<>,
        REQUIRES(InputRange<InputRng1> &&
                         InputRange<InputRng2> &&
                         WeaklyIncrementable<OutputIt> &&
                         Mergeable<iterator_t<InputRng1>, iterator_t<InputRng2>, OutputIt, Comp>)>
OutputIt set_symmetric_difference(InputRng1&& range1, InputRng2&& range2, OutputIt ofirst, Comp comp = {})
{
    return std::set_symmetric_difference(detail::adl_begin(range1), detail::adl_end(range1),
                                         detail::adl_begin(range2), detail::adl_end(range2),
                                         std::move(ofirst), std::move(comp));
}

template <typename InputIt1, typename InputIt2, typename OutputIt, typename Comp = std::less<>,
        REQUIRES(InputIterator<InputIt1> &&
                         InputIterator<InputIt2> &&
                         WeaklyIncrementable<OutputIt> &&
                         Mergeable<InputIt1, InputIt2, OutputIt, Comp>)>
OutputIt set_union(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2,
                        OutputIt ofirst, Comp comp = {})
{
    return std::set_union(std::move(first1), std::move(last1),
                          std::move(first2), std::move(last2),
                          std::move(ofirst), std::move(comp));
}

template <typename InputRng1, typename InputRng2, typename OutputIt, typename Comp = std::less<>,
        REQUIRES(InputRange<InputRng1> &&
                         InputRange<InputRng2> &&
                         WeaklyIncrementable<OutputIt> &&
                         Mergeable<iterator_t<InputRng1>, iterator_t<InputRng2>, OutputIt, Comp>)>
OutputIt set_union(InputRng1&& range1, InputRng2&& range2, OutputIt ofirst, Comp comp = {})
{
    return std::set_union(detail::adl_begin(range1), detail::adl_end(range1),
                          detail::adl_begin(range2), detail::adl_end(range2),
                          std::move(ofirst), std::move(comp));
}

/*
 * Heap operations
 */

template <typename RandomIt, typename Comp = std::less<>,
          REQUIRES(RandomAccessIterator<RandomIt> &&
                   IndirectStrictWeakOrder<Comp, RandomIt>)>
bool is_heap(RandomIt first, RandomIt last, Comp comp = {})
{
    return std::is_heap(std::move(first), std::move(last), std::move(comp));
}

template <typename RandomRng, typename Comp = std::less<>,
        REQUIRES(RandomAccessRange<RandomRng> &&
                 IndirectStrictWeakOrder<Comp, iterator_t<RandomRng>>)>
bool is_heap(RandomRng&& range, Comp comp = {})
{
    return std::is_heap(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename RandomIt, typename Comp = std::less<>,
        REQUIRES(RandomAccessIterator<RandomIt> &&
                  IndirectStrictWeakOrder<Comp, RandomIt>)>
RandomIt is_heap_until(RandomIt first, RandomIt last, Comp comp = {})
{
    return std::is_heap_until(std::move(first), std::move(last), std::move(comp));
}

template <typename RandomRng, typename Comp = std::less<>,
        REQUIRES(RandomAccessRange<RandomRng> &&
                 IndirectStrictWeakOrder<Comp, iterator_t<RandomRng>>)>
safe_iterator_t<RandomRng> is_heap_until(RandomRng&& range, Comp comp = {})
{
    return std::is_heap_until(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename RandomIt, typename Comp = std::less<>,
          REQUIRES(RandomAccessIterator<RandomIt> &&
                   Sortable<RandomIt, Comp>)>
void make_heap(RandomIt first, RandomIt last, Comp comp = {})
{
    std::make_heap(std::move(first), std::move(last), std::move(comp));
}

template <typename RandomRng, typename Comp = std::less<>,
        REQUIRES(RandomAccessRange<RandomRng> &&
                 Sortable<iterator_t<RandomRng>, Comp>)>
void make_heap(RandomRng&& range, Comp comp = {})
{
    std::make_heap(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename RandomIt, typename Comp = std::less<>,
        REQUIRES(RandomAccessIterator<RandomIt> &&
                 Sortable<RandomIt, Comp>)>
void push_heap(RandomIt first, RandomIt last, Comp comp = {})
{
    std::push_heap(std::move(first), std::move(last), std::move(comp));
}

template <typename RandomRng, typename Comp = std::less<>,
        REQUIRES(RandomAccessRange<RandomRng> &&
                 Sortable<iterator_t<RandomRng>, Comp>)>
void push_heap(RandomRng&& range, Comp comp = {})
{
    std::push_heap(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename RandomIt, typename Comp = std::less<>,
        REQUIRES(RandomAccessIterator<RandomIt> &&
                 Sortable<RandomIt, Comp>)>
void pop_heap(RandomIt first, RandomIt last, Comp comp = {})
{
    std::pop_heap(std::move(first), std::move(last), std::move(comp));
}

template <typename RandomRng, typename Comp = std::less<>,
        REQUIRES(RandomAccessRange<RandomRng> &&
                 Sortable<iterator_t<RandomRng>, Comp>)>
void pop_heap(RandomRng&& range, Comp comp = {})
{
    std::pop_heap(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename RandomIt, typename Comp = std::less<>,
        REQUIRES(RandomAccessIterator<RandomIt> &&
                 Sortable<RandomIt, Comp>)>
void sort_heap(RandomIt first, RandomIt last, Comp comp = {})
{
    std::sort_heap(std::move(first), std::move(last), std::move(comp));
}

template <typename RandomRng, typename Comp = std::less<>,
        REQUIRES(RandomAccessRange<RandomRng> &&
                 Sortable<iterator_t<RandomRng>, Comp>)>
void sort_heap(RandomRng&& range, Comp comp = {})
{
    std::sort_heap(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

/*
 * Min/max operations
 */

template <typename T, typename Comp = std::less<>,
          REQUIRES(Copyable<T> &&
                   IndirectStrictWeakOrder<Comp, const T*>)>
constexpr const T& max(const T& a, const T& b, Comp comp = {})
{
    return std::max(a, b, std::move(comp));
}

template <typename T, typename Comp = std::less<>,
          REQUIRES(Copyable<T> &&
                   IndirectStrictWeakOrder<Comp, const T*>)>
constexpr T max(std::initializer_list<T> ilist, Comp comp = {})
{
    return std::max(ilist, std::move(comp));
}

template <typename ForwardRng, typename Comp = std::less<>,
          REQUIRES(ForwardRange<ForwardRng> &&
                   Copyable<value_type_t<iterator_t<ForwardRng>>> &&
                   IndirectStrictWeakOrder<Comp, iterator_t<ForwardRng>>)>
value_type_t<iterator_t<ForwardRng>>
max(ForwardRng&& range, Comp comp = {})
{
    return *std::max_element(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename ForwardIt, typename Comp = std::less<>,
          REQUIRES(ForwardIterator<ForwardIt> &&
                   IndirectStrictWeakOrder<Comp, ForwardIt>)>
ForwardIt max_element(ForwardIt first, ForwardIt last, Comp comp = {})
{
    return std::max_element(std::move(first), std::move(last), std::move(comp));
}

template <typename ForwardRng, typename Comp = std::less<>,
          REQUIRES(ForwardRange<ForwardRng> &&
                   IndirectStrictWeakOrder<Comp, iterator_t<ForwardRng>>)>
safe_iterator_t<ForwardRng>
max_element(ForwardRng&& range, Comp comp = {})
{
    return std::max_element(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename T, typename Comp = std::less<>,
        REQUIRES(Copyable<T> &&
                 IndirectStrictWeakOrder<Comp, const T*>)>
constexpr const T& min(const T& a, const T& b, Comp comp = {})
{
    return std::min(a, b, std::move(comp));
}

template <typename T, typename Comp = std::less<>,
        REQUIRES(Copyable<T> &&
                 IndirectStrictWeakOrder<Comp, const T*>)>
constexpr T min(std::initializer_list<T> ilist, Comp comp = {})
{
    return std::min(ilist, std::move(comp));
}

template <typename ForwardRng, typename Comp = std::less<>,
        REQUIRES(ForwardRange<ForwardRng> &&
                 IndirectStrictWeakOrder<Comp, iterator_t<ForwardRng>> &&
                 Copyable<value_type_t<iterator_t<ForwardRng>>>)>
value_type_t<iterator_t<ForwardRng>>
min(ForwardRng&& range, Comp comp = {})
{
    return *std::min_element(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename ForwardIt, typename Comp = std::less<>,
          REQUIRES(ForwardIterator<ForwardIt> &&
                   IndirectStrictWeakOrder<Comp, ForwardIt>)>
ForwardIt min_element(ForwardIt first, ForwardIt last, Comp comp = {})
{
    return std::min_element(std::move(first), std::move(last), std::move(comp));
}

template <typename ForwardRng, typename Comp = std::less<>,
        REQUIRES(ForwardRange<ForwardRng> &&
                 IndirectStrictWeakOrder<Comp, iterator_t<ForwardRng>>)>
safe_iterator_t<ForwardRng>
min_element(ForwardRng&& range, Comp comp = {})
{
    return std::min_element(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename T, typename Comp = std::less<>,
          REQUIRES(IndirectStrictWeakOrder<Comp, const T*>)>
constexpr std::pair<const T&, const T&> minmax(const T& a, const T& b, Comp comp)
{
    return std::minmax(a, b, std::move(comp));
}

template <typename T, typename Comp = std::less<>,
        REQUIRES(Copyable<T> &&
                 IndirectStrictWeakOrder<Comp, const T*>)>
constexpr std::pair<T, T> minmax(std::initializer_list<T> ilist, Comp comp)
{
    return std::minmax(ilist, std::move(comp));
}

template <typename ForwardRng, typename Comp = std::less<>,
          REQUIRES(Copyable<value_type_t<iterator_t<ForwardRng>>> &&
                   IndirectStrictWeakOrder<Comp, iterator_t<ForwardRng>>)>
constexpr std::pair<range_value_type_t<ForwardRng>, range_value_type_t<ForwardRng>>
minmax(ForwardRng&& range, Comp comp = {})
{
    const auto p = std::minmax_element(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
    return {*p.first, *p.second};
}

template <typename ForwardIt, typename Comp = std::less<>,
          REQUIRES(ForwardIterator<ForwardIt> &&
                   IndirectStrictWeakOrder<Comp, ForwardIt>)>
constexpr std::pair<ForwardIt, ForwardIt>
minmax_element(ForwardIt first, ForwardIt last, Comp comp = {})
{
    return std::minmax_element(std::move(first), std::move(last), std::move(comp));
}

template <typename ForwardRng, typename Comp = std::less<>,
          REQUIRES(ForwardRange<ForwardRng> &&
                   IndirectStrictWeakOrder<Comp, iterator_t<ForwardRng>>)>
constexpr std::pair<safe_iterator_t<ForwardRng>, safe_iterator_t<ForwardRng>>
minmax_element(ForwardRng&& range, Comp comp = {})
{
    return std::minmax_element(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename InputIt1, typename InputIt2, typename Comp = std::less<>,
          REQUIRES(InputIterator<InputIt1> &&
                   InputIterator<InputIt2> &&
                   IndirectStrictWeakOrder<Comp, InputIt1, InputIt2>)>
bool lexicographical_compare(InputIt1 first1, InputIt2 last1,
                             InputIt2 first2, InputIt2 last2, Comp comp = {})
{
    return std::lexicographical_compare(std::move(first1), std::move(last1),
                                        std::move(first2), std::move(last2),
                                        std::move(comp));
}

template <typename InputRng1, typename InputRng2, typename Comp = std::less<>,
        REQUIRES(InputRange<InputRng1> &&
                 InputRange<InputRng2> &&
                 IndirectStrictWeakOrder<Comp, iterator_t<InputRng1>, iterator_t<InputRng2>>)>
bool lexicographical_compare(InputRng1&& range1, InputRng2&& range2, Comp comp = {})
{
    return std::lexicographical_compare(detail::adl_begin(range1), detail::adl_end(range1),
                                        detail::adl_begin(range2), detail::adl_end(range2),
                                        std::move(comp));
}

template <typename ForwardIt1, typename ForwardIt2, typename Pred = std::equal_to<>,
          REQUIRES(ForwardIterator<ForwardIt1> &&
                   ForwardIterator<ForwardIt2> &&
                   IndirectlyComparable<ForwardIt1, ForwardIt2, Pred>)>
NANORANGE_DEPRECATED
bool is_permutation(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2, Pred pred = {})
{
    return std::is_permutation(std::move(first1), std::move(last1), std::move(first2), std::move(pred));
}

template <typename ForwardIt1, typename ForwardIt2, typename Pred = std::equal_to<>,
        REQUIRES(ForwardIterator<ForwardIt1> &&
                 ForwardIterator<ForwardIt2> &&
                 IndirectlyComparable<ForwardIt1, ForwardIt2, Pred>)>
bool is_permutation(ForwardIt1 first1, ForwardIt1 last1, ForwardIt2 first2, ForwardIt2 last2, Pred pred = {})
{
    return std::is_permutation(std::move(first1), std::move(last1), std::move(first2), std::move(last2), std::move(pred));
}

template <typename ForwardRng1, typename ForwardRng2, typename Pred = std::equal_to<>,
        REQUIRES(ForwardRange<ForwardRng1> &&
                 ForwardRange<ForwardRng2> &&
                 IndirectlyComparable<iterator_t<ForwardRng1>, iterator_t<ForwardRng2>, Pred>)>
bool is_permutation(ForwardRng1&& range1, ForwardRng2&& range2, Pred pred = {})
{
    return std::is_permutation(detail::adl_begin(range1), detail::adl_end(range2),
                               detail::adl_begin(range2), detail::adl_end(range2),
                               std::move(pred));
}

template <typename BidirIt, typename Comp = std::less<>,
          REQUIRES(BidirectionalIterator<BidirIt> &&
                   Sortable<BidirIt, Comp>)>
bool next_permutation(BidirIt first, BidirIt last, Comp comp = {})
{
    return std::next_permutation(std::move(first), std::move(last), std::move(comp));
}

template <typename BidirRng, typename Comp = std::less<>,
        REQUIRES(BidirectionalRange<BidirRng> &&
                 Sortable<iterator_t<iterator_t<BidirRng>>, Comp>)>
bool next_permutation(BidirRng&& range, Comp comp = {})
{
    return std::next_permutation(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

template <typename BidirIt, typename Comp = std::less<>,
        REQUIRES(BidirectionalIterator<BidirIt> &&
                 Sortable<BidirIt, Comp>)>
bool prev_permutation(BidirIt first, BidirIt last, Comp comp = {})
{
    return std::prev_permutation(std::move(first), std::move(last), std::move(comp));
}

template <typename BidirRng, typename Comp = std::less<>,
        REQUIRES(BidirectionalRange<BidirRng> &&
                 Sortable<iterator_t<BidirRng>, Comp>)>
bool prev_permutation(BidirRng&& range, Comp comp = {})
{
    return std::prev_permutation(detail::adl_begin(range), detail::adl_end(range), std::move(comp));
}

/*
 * Numeric algorithms
 */

// N.B The Ranges TS does not specify constrained versions of these
// functions. What follows is a pure extension, with my (probably incorrect)
// guess about what the constraints should be

template <typename ForwardIt, typename T,
          REQUIRES(ForwardIterator<ForwardIt> &&
                   Writable<ForwardIt, const T&> &&
                   WeaklyIncrementable<T>)>
void iota(ForwardIt first, ForwardIt last, T value)
{
    return std::iota(std::move(first), std::move(last), std::move(value));
}

template <typename ForwardRng, typename T,
          REQUIRES(ForwardRange<ForwardRng> &&
                   Writable<iterator_t<ForwardRng>, T>)>
void iota(ForwardRng&& range, T value)
{
    return std::iota(detail::adl_begin(range), detail::adl_end(range), std::move(value));
}

template <typename InputIt, typename T, typename BinOp = std::plus<>,
          REQUIRES(InputIterator<InputIt> &&
                   std::is_assignable<T,
                           std::result_of_t<BinOp&(const T&, value_type_t<InputIt>)>>::value)>
T accumulate(InputIt first, InputIt last, T init, BinOp op = {})
{
    return std::accumulate(std::move(first), std::move(last), std::move(init), std::move(op));
}

template <typename InputRng, typename T, typename BinOp = std::plus<>,
        REQUIRES(InputRange<InputRng> &&
                std::is_assignable<T,
                         std::result_of_t<BinOp&(const T&, range_value_type_t<InputRng>)>>::value)>
T accumulate(InputRng&& range, T init, BinOp op = {})
{
    return std::accumulate(detail::adl_begin(range), detail::adl_end(range), std::move(init), std::move(op));
}

// Oh boy
template <typename InputIt1, typename InputIt2, typename T,
          typename BinOp1 = std::plus<>, typename BinOp2 = std::multiplies<>,
          REQUIRES(InputIterator<InputIt1> &&
                   InputIterator<InputIt2> &&
                   std::is_assignable<T, std::result_of_t<
                       BinOp1&(T, std::result_of_t<
                           BinOp2&(value_type_t<InputIt1>, value_type_t<InputIt2>)>)>>::value)>
NANORANGE_DEPRECATED
T inner_product(InputIt1 first1, InputIt1 last1, InputIt2 first2,
                T value, BinOp1 op1 = {}, BinOp2 op2 = {})
{
    return std::inner_product(std::move(first1), std::move(last1), std::move(first2),
                              std::move(value), std::move(op1), std::move(op2));
}

template <typename InputIt1, typename InputIt2, typename T,
        typename BinOp1 = std::plus<>, typename BinOp2 = std::multiplies<>,
        REQUIRES(InputIterator<InputIt1> &&
                         InputIterator<InputIt2> &&
                                 std::is_assignable<T, std::result_of_t<
                                 BinOp1&(T, std::result_of_t<
                         BinOp2&(value_type_t<InputIt1>, value_type_t<InputIt2>)>)>>::value)>
T inner_product(InputIt1 first1, InputIt1 last1, InputIt2 first2, InputIt2 last2,
                T value, BinOp1 op1 = {}, BinOp2 op2 = {})
{
    // Thanks cppreference
    while (first1 != last1 && first2 != last2) {
        value = op1(value, op2(*first1, *first2));
        ++first1;
        ++first2;
    }
    return value;
}

template <typename InputRng1, typename InputRng2, typename T,
        typename BinOp1 = std::plus<>, typename BinOp2 = std::multiplies<>,
        REQUIRES(InputRange<InputRng1> &&
                         InputRange<InputRng2> &&
                                 std::is_assignable<T, std::result_of_t<
                                 BinOp1&(T, std::result_of_t<
                         BinOp2&(range_value_type_t<InputRng1>, range_value_type_t<InputRng2>)>)>>::value)>
T inner_product(InputRng1&& range1, InputRng2&& range2,
                T value, BinOp1 op1 = {}, BinOp2 op2 = {})
{
    return nanorange::inner_product(
                 detail::adl_begin(range1), detail::adl_end(range1),
                 detail::adl_begin(range2), detail::adl_end(range2),
                 std::move(value), std::move(op1), std::move(op2));
}

template <typename InputIt,  typename OutputIt, typename BinOp = std::minus<>,
          REQUIRES(InputIterator<InputIt> &&
                   OutputIterator<OutputIt,
                        std::result_of_t<BinOp&(value_type_t<InputIt>, value_type_t<InputIt>)>>)>
OutputIt adjacent_difference(InputIt first, InputIt last, OutputIt ofirst, BinOp op = {})
{
    return std::adjacent_difference(std::move(first), std::move(last), std::move(ofirst), std::move(op));
}

template <typename InputRng,  typename OutputIt, typename BinOp = std::minus<>,
        REQUIRES(InputRange<InputRng> &&
                OutputIterator<OutputIt,
                         std::result_of_t<BinOp&(range_value_type_t<InputRng>, range_value_type_t<InputRng>)>>)>
OutputIt adjacent_difference(InputRng&& range, OutputIt ofirst, BinOp op = {})
{
    return std::adjacent_difference(detail::adl_begin(range), detail::adl_end(range), std::move(ofirst), std::move(op));
}

template <typename InputIt, typename OutputIt, typename BinOp = std::plus<>,
          REQUIRES(InputIterator<InputIt> &&
                   OutputIterator<OutputIt,
                       std::result_of_t<BinOp&(value_type_t<InputIt>, value_type_t<InputIt>)>>)>
OutputIt partial_sum(InputIt first, InputIt last, OutputIt ofirst, BinOp op = {})
{
    return std::partial_sum(std::move(first), std::move(last), std::move(ofirst), std::move(op));
}

template <typename InputRng, typename OutputIt, typename BinOp = std::plus<>,
        REQUIRES(InputRange<InputRng> &&
                OutputIterator<OutputIt,
                         std::result_of_t<BinOp&(range_value_type_t<InputRng>, range_value_type_t<InputRng>)>>)>
OutputIt partial_sum(InputRng&& range, OutputIt ofirst, BinOp op = {})
{
    return std::partial_sum(detail::adl_begin(range), detail::adl_end(range), std::move(ofirst), std::move(op));
}

#undef CONCEPT
#undef REQUIRES

} // end namespace nanorange

#endif
