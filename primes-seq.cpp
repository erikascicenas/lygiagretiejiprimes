#include <iostream>
#include <vector>
#include <cmath>

#include "sieve.h"

int main(int argc, char** argv) {
    //Currently, implement basic sieve with optimizations

    Sieve s(1000000+3);

    //s.process_sieve();
    std::cout << "Found primes: " << s.getnumprimes() << std::endl;

    return 0;
}