#include "sieve.h"
#include <stdexcept>
#include <cmath>


/// @brief Find index of number in sieve
/// @param num Number whose index we want to find
/// @param start start of sieve
/// @return index in sieve
inline std::int64_t convertnumtoidx(std::int64_t num, std::int64_t start) {
    if(num % 2 == 0) throw new std::invalid_argument("Tried to get index of even number");
    return (num-start)/2;
}

/// @brief Mark primes in sieve array.
/// @param sieve_arr Sieve vector. Only stores odd numbers in interval.
/// @param prime Prime whose multiples to mark.
/// @param start Start of sieve.
/// @param maxnum End of sieve.
void mark_primes(std::vector<bool>& sieve_arr, std::int64_t prime, std::int64_t start, std::int64_t maxnum) {
    if(prime >= start && sieve_arr[convertnumtoidx(prime, start)]) return; //Skip checking if we are given a composite number
    std::int64_t coeff = static_cast<int64_t>(std::ceil(static_cast<double>(start) / prime));
    coeff = coeff % 2 ? coeff : coeff + 1;
    std::int64_t i = (coeff > 3 ? coeff : 3)*prime;
    for(; i <= maxnum; i += 2*prime) {
        // if(i < start) continue;
        sieve_arr[convertnumtoidx(i, start)] = true;
    }
}

Sieve::Sieve(std::int64_t end, std::int64_t start, const std::vector<std::int64_t>& prime_list):
prime_list(prime_list), start(start) {
    //Error checking
    if(start % 2 == 0 || end % 2 == 0) throw new std::invalid_argument("Start/end cannot be even number.");


    found_primes = std::vector<std::int64_t>();
    sieve_array = std::vector<bool>((end-start)/2 + 1);
    maxnum = end;
    if(!prime_list.empty()) {
        for(auto prime : prime_list)
            mark_primes(sieve_array, prime, start, maxnum);
    }

    processed = false;
}

void Sieve::process_sieve() {
    if(processed) return;
    auto maxiter = static_cast<std::int64_t>(std::sqrt(maxnum));
    for(size_t i = start; i <= maxiter; i+=2) {
        mark_primes(sieve_array, i, start, maxnum);
    }

    for(size_t i = start; i <= maxnum; i+=2) {
        if(!sieve_array[convertnumtoidx(i, start)]) found_primes.push_back(i);
    }

    processed = true;
}

std::int64_t Sieve::getnumprimes() {
    if(!processed)
        process_sieve();
    return found_primes.size();
}

const std::int64_t* Sieve::getprimes() {
    if(!processed)
        process_sieve();
    return found_primes.data();
}

const std::vector<std::int64_t>& Sieve::getprimevector() {
    if(!processed)
        process_sieve();
    return found_primes;
}