#include <iostream>
#include <vector>
#include <cmath>
#include <cstdlib>

#include "sieve.h"

// const std::size_t MAX_MEMORY = 1000000000; //1GB
// constexpr std::size_t MAX_SEGMENT = MAX_MEMORY / sizeof(std::size_t);

const std::size_t MAX_SEGMENT = 20;

int main(int argc, char** argv) {
    //Currently, basic segmented sieve.
    if(argc != 2) {
        std::cerr << "Usage: " << argv[0] << " MAXNUM\n";
        return -1;
    }
    std::size_t maxnum = std::atoll(argv[1]);
    maxnum = maxnum % 2 ? maxnum : maxnum + 1;
    std::size_t iters = (maxnum - 1)/(2*MAX_SEGMENT + 2) + 1; //todo: load splitting
    std::vector<std::size_t> primes;
    
    for(std::size_t i = 0; i < iters; ++i) {
        std::size_t start = 3 + 2*i + 2*i*MAX_SEGMENT;
        std::size_t end = 3 + 2*i + 2*(i+1)*MAX_SEGMENT;
        end = end > maxnum ? maxnum : end;
        // std::cout << i << std::endl; TODO add pretty output
        Sieve s(end, start, primes); //Lifetime until loop end
        primes.insert(std::end(primes), std::begin(s.getprimevector()), std::end(s.getprimevector()));
    }

    std::cout << "Found primes: " << primes.size() << std::endl;

    return 0;
}