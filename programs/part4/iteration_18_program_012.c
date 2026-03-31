/* OpenACC program designed to trigger partitioning analysis in omp-oacc-neuter-broadcast.cc
 * Specifically targets the switch statement cases 0-7 for different partitioning types.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SMALL_SIZE 128

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i + j;
            }
        }
    }
}

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / (i + j + 1);
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop seq
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - (i * j);
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang vector
        for (i = 0; i < n; i++) {
            #pragma acc loop seq
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | (i << 4);
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker vector
        for (i = 0; i < n; i++) {
            #pragma acc loop seq
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] & (j << 2);
            }
        }
    }
}

/* Function 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++) {
            #pragma acc loop seq
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] ^ (i * j);
            }
        }
    }
}

/* Variant using kernels construct */
#ifdef VARIANT_KERNELS
void compute_kernels_variant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3;
            }
        }
    }
}
#endif

/* Variant with collapsed loops */
#ifdef VARIANT_COLLAPSE
void compute_collapsed(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector collapse(2)
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + 100;
            }
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int i, j, n;
    volatile int checksum = 0;
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
        }
    }
    
    /* Read runtime-dependent size */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SMALL_SIZE) n = SMALL_SIZE;
    } else {
        printf("Enter size (1-%d): ", SMALL_SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SMALL_SIZE) n = SMALL_SIZE;
    }
    
    /* Clear destination array */
    memset(dst, 0, sizeof(dst));
    
    /* Call different functions based on input to trigger various partitioning cases */
    switch (n % 8) {
        case 0: compute_gang_redundant(n, src, dst); break;
        case 1: compute_gang_partitioned(n, src, dst); break;
        case 2: compute_worker_partitioned(n, src, dst); break;
        case 3: compute_gang_worker_partitioned(n, src, dst); break;
        case 4: compute_vector_partitioned(n, src, dst); break;
        case 5: compute_gang_vector_partitioned(n, src, dst); break;
        case 6: compute_worker_vector_partitioned(n, src, dst); break;
        case 7: compute_fully_partitioned(n, src, dst); break;
    }
    
#ifdef VARIANT_KERNELS
    compute_kernels_variant(n, src, dst);
#endif
    
#ifdef VARIANT_COLLAPSE
    compute_collapsed(n, src, dst);
#endif
    
    /* Calculate checksum to prevent optimization */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
