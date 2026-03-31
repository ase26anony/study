/* This program is designed to trigger the OpenACC partitioning classification
   logic in GCC's omp-oacc-neuter-broadcast.cc, specifically the switch
   statement returning strings for different partitioning types (cases 0-7). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SMALL_SIZE 128

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    /* Gang redundant - only 1 gang */
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            /* Runtime-dependent inner loop bound */
            int limit = (i % 2 == 0) ? n : SIZE;
            #pragma acc loop vector
            for (j = 0; j < limit; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang partitioned */
void compute_gang_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    /* Gang partitioned - multiple gangs, no workers/vectors specified */
    #pragma acc parallel num_gangs(8) copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            /* Runtime-dependent inner loop bound */
            int limit = (i < n * 100) ? SMALL_SIZE : SIZE;
            #pragma acc loop vector
            for (j = 0; j < limit; j++) {
                dst[i][j] = src[i][j] + i + j;
            }
        }
    }
}

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    /* Worker partitioned - workers only */
    #pragma acc parallel num_workers(4) copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            /* Runtime-dependent inner loop bound */
            int limit = SIZE - (n * 10);
            if (limit < 1) limit = 1;
            #pragma acc loop vector
            for (j = 0; j < limit; j++) {
                dst[i][j] = src[i][j] * 3 - j;
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    /* Gang+worker partitioned */
    #pragma acc parallel num_gangs(4) num_workers(2) copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                /* Conditional to prevent optimization */
                if (j < n * 50) {
                    dst[i][j] = src[i][j] / 2;
                } else {
                    dst[i][j] = src[i][j];
                }
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    /* Vector partitioned - vector length only */
    #pragma acc parallel vector_length(32) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop vector
        for (i = 0; i < SIZE; i++) {
            /* Runtime-dependent inner loop bound */
            int limit = (i % 3 == 0) ? n * 20 : SIZE;
            for (j = 0; j < limit; j++) {
                dst[i][j] = src[i][j] - i;
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    /* Gang+vector partitioned */
    #pragma acc parallel num_gangs(8) vector_length(64) copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            /* Runtime-dependent inner loop bound */
            int limit = SIZE - (n * 5);
            #pragma acc loop vector
            for (j = 0; j < limit; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    /* Worker+vector partitioned */
    #pragma acc parallel num_workers(2) vector_length(128) copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            /* Runtime-dependent inner loop bound */
            int limit = (n > 1) ? SIZE : SMALL_SIZE;
            #pragma acc loop vector
            for (j = 0; j < limit; j++) {
                dst[i][j] = src[i][j] + 100;
            }
        }
    }
}

/* Function 8: Fully partitioned (gang+worker+vector) */
void compute_fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j, k;
    /* Fully partitioned - all three levels specified */
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                /* Runtime-dependent innermost loop */
                int limit = (i + j) % (n + 10);
                #pragma acc loop vector
                for (k = 0; k < limit; k++) {
                    /* Simple computation with side effect */
                    dst[i][j] += src[i][j] * k;
                }
            }
        }
    }
}

/* Variant using kernels construct */
#ifdef VARIANT_KERNELS
void compute_kernels_variant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    /* Using kernels with different partitioning */
    #pragma acc kernels num_gangs(8) num_workers(1) vector_length(32) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            int limit = (n % 2 == 0) ? SIZE : SMALL_SIZE;
            #pragma acc loop worker
            for (j = 0; j < limit; j++) {
                dst[i][j] = src[i][j] * 5;
            }
        }
    }
}
#endif

/* Variant with collapsed loops */
#ifdef VARIANT_COLLAPSE
void compute_collapsed(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    /* Collapsed loops with vector partitioning */
    #pragma acc parallel vector_length(64) copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop collapse(2) vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                if (j < n * 30) {
                    dst[i][j] = src[i][j] * 7;
                }
            }
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int i, j, n;
    volatile int checksum = 0; /* volatile to prevent optimization */
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = (i * 3 + j * 7) % 100;
        }
    }
    
    /* Read runtime parameter for loop bounds */
    if (argc > 1) {
        n = atoi(argv[1]) % 4; /* Ensure n is 0-3 */
    } else {
        printf("Enter a number (0-3): ");
        scanf("%d", &n);
        n = n % 4;
    }
    
    /* Clear destination array */
    memset(dst, 0, sizeof(dst));
    
    /* Call different functions based on n to trigger various partitioning cases */
    switch (n) {
        case 0:
            compute_gang_redundant(src, dst, n);
            break;
        case 1:
            compute_gang_partitioned(src, dst, n);
            break;
        case 2:
            compute_worker_partitioned(src, dst, n);
            break;
        case 3:
            compute_gang_worker_partitioned(src, dst, n);
            break;
    }
    
    /* Always call the fully partitioned version for additional coverage */
    compute_fully_partitioned(src, dst, n);
    
#ifdef VARIANT_KERNELS
    compute_kernels_variant(src, dst, n);
#endif
    
#ifdef VARIANT_COLLAPSE
    compute_collapsed(src, dst, n);
#endif
    
    /* Additional calls to cover all partitioning types */
    compute_vector_partitioned(src, dst, n);
    compute_gang_vector_partitioned(src, dst, n);
    compute_worker_vector_partitioned(src, dst, n);
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < SIZE; i += 64) {
        for (j = 0; j < SIZE; j += 64) {
            checksum += dst[i][j];
        }
    }
    
    /* Print result to create side effect */
    printf("Checksum: %d (n=%d)\n", checksum, n);
    
    return 0;
}
