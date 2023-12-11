#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

#include "sieve.h"

int main(int argc, char** argv) {
    //Currently, basic segmented sieve.
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " MAXNUM\n";
        return -1;
    }
    std::size_t maxnum = std::atoll(argv[1]);
    maxnum = maxnum % 2 ? maxnum : maxnum + 1;
    std::size_t segment = (maxnum/2) % 2 ? maxnum/2 : maxnum/2 - 1;

    Sieve s1(segment), s2(maxnum, segment + 2, s1.getprimevector());

    //s.process_sieve();
    std::cout << "Found primes: " << s1.getnumprimes() + s2.getnumprimes() << std::endl;
    for(auto x : s1.getprimevector()) 
        std::cout << x << std::endl;
    for(auto x : s2.getprimevector()) 
        std::cout << x << std::endl;
    
    // std::cout << "Should be 78499\n.";

    return 0;
}