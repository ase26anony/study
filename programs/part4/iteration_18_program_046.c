/* This program is designed to trigger the partitioning classification logic
   in GCC's OpenACC neutering/broadcast analysis, specifically targeting the
   switch statement that returns descriptive strings for partitioning types. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SMALL_SIZE 128

/* Variant 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 0: gang redundant - only num_gangs(1) */
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Variant 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 1: gang partitioned - only num_gangs specified */
    #pragma acc kernels num_gangs(8) copy(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

/* Variant 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 2: worker partitioned - only num_workers specified */
    #pragma acc parallel num_workers(4) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

/* Variant 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(4) num_workers(2) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SMALL_SIZE; j++) {
                #pragma acc loop vector
                for (k = 0; k < n; k++) {  /* Runtime-dependent bound */
                    dst[i][j] += src[i][k] * 3;
                }
            }
        }
    }
}

/* Variant 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 4: vector partitioned - only vector_length specified */
    #pragma acc kernels vector_length(32) copy(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Variant 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel num_gangs(8) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 6: worker+vector partitioned */
    #pragma acc kernels num_workers(4) vector_length(128) \
        copy(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] % 17;
            }
        }
    }
}

/* Variant 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j, k;
    
    /* Case 7: fully partitioned - all three clauses specified */
    #pragma acc parallel num_gangs(16) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SMALL_SIZE; j++) {
                #pragma acc loop vector
                for (k = 0; k < n; k++) {  /* Runtime-dependent bound */
                    dst[i][j] += src[i][k] * src[j][k];
                }
            }
        }
    }
}

/* Conditional compilation for different partitioning behaviors */
#ifdef VARIANT_A
#define USE_GANG_REDUNDANT
#elif defined(VARIANT_B)
#define USE_WORKER_VECTOR
#else
#define USE_FULL_PARTITIONING
#endif

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int i, j, n;
    volatile int checksum = 0;  /* Prevent optimization */
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = i * 1000 + j;
        }
    }
    
    /* Read runtime-dependent loop bound */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE / 2;
    } else {
        printf("Enter loop bound (1-%d): ", SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SIZE) n = SIZE / 2;
    }
    
    /* Clear destination array */
    memset(dst, 0, sizeof(dst));
    
    /* Call different functions based on conditional compilation */
    #ifdef USE_GANG_REDUNDANT
    compute_gang_redundant(n, src, dst);
    compute_gang_partitioned(n, src, dst);
    #elif defined(USE_WORKER_VECTOR)
    compute_worker_partitioned(n, src, dst);
    compute_vector_partitioned(n, src, dst);
    compute_worker_vector_partitioned(n, src, dst);
    #else
    /* Call all functions to trigger all partitioning cases */
    compute_gang_redundant(n, src, dst);
    compute_gang_partitioned(n, src, dst);
    compute_worker_partitioned(n, src, dst);
    compute_gang_worker_partitioned(n, src, dst);
    compute_vector_partitioned(n, src, dst);
    compute_gang_vector_partitioned(n, src, dst);
    compute_worker_vector_partitioned(n, src, dst);
    compute_fully_partitioned(n, src, dst);
    #endif
    
    /* Calculate checksum to prevent optimization */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
