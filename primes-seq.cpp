#include <iostream>
#include <vector>
#include <iomanip>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cassert>

#include <mpi.h>

#include "sieve.h"

using cint = std::int64_t;


// const cint SEGMENT = 20;

inline cint message_length(cint maxnum) {
    return 1.5 * maxnum / std::log(maxnum); //Based on prime counting function
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int pid; MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    int nproc; MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    //Currently, basic segmented sieve.
    if(argc != 4) {
        std::cerr << "Usage: " << argv[0] << " MAXNUM MAX_MEMORY(GB) JOBS\n";
        return -1;
    }

    cint maxnum = std::atoll(argv[1]);
    maxnum = maxnum % 2 ? maxnum : maxnum + 1;
    const cint MAX_MEMORY = std::atof(argv[2])*1000000000 - nproc*message_length(maxnum)*sizeof(cint); //1GB
    assert(MAX_MEMORY > 0);
    const cint MAX_SEGMENT = MAX_MEMORY / (2*sizeof(bool)*nproc);
    assert(MAX_SEGMENT > 1000);
    const cint SEGMENT = MAX_SEGMENT / std::atoi(argv[3]);

    cint iters = std::ceil(static_cast<double>(maxnum - 1)/(2*SEGMENT + 2)); //todo: load splitting
    std::vector<cint> primes;
    //First, find primes in sqrt(N) interval for other processes to use
    cint maxnum_init = std::sqrt(maxnum)+1;
    cint init_iters = std::ceil(static_cast<double>(maxnum_init - 1)/(2*SEGMENT + 2));
    std::vector<cint> i_primes;

    double t_start = MPI_Wtime();

    if(!pid) {
        for(cint i = 0; i < init_iters; ++i) {
            std::cout << "Initializing sieving prines\n";
            cint start = 3 + 2*i + 2*i*SEGMENT;
            cint end = 3 + 2*i + 2*(i+1)*SEGMENT;
            end = end > maxnum ? maxnum : end;
            Sieve s(end, start, i_primes); //Lifetime until loop end
            i_primes.insert(std::end(i_primes), std::begin(s.getprimevector()), std::end(s.getprimevector()));
        }
        i_primes.resize(message_length(3 + 2*init_iters + 2*init_iters*SEGMENT), -i_primes.size());
    } else {
        i_primes.resize(message_length(3 + 2*init_iters + 2*init_iters*SEGMENT));
    }
    MPI_Bcast(i_primes.data(), i_primes.size(), MPI_INT64_T, 0, MPI_COMM_WORLD);
    i_primes.resize(-i_primes[i_primes.size()-1]);

    primes.reserve(message_length(maxnum));

    if(!pid) std::cout << std::fixed << std::setprecision(2) << MPI_Wtime() << ":\tStarting main sieving!\n";

    for(cint i = init_iters + pid; i < iters; i += nproc) {
        std::cout << std::fixed;
        std::cout << std::setprecision(1) << MPI_Wtime() << "s: [" << pid << "] " << 100.0*(i-pid)/(iters - init_iters) << "%\t";
        std::cout << "Current number of primes found by process: " << primes.size() << std::endl;
        cint start = 3 + 2*i + 2*i*SEGMENT;
        cint end = 3 + 2*i + 2*(i+1)*SEGMENT;
        end = end > maxnum ? maxnum : end;
        // std::cout << i << std::endl; TODO add pretty output
        Sieve s(end, start, i_primes); //Lifetime until loop end
        primes.insert(std::end(primes), std::begin(s.getprimevector()), std::end(s.getprimevector()));
    }

    if(!pid) std::cout << MPI_Wtime() << "s:\tPrime sieving complete; collecting results\n";

    cint msg_length = message_length(maxnum);
    assert(primes.size() < msg_length);
    primes.resize(msg_length, -1);
    cint* recvbuf = new cint[nproc*msg_length];
    MPI_Gather(primes.data(), primes.size(), MPI_INT64_T, recvbuf, msg_length, MPI_INT64_T, 0, MPI_COMM_WORLD);

    if(!pid) {
        i_primes.reserve(nproc*msg_length);
        for(cint i = 0; i < nproc*msg_length; ++i) {
            if(recvbuf[i] <= 0) continue;
            i_primes.push_back(recvbuf[i]);
        }
        i_primes.insert(i_primes.begin(), 2);
        // for(auto p : i_primes) std::cout << p << std::endl;
        std::cout << "Found primes: " << i_primes.size() << std::endl;
        std::cout << "Time: " << MPI_Wtime() - t_start << " seconds\n";
    }

    delete[] recvbuf;
    MPI_Finalize();
    return 0;
}