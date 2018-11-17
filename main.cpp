// #include "grammar2.hpp"
#include "grammarC.hpp"

#include <iostream>
#include <cstdlib>

using CXGram::Grammar;
using CXGram::Rule;
using CXGram::WRule;

#define DEFINE_SYMBOL( _n_, _is_term_, _text_ ) \
    struct _n_ { \
        static constexpr const char * Name = _text_; \
        static constexpr bool IsTerminal = _is_term_; \
    }

#define DEFINE_TERM( _n_, _text_ )    DEFINE_SYMBOL( _n_, true, _text_ )
#define DEFINE_NONTERM( _n_, _text_ ) DEFINE_SYMBOL( _n_, false, _text_ )

DEFINE_NONTERM(A, "A");
DEFINE_NONTERM(B, "B");
DEFINE_NONTERM(C, "C");
DEFINE_NONTERM(D, "D");
DEFINE_NONTERM(E, "E");

DEFINE_TERM(a, "a");
DEFINE_TERM(b, "b");
DEFINE_TERM(c, "c");
DEFINE_TERM(d, "d");
DEFINE_TERM(e, "e");

constexpr Grammar<
    A,
    Rule<A, A, A>,
    Rule<A, A, B, C, D, E>,
    Rule<A, C, D, E>,
    Rule<A, D, E>,
    Rule<B, b>,
    Rule<B, B, E, C>,
    Rule<C, c>,
    Rule<C, c, d, c, d, c>,
    Rule<C, D, E>,
    Rule<D, d>,
    Rule<D, B, E>,
    Rule<E, e>
> toy{};

DEFINE_NONTERM(Expr, "E");
DEFINE_NONTERM(Term, "T");
DEFINE_NONTERM(Factor, "F");
DEFINE_NONTERM(Digits, "<#s>");
DEFINE_NONTERM(Digit, "<#>");

DEFINE_TERM(LParen, "(");
DEFINE_TERM(RParen, ")");

DEFINE_TERM(Plus, "+");
DEFINE_TERM(Minus, "-");

DEFINE_TERM(Times, "*");
DEFINE_TERM(Div, "/");

DEFINE_TERM(n0, "0");
DEFINE_TERM(n1, "1");
DEFINE_TERM(n2, "2");
DEFINE_TERM(n3, "3");
DEFINE_TERM(n4, "4");
DEFINE_TERM(n5, "5");
DEFINE_TERM(n6, "6");
DEFINE_TERM(n7, "7");
DEFINE_TERM(n8, "8");
DEFINE_TERM(n9, "9");

constexpr Grammar<
    Expr,

    Rule<Expr, Term>,
    Rule<Expr, Term>,
    Rule<Expr, Term>,
    Rule<Expr, Expr, Plus, Term>,
    Rule<Expr, Expr, Minus, Term>,

    Rule<Term, Factor>,
    Rule<Term, Factor>,
    Rule<Term, Factor>,
    Rule<Term, Term, Times, Factor>,
    Rule<Term, Term, Div, Factor>,

    Rule<Factor, Digits>,
    Rule<Factor, Digits>,
    Rule<Factor, Digits>,
    Rule<Factor, LParen, Expr, RParen>,

    Rule<Digits, Digit>,
    Rule<Digits, Digits, Digit>,

    Rule<Digit, n0>,
    Rule<Digit, n1>,
    Rule<Digit, n2>,
    Rule<Digit, n3>,
    Rule<Digit, n4>,
    Rule<Digit, n5>,
    Rule<Digit, n6>,
    Rule<Digit, n7>,
    Rule<Digit, n8>,
    Rule<Digit, n9>
> math{};

constexpr Grammar<
    Expr,

    WRule<6, Expr, Term>,
    WRule<2, Expr, Expr, Plus, Term>,
    WRule<2, Expr, Expr, Minus, Term>,

    WRule<6, Term, Factor>,
    WRule<2, Term, Term, Times, Factor>,
    WRule<2, Term, Term, Div, Factor>,

    WRule<8, Factor, Digits>,
    WRule<2, Factor, LParen, Expr, RParen>,

    WRule<4, Digits, Digit>,
    WRule<6, Digits, Digits, Digit>,

    WRule<0, Digit, n0>,
    WRule<0, Digit, n1>,
    WRule<1, Digit, n2>,
    WRule<1, Digit, n3>,
    WRule<1, Digit, n4>,
    WRule<0, Digit, n5>,
    WRule<1, Digit, n6>,
    WRule<0, Digit, n7>,
    WRule<1, Digit, n8>,
    WRule<1, Digit, n9>
> weighted_math{};

template<int I>
struct print {
    template<typename G>
    void operator()(G const & grammar) {
        print<I - 1>{}(grammar);
        std::cout << I << ": " << grammar.template produce<I>() << '\n';
    }
};

template<>
struct print<0> {
    template<typename G>
    void operator()(G const & grammar) {}
};

template<char C> struct checker {};
checker<toy.produce<0>()[0]> c;

int main() {
    std::cout << "toy:\n";
    print<4>{}(toy);

    std::cout << "math:\n";
    print<4>{}(math);
}
