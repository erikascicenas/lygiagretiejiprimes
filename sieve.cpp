#include "sieve.h"
#include <stdexcept>
#include <cmath>

inline std::size_t convertnumtoidx(std::size_t num, std::size_t start) {
    if(num % 2 == 0) throw new std::invalid_argument("Tried to get index of even number");
    return (num-start)/2;
}

void mark_primes(std::vector<bool>& sieve_arr, std::size_t prime, std::size_t start, std::size_t maxnum) {
    if(prime >= start && sieve_arr[convertnumtoidx(prime, start)]) return; //Skip checking if we are given a composite number
    for(std::size_t i = 3*prime; i <= maxnum; i += 2*prime) {
        if(i < start) continue;
        sieve_arr[convertnumtoidx(i, start)] = true;
    }
}

Sieve::Sieve(std::size_t end, std::size_t start, const std::vector<std::size_t>& prime_list):
prime_list(prime_list), start(start) {
    //Error checking
    if(start % 2 == 0 || end % 2 == 0) throw new std::invalid_argument("Start/end cannot be even number.");


    found_primes = std::vector<std::size_t>();
    sieve_array = std::vector<bool>((end-start)/2);
    maxnum = end;
    if(!prime_list.empty()) {
        for(auto prime : prime_list)
            mark_primes(sieve_array, prime, start, maxnum);
    }

    processed = false;
}

void Sieve::process_sieve() {
    if(processed) return;
    auto maxiter = static_cast<std::size_t>((maxnum));
    for(size_t i = start; i < maxiter; i+=2) {
        mark_primes(sieve_array, i, start, maxnum);
    }

    for(size_t i = start; i <= maxnum; i+=2) {
        if(!sieve_array[convertnumtoidx(i, start)]) found_primes.push_back(i);
    }

    processed = true;
}

std::size_t Sieve::getnumprimes() {
    if(!processed)
        process_sieve();
    return found_primes.size();
}

const std::size_t* Sieve::getprimes() {
    if(!processed)
        process_sieve();
    return found_primes.data();
}

const std::vector<std::size_t>& Sieve::getprimevector() {
    if(!processed)
        process_sieve();
    return found_primes;
}