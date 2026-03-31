/* OpenACC program designed to trigger partitioning classification logic
   in GCC's omp-oacc-neuter-broadcast.cc, specifically lines 335-343. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SMALL_SIZE 256

/* Variant 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang redundant - only 1 gang */
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Variant 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang partitioned - multiple gangs, 1 worker, 1 vector */
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i + j;
            }
        }
    }
}

/* Variant 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Worker partitioned - 1 gang, multiple workers */
    #pragma acc kernels num_gangs(1) num_workers(4) vector_length(1) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Variant 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang+worker partitioned */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / (i + j + 1);
            }
        }
    }
}

/* Variant 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Vector partitioned - 1 gang, 1 worker, multiple vectors */
    #pragma acc kernels num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | 0x1;
            }
        }
    }
}

/* Variant 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang+vector partitioned */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] & 0xFF;
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Worker+vector partitioned */
    #pragma acc kernels num_gangs(1) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] ^ (i * j);
            }
        }
    }
}

/* Variant 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Fully partitioned - all dimensions partitioned */
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3 - (i + j);
            }
        }
    }
}

/* Function with conditional compilation for different partitioning */
void compute_variant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE], int variant) {
#ifdef VARIANT_A
    if (variant == 0) {
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(32) \
            copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
        {
            #pragma acc loop gang collapse(2)
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    dst[i][j] = src[i][j] + 1;
                }
            }
        }
    }
#elif defined(VARIANT_B)
    if (variant == 1) {
        #pragma acc kernels num_gangs(1) num_workers(8) vector_length(1) \
            copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop worker
                for (int j = 0; j < n; j++) {
                    dst[i][j] = src[i][j] * 2;
                }
            }
        }
    }
#else
    if (variant == 2) {
        #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
            copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
        {
            #pragma acc loop gang
            for (int i = 0; i < n; i++) {
                #pragma acc loop vector
                for (int j = 0; j < n; j++) {
                    dst[i][j] = src[i][j] - (i - j);
                }
            }
        }
    }
#endif
}

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int i, j, n;
    volatile int checksum = 0;
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = (i * 17 + j * 13) % 100;
        }
    }
    
    /* Read runtime-dependent size */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SMALL_SIZE;
    } else {
        printf("Enter size (1-%d): ", SIZE);
        if (scanf("%d", &n) != 1) n = SMALL_SIZE;
        if (n <= 0 || n > SIZE) n = SMALL_SIZE;
    }
    
    /* Call different functions to trigger various partitioning scenarios */
    compute_gang_redundant(n, src, dst);
    
    /* Use volatile to prevent optimization */
    volatile int *volatile_dst = &dst[0][0];
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    compute_gang_partitioned(n, src, dst);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    compute_worker_partitioned(n, src, dst);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    compute_gang_worker_partitioned(n, src, dst);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    compute_vector_partitioned(n, src, dst);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    compute_gang_vector_partitioned(n, src, dst);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    compute_worker_vector_partitioned(n, src, dst);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    compute_fully_partitioned(n, src, dst);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    /* Call variant function with different preprocessor definitions */
    compute_variant(n, src, dst, 0);
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    /* Print checksum to create side effect */
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
