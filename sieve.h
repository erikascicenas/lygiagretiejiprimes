#ifndef SIEVE_H
#define SIEVE_H
#include <cstddef>
#include <vector>
#include <cstdint>


/// @brief A class to hold our sieve from an arbitrary starting point.
class Sieve {
public:
    /// @brief Generate a sieve.
    /// @param end What number do we wish to end our sieve on. Must be odd.
    /// @param start What number do we wish to start our sieve on. Must be odd and default to 3.
    /// @param prime_list A vector of initial primes to mark. This is used in the segmented sieve.
    Sieve(std::int64_t end, std::int64_t start = 3, const std::vector<std::int64_t>& prime_list = {});

    /// @brief Process the sieve. Can be called separately, but nothing will break if you don't.
    void process_sieve();
    /// @brief Get the number of primes we found in the sieve.
    /// @return number of primes in sieve.
    std::int64_t getnumprimes();
    /// @brief Get primes.
    /// @return Primes.
    const std::int64_t* getprimes();
    /// @brief get primes, but a vector.
    /// @return the same primes, but a vector.
    const std::vector<std::int64_t>& getprimevector();
private:
    const std::vector<std::int64_t>& prime_list;
    std::vector<std::int64_t> found_primes;
    std::vector<bool> sieve_array;
    std::int64_t maxnum, start;
    bool processed;
};

#endif //SIEVE_H