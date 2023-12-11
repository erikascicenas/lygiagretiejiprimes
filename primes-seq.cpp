#include <iostream>
#include <vector>
#include <cmath>

#include "sieve.h"

int main(int argc, char** argv) {
    //Currently, implement basic sieve with optimizations

    // Sieve s(1000000+3);
    // Sieve s1(500000+3);
    // Sieve s2(1000000+3, 500000+3+2, s1.getprimevector());
    Sieve s1(21), s2(51, 23, s1.getprimevector());

    //s.process_sieve();
    std::cout << "Found primes: " << s1.getnumprimes() + s2.getnumprimes() << std::endl;
    for(auto x : s1.getprimevector()) 
        std::cout << x << std::endl;
    for(auto x : s2.getprimevector()) 
        std::cout << x << std::endl;
    
    // std::cout << "Should be 78499\n.";

    return 0;
}