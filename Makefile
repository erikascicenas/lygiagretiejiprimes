primes-seq.out: primes-seq.cpp sieve.cpp
	mpic++ -Wall -Wextra -Wpedantic -std=c++14 -g -o primes-seq.out primes-seq.cpp sieve.cpp
