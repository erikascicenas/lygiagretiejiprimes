#ifndef SIEVE_H
#define SIEVE_H
#include <cstddef>
#include <vector>


/// @brief A class to hold our sieve from an arbitrary starting point.
class Sieve {
public:
    /// @brief Generate a sieve.
    /// @param end What number do we wish to end our sieve on. Must be odd.
    /// @param start What number do we wish to start our sieve on. Must be odd and default to 3.
    /// @param prime_list A vector of initial primes to mark. This is used in the segmented sieve.
    Sieve(std::size_t end, std::size_t start = 3, const std::vector<std::size_t>& prime_list = {});

    /// @brief Process the sieve. Can be called separately, but nothing will break if you don't.
    void process_sieve();
    /// @brief Get the number of primes we found in the sieve.
    /// @return number of primes in sieve.
    std::size_t getnumprimes();
    const std::size_t* getprimes();
    const std::vector<std::size_t>& getprimevector();
private:
    const std::vector<std::size_t>& prime_list;
    std::vector<std::size_t> found_primes;
    std::vector<bool> sieve_array;
    std::size_t maxnum, start;
    bool processed;
};

#endif //SIEVE_H