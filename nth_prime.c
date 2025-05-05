/******************************************************************************
* FILE: mpi_nth_prime.c
* DESCRIPTION:
*   Generates the nth prime number using MPI. The program asks for a command line
*   argument 'n' and, for n > 4, it estimates the upper bound for the nth prime
*   number based on the formula: limit = n*(log(n) + log(log(n)))*1.2.
*   Each process then traversses odd numbers starting from 11, with a step equals
*   to ntasks*2, stores the first primes found and, by the end, the results are
*   grouped in the 0th process, they are the sorted and then the nth prime number
*   is shown.
*
*   For n <= 4, the primes {2, 3, 5, 7} are treated directly.
*
* USAGE:
*   mpirun -np <número_de_processos> ./mpi_nth_prime <n>
*
* AUTHOR: Fellipe Toffolo de Souza
* LAST REVISED: 05/05/2025
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* Function that tests prime numbers */
int isprime(int n) {
    int i, squareroot;
    if (n < 2) return 0;
    if (n % 2 == 0)
        return (n == 2);
    squareroot = (int) sqrt(n);
    for (i = 3; i <= squareroot; i += 2) {
        if (n % i == 0)
            return 0;
    }
    return 1;
}

/* Comparison function for qsort */
int cmpfunc (const void * a, const void * b) {
    return (*(int*)a - *(int*)b);
}

int main (int argc, char *argv[])
{
    int ntasks, rank, nth;
    double start_time, end_time;

    MPI_Init(&argc,&argv);
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    MPI_Comm_size(MPI_COMM_WORLD,&ntasks);

    /* Verifies if the argument was given */
    if (argc < 2) {
        if (rank == 0)
            fprintf(stderr, "Usage: %s n\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    nth = atoi(argv[1]);
    if (nth < 1) {
        if (rank == 0)
            fprintf(stderr, "n must be a positive integer.\n");
        MPI_Finalize();
        exit(1);
    }

    /* Treats the 4 first primes directly */
    if (nth <= 4) {
        if (rank == 0) {
            int primeiros[4] = {2, 3, 5, 7};
            printf("The %d-th prime number is %d\n", nth, primeiros[nth-1]);
        }
        MPI_Finalize();
        return 0;
    }

    start_time = MPI_Wtime();

    /* Estimates an upper bound for the nth prime number (n >= 6) */
    double n_d = (double) nth;
    int limit = (int)(n_d * (log(n_d) + log(log(n_d))) * 1.2);
    if (limit < 11) limit = 11;  /* Ensures that the limit is at least 11 */

    /* Each process covers odd numbers starting from 11 */
    int start_val = 11;
    int mystart = start_val + rank * 2; /* ensures that they are odd */
    int stride = ntasks * 2;            /* skips even numbers */

    /* Dynamic vector for storing local primes */
    int local_count = 0, local_capacity = 100;
    int *local_primes = malloc(local_capacity * sizeof(int));
    if (local_primes == NULL) {
        fprintf(stderr, "Memory allocation error\n");
        MPI_Finalize();
        exit(1);
    }

    /* covers numbers based in each process */
    for (int num = mystart; num <= limit; num += stride) {
        if (isprime(num)) {
            if (local_count >= local_capacity) {
                local_capacity *= 2;
                local_primes = realloc(local_primes, local_capacity * sizeof(int));
                if (local_primes == NULL) {
                    fprintf(stderr, "Memory allocation error\n");
                    MPI_Finalize();
                    exit(1);
                }
            }
            local_primes[local_count++] = num;
        }
    }

    /* Each process sends the numbers of primes found to the 0th process */
    int *recvcounts = NULL;
    if (rank == 0)
        recvcounts = malloc(ntasks * sizeof(int));
    MPI_Gather(&local_count, 1, MPI_INT, recvcounts, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int total_primes = 0;
    int *displs = NULL;
    int *all_primes = NULL;
    if (rank == 0) {
        displs = malloc(ntasks * sizeof(int));
        displs[0] = 0;
        total_primes = recvcounts[0];
        for (int i = 1; i < ntasks; i++) {
            displs[i] = displs[i-1] + recvcounts[i-1];
            total_primes += recvcounts[i];
        }
        /* Allocates memory for the collected primes plus the first 4 primes */
        all_primes = malloc((total_primes + 4) * sizeof(int));
        if (all_primes == NULL) {
            fprintf(stderr, "Memory allocation error\n");
            MPI_Finalize();
            exit(1);
        }
        /* Inserts the first primes */
        all_primes[0] = 2;
        all_primes[1] = 3;
        all_primes[2] = 5;
        all_primes[3] = 7;
    }

    /* Collects the primes founf by the process (following the 4th position) */
    MPI_Gatherv(local_primes, local_count, MPI_INT,
                all_primes ? all_primes+4 : NULL, recvcounts, displs, MPI_INT,
                0, MPI_COMM_WORLD);

    if (rank == 0) {
        int total_count = total_primes + 4;
        /* Sorts the global prime vector */
        qsort(all_primes, total_count, sizeof(int), cmpfunc);
        if (nth > total_count)
            printf("The estimated upper bound (%d) was not enough to find the %d-th prime.\n", limit, nth);
        else
            printf("The %d-th prime number is %d\n", nth, all_primes[nth-1]);
        end_time = MPI_Wtime();
        printf("Elapsed time: %.2lf seconds\n", end_time - start_time);
    }

    free(local_primes);
    if (rank == 0) {
        free(recvcounts);
        free(displs);
        free(all_primes);
    }

    MPI_Finalize();
    return 0;
}
