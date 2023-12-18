#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <cstdint>
#include <cassert>
#include <cstring>
#include <algorithm>

#include <mpi.h>

#include "sieve.h"

using cint = std::int64_t;

constexpr double MINIMUM_TIME_UPDATE = 5; //seconds

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int pid; MPI_Comm_rank(MPI_COMM_WORLD, &pid);
    int nproc; MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    //Currently, basic segmented sieve.
    if(argc != 5) {
        std::cerr << "Usage: " << argv[0] << " MAXNUM SEGMENT_LENGTH SAVE_PRIMES START_NUMBER\n";
        std::cerr << "MAXNUM -- until what number should the search be executed; +1 if even.\n";
        std::cerr << "SEGMENT_LENGTH -- length of segment used by each process; by that you can judge maximum memory usage\n";
        std::cerr << "SAVE_PRIMES -- if 't' (exactly), it saves primes to file. Increases memory usage significantly for large N\n";
        std::cerr << "START_NUMBER -- number to start searching from; by default set this to 1 (to include 2)";
        std::cerr << std::endl;
        return -1;
    }

    cint maxnum = std::atoll(argv[1]);
    cint start_num = std::atoll(argv[4]);
    bool includetwo = start_num == 1 || start_num == 2;
    start_num = start_num <= 1 ? 3 : start_num;
    start_num = start_num % 2 ? start_num : start_num + 1;
    maxnum = maxnum % 2 ? maxnum : maxnum + 1;
    const cint SEGMENT = std::atoll(argv[2]);
    const bool SAVEPRIMES = argv[3][0] == 't';

    if(!pid)
        std::cout << "ARGUMENTS:\n" << "MAXNUM: " << maxnum << "\nSEGMENT LENGTH: " << SEGMENT << 
            "\nSAVE? " << (SAVEPRIMES ? "true" : "false") <<
            "\nSTART_NUM: " << start_num << std::endl;

    cint iters = std::ceil(static_cast<double>(maxnum - 1)/(2*SEGMENT + 2));
    std::vector<cint> primes;
    cint prime_num = 0;
    //First, find primes in sqrt(N) interval for other processes to use
    cint maxnum_init = std::sqrt(maxnum)+1;
    maxnum_init = maxnum_init % 2 ? maxnum_init : maxnum_init + 1;
    cint init_iters = std::floor(static_cast<double>(maxnum_init - 1)/(2*SEGMENT + 2));
    cint init_itersstart = std::floor(static_cast<double>(start_num - 1)/(2*SEGMENT + 2));
    std::vector<cint> i_primes;

    double t_start = MPI_Wtime();
    cint msg_len;
    if(!pid) {
        std::cout << "Initializing sieving primes\n";
        Sieve s(maxnum_init, 3);
        i_primes.insert(std::end(i_primes), std::begin(s.getprimevector()), std::end(s.getprimevector()));
        msg_len = i_primes.size();
    }
    //Send how many primes to receive.
    MPI_Bcast(&msg_len, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);
    i_primes.resize(msg_len);
    //Send primes
    MPI_Bcast(i_primes.data(), i_primes.size(), MPI_INT64_T, 0, MPI_COMM_WORLD);

    if(!pid) std::cout << std::fixed << std::setprecision(2) << MPI_Wtime() << "s:\tStarting main sieving!\n";

    double last_update = MPI_Wtime();
    cint last_i = pid;
    for(cint i = (init_iters > init_itersstart ? init_iters : init_itersstart) + pid; i < iters; i += nproc) {
        if(MPI_Wtime() - last_update >= MINIMUM_TIME_UPDATE) {
            double time_intv = MPI_Wtime() - last_update;
            last_update += time_intv;
            std::cout << std::fixed;
            std::cout << std::setprecision(1) << last_update << "s: [" << pid << "] " << 100.0*(i-pid)/(iters-init_iters) << "%\t";
            std::cout << "Current number of primes found by process: " << (SAVEPRIMES ? primes.size() : prime_num);
            std::cout << "\tETA: " << time_intv/(i - last_i)*(iters - init_iters - i + pid) << 's' << std::endl;
            last_i = i;
        }
        cint start = 3 + 2*i + 2*i*SEGMENT;
        start = start >= maxnum_init + 2 ? start : maxnum_init + 2;
        start = start < start_num ? start_num : start;
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
    
    cint msg_length, total, *recvbuf, *recvbuf2;
    MPI_Status status;

    if(SAVEPRIMES) {
        recvbuf = new cint[nproc];
        msg_length = primes.size();
        MPI_Gather(&msg_length, 1, MPI_INT64_T, recvbuf, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);
        if(!pid) {
            total = 0;
            for(int i = 0; i < nproc; ++i)
                total += recvbuf[i];
            recvbuf2 = new cint[total];
            std::memcpy(recvbuf2, primes.data(), primes.size()*sizeof(cint));
            cint *temp_pointer = &recvbuf2[primes.size()];
            for(int i = 1; i < nproc; ++i) {
                MPI_Recv(temp_pointer, recvbuf[i], MPI_INT64_T, i, 0, MPI_COMM_WORLD, &status);
                temp_pointer = &temp_pointer[recvbuf[i]];
            }
        } else {
            MPI_Send(primes.data(), primes.size(), MPI_INT64_T, 0, 0, MPI_COMM_WORLD);
        }
    } else {
        recvbuf = new cint[nproc];
        MPI_Gather(&prime_num, 1, MPI_INT64_T, recvbuf, 1, MPI_INT64_T, 0, MPI_COMM_WORLD);
    }

    if(!pid) {
        std::cout << "Time: " << MPI_Wtime() - t_start << " seconds\n";
        if(!includetwo) {
            auto i_primes_end = std::remove_if(i_primes.begin(), i_primes.end(), [start_num](cint i){ return i < start_num; });
            i_primes.erase(i_primes_end, i_primes.end());
        }
        if(SAVEPRIMES) {
            i_primes.reserve(total);
            for(cint i = 0; i < total; ++i)
                i_primes.push_back(recvbuf2[i]);
            if(includetwo)
                i_primes.push_back(2);
            std::cout << "Found primes: " << i_primes.size() << std::endl;
            std::sort(i_primes.begin(), i_primes.end());
            std::ofstream prime_out("primes.out");
            for(auto p : i_primes) prime_out << p << std::endl;
            delete[] recvbuf2;
        } else {
            prime_num = i_primes.size() + (includetwo ? 1 : 0); //+1 for 2
            for(int i = 0; i < nproc; ++i)
                prime_num += recvbuf[i];
            std::cout << "Found primes: " << prime_num << std::endl;
        }
    }

    delete[] recvbuf;
    MPI_Finalize();
    return 0;
}