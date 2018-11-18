// Grammar type - take 3 (constexpr)
// Brian Heim 2018-10-27

#include <vector>
#include <iostream>
#include <cstdlib>
#include <unordered_map>
#include <algorithm>
#include <array>

#include <cstdint>
#include <cstddef>

#include <utility>
#include <type_traits>

namespace CXGram {

template<typename... Ts> struct Return {};

template<size_t... I> using ISeq = std::index_sequence<I...>;
template<size_t N> using MakeISeq = std::make_index_sequence<N>;

// Utility functions
constexpr unsigned int Strlen( const char * s ) {
    unsigned int i = 0;
    while ( *s++ )
        ++i;
    return i;
}

constexpr size_t CalcNextI( size_t i ) {
    i ^= i << 13;
    i ^= i >> 7;
    i ^= i >> 17;
    return i;
}

// Utility templates - list
template<typename... Ts> struct List {
    constexpr static auto Size = sizeof...(Ts);
};

template<typename...> struct ListCat;
template<typename... Ls, typename ... Rs>
struct ListCat<List<Ls...>, List<Rs...>> {
    using Result = List<Ls..., Rs...>;
};

template<typename, typename> struct ListAppend;
template<typename Head, typename... Tail>
struct ListAppend<Head, List<Tail...>> { using Result = List<Head, Tail...>; };

// Utility templates - Select
template<size_t, typename> struct Select;

template<size_t I, typename Head, typename... Tail>
struct Select<I, List<Head, Tail...>> { using Result = typename Select<I - 1, List<Tail...>>::Result; };

template<typename Head, typename... Tail>
struct Select<0, List<Head, Tail...>> { using Result = Head; };

// Utility templates - WSelect, weighted select
// We do need a base case because std::conditional_t will eventually recurse here
template<size_t, typename> struct WSelect { using Result = void; };

template<size_t I, typename Head, typename... Tail>
struct WSelect<I, List<Head, Tail...>> {
    using Result = std::conditional_t<I < Head::Weight,
                                      Head,
                                      typename WSelect<I - Head::Weight, List<Tail...>>::Result>;
};

// Utility templates - WeightSum, sum of rule weights
template<typename> struct WeightSum;

template<typename ... Rs>
struct WeightSum<List<Rs...>> {
    constexpr static auto Result = (0 + ... + Rs::Weight);
};

// Simple templates - weighted rule type
template<size_t W, typename L, typename... Rs>
struct WRule {
    using Lhs = L;
    using Rhs = List<Rs...>;
    constexpr static auto Weight = W;
};

// Simple templates - normal rules are just 1-weighted rules
template<typename L, typename... Rs>
using Rule = WRule<1, L, Rs...>;

// Simple templates - checks if list needs to be expanded
template<typename> struct IsAllTerminal;
template<typename ... Rs>
struct IsAllTerminal<List<Rs...>> {
    static constexpr bool Result = (Rs::IsTerminal && ...);
};

// base case chosen when list is empty
template<typename S, typename RList> struct MatchingRules { using Result = RList; };
template<typename S, typename R, typename...Rs>
struct MatchingRules<S, List<R, Rs...>> {
    using Tail = typename MatchingRules<S, List<Rs...>>::Result;
    using Result = std::conditional_t<std::is_same_v<S, typename R::Lhs>,
          typename ListAppend<R, Tail>::Result, Tail>;
};

// Expand a single non-terminal, or don't if it's a terminal
template<size_t, typename, typename, bool> struct ExpandOne;

// I = RNG value, S = symbol to expand, RList = list of rules
template<size_t I, typename S, typename RList>
struct ExpandOne<I, S, RList, false> {
    using Matches = typename MatchingRules<S, RList>::Result;
    constexpr static auto Sum = WeightSum<Matches>::Result;
    using Result = typename WSelect<I % Sum, Matches>::Result::Rhs;
    constexpr static auto NextI = CalcNextI(I);
};

template<size_t I, typename S, typename Rs>
struct ExpandOne<I, S, Rs, true> {
    using Result = List<S>;
    constexpr static auto NextI = I;
};

// Expand an entire list once.
template<size_t, typename, typename, typename> struct ExpandOnce;

// Base case - nothing left to expand
template<size_t InI, typename ExpList, typename RList>
struct ExpandOnce<InI, ExpList, List<>, RList> {
    constexpr static auto I = InI;
    using Result = ExpList;
};

// Expand Next and place its expansion in List<Prev..., _>
template<size_t InI, typename... Prev, typename Next, typename ...Tail, typename RList>
struct ExpandOnce<InI, List<Prev...>, List<Next, Tail...>, RList>
{
    using ExpandedNext = ExpandOne<InI, Next, RList, Next::IsTerminal>;
    constexpr static auto NextI = ExpandedNext::NextI;
    using NewHead = typename ListCat<List<Prev...>, typename ExpandedNext::Result>::Result;
    using ExpandedTail = ExpandOnce<NextI, NewHead, List<Tail...>, RList>;
    using Result = typename ExpandedTail::Result;
    constexpr static auto I = ExpandedTail::I;
};

// Expand entire string repeatedly until all terminal.
template<size_t, typename, typename, bool> struct ExpandImpl;

// Base case - done expanding
template<size_t InI, typename ExpList, typename RList>
struct ExpandImpl<InI, ExpList, RList, true> { using Result = ExpList; };

template<size_t InI, typename ... Ss, typename RList>
struct ExpandImpl<InI, List<Ss...>, RList, false> {
    using Expanded = ExpandOnce<InI, List<>, List<Ss...>, RList>;
    using PrevResult = typename Expanded::Result;
    constexpr static auto I = Expanded::I;
    using Result = typename ExpandImpl<I, PrevResult, RList, IsAllTerminal<PrevResult>::Result || (sizeof...(Ss) > 100)>::Result;
};

// Helper template for Expanding
template<unsigned int I, typename S, typename RList>
struct Expand {
    using Result = typename ExpandImpl<I, List<S>, RList, S::IsTerminal>::Result;
};

// Helpers for Concat
template<const char *, typename ...> struct ConcatImpl;
template<const char * Str, size_t... LhsI, size_t... RhsI, typename Sym, typename ... Syms>
struct ConcatImpl<Str, ISeq<LhsI...>, ISeq<RhsI...>, Sym, Syms...> {
    constexpr static size_t Size = sizeof...(LhsI) + sizeof...(RhsI);
    constexpr static const char String[Size + 1] = {Str[LhsI]..., Sym::Name[RhsI]..., '\0' };
};

template<const char * Str, size_t... LhsI, size_t... RhsI, typename Sym, typename Sym2, typename ... Syms>
struct ConcatImpl<Str, ISeq<LhsI...>, ISeq<RhsI...>, Sym, Sym2, Syms...> {
    constexpr static size_t Size = sizeof...(LhsI) + sizeof...(RhsI);
    constexpr static const char Partial[Size] = {Str[LhsI]..., Sym::Name[RhsI]... };
    constexpr static const char * String = ConcatImpl<
        Partial, MakeISeq<Size>, MakeISeq<Strlen(Sym2::Name)>, Sym2, Syms...>::String;
};

// Strings together all the names of the symbols in Ss into a single const char array
template<typename> struct Concat;
template<typename S, typename ... Ss>
struct Concat<List<S, Ss...>> {
    constexpr static const char Dummy[1] = { '\0' };
    constexpr static const char * String = ConcatImpl<
        (const char*)Dummy, MakeISeq<0>, MakeISeq<Strlen(S::Name)>, S, Ss...>::String;
};

template<unsigned int I, typename S, typename... Rs>
struct ProduceImpl
{
    using Result = typename Expand<I, S, List<Rs...>>::Result;
    constexpr static const char * String = Concat<Result>::String;
};

template<typename S, typename ... Rs>
struct Grammar {
    template<int I>
    constexpr const char * produce() const {
        return ProduceImpl<I, S, Rs...>::String;
    }

    template<unsigned int I>
    static constexpr const char * Production = ProduceImpl<I, S, Rs...>::String;
};

} // namespace CXGram
