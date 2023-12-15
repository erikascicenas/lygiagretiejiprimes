#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cassert>

#include <mpi.h>

#include "sieve.h"

using cint = std::int64_t;

constexpr double MINIMUM_TIME_UPDATE = 5; //seconds

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
        std::cerr << "Usage: " << argv[0] << " MAXNUM SEGMENT_LENGTH SAVE_PRIMES \n"; //TODO fix
        return -1;
    }

    cint maxnum = std::atoll(argv[1]);
    maxnum = maxnum % 2 ? maxnum : maxnum + 1;
    const cint SEGMENT = std::atoll(argv[2]);
    const bool SAVEPRIMES = argv[3][0] == 't';

    if(!pid)
        std::cout << "ARGUMENTS:\n" << "MAXNUM: " << maxnum << "\nSEGMENT LENGTH: " << SEGMENT << 
            "\nSAVE? " << (SAVEPRIMES ? "true" : "false") << std::endl;

    cint iters = std::ceil(static_cast<double>(maxnum - 1)/(2*SEGMENT + 2)); //todo: load splitting
    std::vector<cint> primes;
    cint prime_num = 0;
    //First, find primes in sqrt(N) interval for other processes to use
    cint maxnum_init = std::sqrt(maxnum)+1;
    cint init_iters = std::ceil(static_cast<double>(maxnum_init - 1)/(2*SEGMENT + 2));
    std::vector<cint> i_primes;

    double t_start = MPI_Wtime();

    if(!pid) {
        std::cout << "Initializing sieving primes\n";
        for(cint i = 0; i < init_iters; ++i) {
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

    if(!pid) std::cout << std::fixed << std::setprecision(2) << MPI_Wtime() << "s:\tStarting main sieving!\n";

    double last_update = MPI_Wtime();
    cint last_i = pid;

    for(cint i = init_iters + pid; i < iters; i += nproc) {
        if(MPI_Wtime() - last_update >= MINIMUM_TIME_UPDATE) {
            double time_intv = MPI_Wtime() - last_update;
            last_update += time_intv;
            std::cout << std::fixed;
            std::cout << std::setprecision(1) << last_update << "s: [" << pid << "] " << 100.0*(i-pid)/(iters - init_iters) << "%\t";
            std::cout << "Current number of primes found by process: " << (SAVEPRIMES ? primes.size() : prime_num);
            std::cout << "\tETA: " << time_intv/(i - last_i)*(iters - init_iters - i + pid) << 's' << std::endl;
            last_i = i;
        }
        cint start = 3 + 2*i + 2*i*SEGMENT;
        cint end = 3 + 2*i + 2*(i+1)*SEGMENT;
        end = end > maxnum ? maxnum : end;
        // std::cout << i << std::endl; TODO add pretty output
        Sieve s(end, start, i_primes); //Lifetime until loop end
        if(SAVEPRIMES)
            primes.insert(std::end(primes), std::begin(s.getprimevector()), std::end(s.getprimevector()));
        else
            prime_num += s.getnumprimes();
    }

    if(!pid) std::cout << MPI_Wtime() << "s:\tPrime sieving complete; collecting results\n";
    
    cint msg_length, *recvbuf;

    if(SAVEPRIMES) {
        msg_length = message_length(maxnum);
        primes.resize(msg_length, -1);
        recvbuf = new cint[nproc*msg_length];
        MPI_Gather(primes.data(), primes.size(), MPI_INT64_T, recvbuf, msg_length, MPI_INT64_T, 0, MPI_COMM_WORLD);
    } else {
        recvbuf = new cint[nproc];
        MPI_Gather(&prime_num, 1, MPI_INT64_T, recvbuf, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);
    }

    if(!pid) {
        std::cout << "Time: " << MPI_Wtime() - t_start << " seconds\n";
        if(SAVEPRIMES) {
            i_primes.reserve(nproc*msg_length);
            for(cint i = 0; i < nproc*msg_length; ++i) {
                if(recvbuf[i] <= 0) continue;
                i_primes.push_back(recvbuf[i]);
            }
            i_primes.insert(i_primes.begin(), 2);
            std::cout << "Found primes: " << i_primes.size() << std::endl;
            std::ofstream prime_out("primes.out");
            for(auto p : i_primes) prime_out << p << std::endl;
        } else {
            prime_num = i_primes.size() + 1; //+1 for 2
            for(int i = 0; i < nproc; ++i)
                prime_num += recvbuf[i];
            std::cout << "Found primes: " << prime_num << std::endl;
        }
    }

    delete[] recvbuf;
    MPI_Finalize();
    return 0;
}