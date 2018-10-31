// #include "grammar2.hpp"
#include "grammarC.hpp"

#include <iostream>
#include <cstdlib>

/*
void main_grammar2() {
    int i;
    std::cin >> i;
    std::srand( i );
    using namespace grammar2;

    NTERM(A);
    NTERM(B);
    TERM(a);
    TERM(b);

    grammar g(
        A,
        {
            ( A | B, B ),
            ( A | A, A ),
            ( A | a ),
            ( B | B, B, a ),
            ( B | b )
        });

    std::cout << g.produce() << std::endl;
}
*/

template<char C> struct checker {};
void main_grammarC() {
    using namespace grammarC;
    checker<gramC.produce<0>()[0]> c;
    std::cout << grammarC::gramC.produce<1>() << std::endl;
    std::cout << grammarC::gramC.produce<2>() << std::endl;
    std::cout << grammarC::gramC.produce<3>() << std::endl;
    std::cout << grammarC::gramC.produce<4>() << std::endl;
    std::cout << grammarC::gramC.produce<5>() << std::endl;
    std::cout << grammarC::gramC.produce<6>() << std::endl;
    std::cout << grammarC::gramC.produce<7>() << std::endl;
}

int main() { main_grammarC(); }
