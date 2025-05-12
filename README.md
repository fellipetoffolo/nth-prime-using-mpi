## MPI program for calculating the nth prime number

 This program takes advantage of multiple processing units to calculate the nth prime number based on 
parallelism by data partitioning. It uses the MPI C library for passing messages betweens the proccess,
enabling them to communicate with each other.

## Compilation and Usage

It is necessary to install the libraries libopenmpi-dev and openmpi-bin.

To compile the programa, the command
    ``` mpicc nth_prime.c -o nth_prime ```
should be executed, generating an executable

To run the executable on one machine with multiple cores, run the command
    ``` mpirun --host localhost:<n> ./mpi_nth_prime <N> ```
substituting "n" by the number of cores and "N" by the prime number you want to calculate. Alternatively, to run it on many machines you can run
    ```` mpirun -np <n> --hostfile <file> ./mpi_nth_prime <N> ```
to specify a "file" with all the hosts names and slots, each separated by a new line. In the last case, the number of procecess "n" can be any number between zero and the summed number of cores of all the machines used


## Logic

#### The program works in the following way:

It estimates an upper bound for the nth prime number using the approximation showed by Christian Axler on
the paper "New estimates for the nth prime number", which employed the expression n*(log(n) + log(log(n))).
For this case, a slight variant was used: n*(log(n) + log(log(n))*1.2).

For n ≤ 4, the primes {2, 3, 5, 7} are handled directly without computation.
For n > 4

The program initializes an MPI environment and distributes the task of checking for primes across all 
available processes.

Each process starts testing odd numbers from 11, incrementing by a stride of 2 × the number of tasks 
to ensure coverage without overlap.

A local dynamic array stores the prime numbers found by each process within the estimated upper limit.

Once all processes finish, they send their local results to the root process (rank 0).

The root process gathers all primes, adds the first four known primes, sorts the complete list, and 
retrieves the nth prime.

If the total number of primes found is insufficient, a warning is shown indicating the estimated 
limit was too low.

Finally, the root process reports the nth prime number and the total elapsed execution time.


## Functions

#### 1. The isprime function verifies if a given number is prime. 

- If it is less than 2, it is not prime.
- If it is 2, it is prime.
- If it is bigger than 2:
    - If it is even, it is not prime
    - If it is odd:
    - Compute the square root of n (rounded down).
    - Check all odd numbers from 3 up to the square root:
        - If any of them divides n exactly, it is not prime.
    - If none divide n, then n is prime.
- Why? Well, supposing that a*b = n, and that a > √n and b > √n, then a\*b > n, which is a contradiction, so
at least one of then must be less than or equal to √n.

#### 2. The cmpfunc compares two numbers on the qsort function

- If a - b = 0, the a and b are equal.


#### 3. The main function coordinates the entire MPI-based parallel prime search. 

- It initializes the MPI environment and retrieves:

    - ntasks: the number of MPI processes.

    - rank: the ID of the current process.

- It checks whether the user provided an argument:

    - If not, it prints a usage message and terminates.

- It reads the desired nth prime number from the command-line argument:

    - If nth < 1, the program exits with an error message.

- If nth <= 4, the result is returned directly from a predefined list {2, 3, 5, 7}.]

- If nth > 4, the algorithm proceeds with parallel processing:

    - It estimates an upper bound for the nth prime using the formula: n * (log(n) + log(log(n))) * 1.2

    - Ensures the limit is at least 11.

    - Each process starts checking odd numbers from a unique offset, incrementing by ntasks * 2.


- Each process:

    - Allocates a dynamic array to store the prime numbers it finds.
    
    - Iterates through its range using a stride that avoids overlap with other processes.

    - Expands its array if more primes are found than initially expected.


- All processes:

    - Send the count of local primes to the root process (rank == 0).

    - Then send their actual prime values using MPI_Gather and MPI_Gatherv.


- The root process:

    - Computes the displacements to gather all primes in one buffer.

    - Allocates memory for all received primes plus the 4 known initial ones.

    - Merges the local primes into a single array and prepends the known primes {2, 3, 5, 7}.

    - Sorts the complete array using qsort.


- Final output:

    - If the total number of primes found is less than nth, the estimated bound was too small.

    - Otherwise, it prints the nth prime and the elapsed execution time.