/* This program is designed to trigger the partitioning classification logic
   in GCC's OpenACC neutering/broadcast analysis, specifically targeting the
   switch statement that returns descriptive strings for different partitioning
   types (cases 0-7). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define ITER 8

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Case 0: gang redundant - only num_gangs(1) specified */
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
    /* Case 1: gang partitioned - only num_gangs specified */
    #pragma acc parallel num_gangs(8) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Case 2: worker partitioned - only num_workers specified */
    #pragma acc parallel num_workers(4) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(4) num_workers(2) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Case 4: vector partitioned - only vector_length specified */
    #pragma acc parallel vector_length(32) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Case 5: gang+vector partitioned */
    #pragma acc parallel num_gangs(8) vector_length(64) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang vector
        for (i = 0; i < n; i++) {
            #pragma acc loop seq
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | 0xFF;
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(4) vector_length(128) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker vector
        for (i = 0; i < n; i++) {
            #pragma acc loop seq
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] & 0x7F;
            }
        }
    }
}

/* Function 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Case 7: fully partitioned - all three clauses specified */
    #pragma acc parallel num_gangs(16) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++) {
            #pragma acc loop seq
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i + j;
            }
        }
    }
}

/* Main computational kernel with conditional compilation for path variation */
#ifdef VARIANT_A
void compute_kernel(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE], int mode) {
    if (mode == 0) {
        #pragma acc parallel num_gangs(8) num_workers(1) vector_length(32) \
            copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                #pragma acc loop vector
                for (int j = 0; j < n; j++) {
                    dst[i][j] = src[i][j] * 3;
                }
            }
        }
    } else {
        #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) \
            copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
        {
            #pragma acc loop gang worker
            for (int i = 0; i < n; i++) {
                #pragma acc loop vector
                for (int j = 0; j < n; j++) {
                    dst[i][j] = src[i][j] + 5;
                }
            }
        }
    }
}
#elif defined(VARIANT_B)
void compute_kernel(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE], int mode) {
    #pragma acc kernels num_gangs(mode > 0 ? 8 : 1) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - (i * j);
            }
        }
    }
}
#else
void compute_kernel(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE], int mode) {
    #pragma acc parallel num_gangs(4) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < n; j++) {
                #pragma acc loop vector
                for (int k = 0; k < ITER; k++) {
                    dst[i][j] += src[i][j] * k;
                }
            }
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int i, j, n, mode;
    volatile int checksum = 0;
    
    /* Initialize arrays with pattern-based data */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
            dst[i][j] = 0;
        }
    }
    
    /* Read N from input for runtime-dependent loop bounds */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE;
    } else {
        printf("Enter matrix dimension (1-%d): ", SIZE);
        if (scanf("%d", &n) != 1) n = SIZE;
        if (n <= 0 || n > SIZE) n = SIZE;
    }
    
    /* Read mode for variant selection */
    if (argc > 2) {
        mode = atoi(argv[2]);
    } else {
        printf("Enter mode (0-7 for different partitioning types): ");
        if (scanf("%d", &mode) != 1) mode = 0;
        mode = mode % 8;
    }
    
    /* Call different functions based on mode to trigger various partitioning cases */
    switch (mode) {
        case 0: compute_gang_redundant(n, src, dst); break;
        case 1: compute_gang_partitioned(n, src, dst); break;
        case 2: compute_worker_partitioned(n, src, dst); break;
        case 3: compute_gang_worker_partitioned(n, src, dst); break;
        case 4: compute_vector_partitioned(n, src, dst); break;
        case 5: compute_gang_vector_partitioned(n, src, dst); break;
        case 6: compute_worker_vector_partitioned(n, src, dst); break;
        case 7: compute_fully_partitioned(n, src, dst); break;
    }
    
    /* Also call the conditional kernel */
    compute_kernel(n, src, dst, mode);
    
    /* Validation step with checksum to prevent optimization */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Force side-effect with printf */
    printf("Computation completed with n=%d, mode=%d\n", n, mode);
    
    return 0;
}
