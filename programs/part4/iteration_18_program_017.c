#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define BLOCK 32

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] | 0xFF;
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] & 0x7F;
            }
        }
    }
}

/* Function 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] ^ 0x55;
            }
        }
    }
}

/* Variant using kernels construct */
#ifdef VARIANT_KERNELS
void compute_kernels_variant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * 3;
            }
        }
    }
}
#endif

/* Variant with collapsed loops */
#ifdef VARIANT_COLLAPSE
void compute_collapsed(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(8) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + src[i][j];
            }
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    
    /* Initialize with pattern-based data */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = (i * 17 + j * 13) % 256;
        }
    }
    
    /* Read N from input for runtime-dependent bounds */
    int N;
    if (argc > 1) {
        N = atoi(argv[1]);
    } else {
        printf("Enter N (1-512): ");
        scanf("%d", &N);
    }
    
    /* Ensure N is within bounds */
    if (N < 1) N = 1;
    if (N > 512) N = 512;
    
    /* Call different functions based on N to trigger various partitioning cases */
    volatile int checksum = 0;
    
    if (N % 8 == 0) {
        compute_gang_redundant(N, src, dst);
    } else if (N % 8 == 1) {
        compute_gang_partitioned(N, src, dst);
    } else if (N % 8 == 2) {
        compute_worker_partitioned(N, src, dst);
    } else if (N % 8 == 3) {
        compute_gang_worker_partitioned(N, src, dst);
    } else if (N % 8 == 4) {
        compute_vector_partitioned(N, src, dst);
    } else if (N % 8 == 5) {
        compute_gang_vector_partitioned(N, src, dst);
    } else if (N % 8 == 6) {
        compute_worker_vector_partitioned(N, src, dst);
    } else {
        compute_fully_partitioned(N, src, dst);
    }
    
#ifdef VARIANT_KERNELS
    /* Alternate path using kernels construct */
    compute_kernels_variant(N / 2, src, dst);
#endif
    
#ifdef VARIANT_COLLAPSE
    /* Alternate path with collapsed loops */
    compute_collapsed(N, src, dst);
#endif
    
    /* Validation checksum to prevent optimization */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < SIZE; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Force side effects */
    volatile int dummy = checksum;
    
    return 0;
}
