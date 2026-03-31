/* 
 * OpenACC Partitioning Test Program
 * Designed to trigger all partitioning classification cases in omp-oacc-neuter-broadcast.cc
 * Compile with: gcc -O1 -fopenacc -foffload=disable -fdump-tree-oaccloops
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SMALL_SIZE 256

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
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

/* Function 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
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

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3 - j;
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
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + (i * j);
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
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] << 1;
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
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / 2;
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
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | 0x01;
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
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
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
                dst[i][j] = src[i][j] - 5;
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
        #pragma acc loop gang worker collapse(2)
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + 100;
            }
        }
    }
}
#endif

/* Initialize arrays with pattern */
void init_arrays(int src[SIZE][SIZE]) {
    int i, j;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
        }
    }
}

/* Compute checksum to prevent optimization */
int compute_checksum(int arr[SIZE][SIZE], int n) {
    int sum = 0;
    volatile int prevent_opt = 0; /* volatile to prevent optimization */
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            sum += arr[i][j];
            prevent_opt = arr[i][j]; /* Side effect */
        }
    }
    
    printf("Checksum: %d\n", sum);
    return sum;
}

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int n = SMALL_SIZE; /* Default size */
    
    /* Read runtime-dependent size */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SMALL_SIZE;
    } else {
        printf("Enter matrix dimension (1-%d): ", SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SIZE) n = SMALL_SIZE;
    }
    
    /* Initialize source array */
    init_arrays(src);
    memset(dst, 0, sizeof(dst));
    
    printf("Testing with n = %d\n", n);
    
    /* Call different functions to trigger various partitioning cases */
    compute_gang_redundant(n, src, dst);
    compute_checksum(dst, n);
    
    memset(dst, 0, sizeof(dst));
    compute_gang_partitioned(n, src, dst);
    compute_checksum(dst, n);
    
    memset(dst, 0, sizeof(dst));
    compute_worker_partitioned(n, src, dst);
    compute_checksum(dst, n);
    
    memset(dst, 0, sizeof(dst));
    compute_gang_worker_partitioned(n, src, dst);
    compute_checksum(dst, n);
    
    memset(dst, 0, sizeof(dst));
    compute_vector_partitioned(n, src, dst);
    compute_checksum(dst, n);
    
    memset(dst, 0, sizeof(dst));
    compute_gang_vector_partitioned(n, src, dst);
    compute_checksum(dst, n);
    
    memset(dst, 0, sizeof(dst));
    compute_worker_vector_partitioned(n, src, dst);
    compute_checksum(dst, n);
    
    memset(dst, 0, sizeof(dst));
    compute_fully_partitioned(n, src, dst);
    compute_checksum(dst, n);
    
    /* Conditional compilation variants */
#ifdef VARIANT_KERNELS
    memset(dst, 0, sizeof(dst));
    compute_kernels_variant(n, src, dst);
    compute_checksum(dst, n);
#endif

#ifdef VARIANT_COLLAPSE
    memset(dst, 0, sizeof(dst));
    compute_collapsed(n, src, dst);
    compute_checksum(dst, n);
#endif
    
    return 0;
}
