.PHONY: all

all: primes-seq.out primes-seq-opt.out

primes-seq.out: primes-seq.cpp sieve.cpp
	mpic++ -Wall -Wextra -Wpedantic -std=c++14 -g -o primes-seq.out primes-seq.cpp sieve.cpp

primes-seq-opt.out: primes-seq.cpp sieve.cpp
	mpic++ -Wall -Wextra -Wpedantic -std=c++14 -O3 -o primes-seq-opt.out primes-seq.cpp sieve.cpp
