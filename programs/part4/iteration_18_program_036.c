/* 
 * OpenACC Partitioning Test Program
 * Designed to trigger all cases in omp-oacc-neuter-broadcast.cc lines 335-343
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SMALL_SIZE 256

/* Variant 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 0: gang redundant - only num_gangs(1) */
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            /* Runtime-dependent inner loop bound forces partitioning analysis */
            for (j = 0; j < n * 32; j++) {
                if (j < SIZE) {
                    dst[i][j] = src[i][j] * 2;
                }
            }
        }
    }
}

/* Variant 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 1: gang partitioned - only num_gangs specified */
    #pragma acc kernels num_gangs(8) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            /* Nested loops with data-dependent bounds */
            for (j = 0; j < (n % 8) * 128; j++) {
                if (j < SIZE) {
                    dst[i][j] = src[i][j] + i + j;
                }
            }
        }
    }
}

/* Variant 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 2: worker partitioned - only num_workers specified */
    #pragma acc parallel num_workers(4) copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n * 64; j++) {
                if (j < SIZE) {
                    dst[i][j] = src[i][j] * src[i][j];
                }
            }
        }
    }
}

/* Variant 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 3: gang+worker partitioned */
    #pragma acc parallel num_gangs(4) num_workers(2) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < (n + 1) * 128; j++) {
                if (j < SIZE) {
                    dst[i][j] = src[i][j] / (n + 1);
                }
            }
        }
    }
}

/* Variant 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 4: vector partitioned */
    #pragma acc parallel vector_length(32) copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n * 96; j++) {
                if (j < SIZE) {
                    dst[i][j] = src[i][j] - (i + j);
                }
            }
        }
    }
}

/* Variant 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 5: gang+vector partitioned */
    #pragma acc kernels num_gangs(8) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < (n % 4) * 256; j++) {
                if (j < SIZE) {
                    dst[i][j] = src[i][j] | (i * j);
                }
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 6: worker+vector partitioned */
    #pragma acc parallel num_workers(2) vector_length(128) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n * 192; j++) {
                if (j < SIZE) {
                    dst[i][j] = src[i][j] & 0xFF;
                }
            }
        }
    }
}

/* Variant 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Case 7: fully partitioned - all three clauses specified */
    #pragma acc parallel num_gangs(16) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                /* Vector operation inside worker loop */
                #pragma acc loop vector
                for (int k = 0; k < n * 8; k++) {
                    if (k < 16) {
                        dst[i][j] += src[i][j] * k;
                    }
                }
            }
        }
    }
}

/* Conditional compilation for different partitioning scenarios */
#ifdef VARIANT_A
void compute_variant_a(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    /* Switch between gang and worker partitioning based on input */
    if (n % 2 == 0) {
        #pragma acc parallel num_gangs(8) num_workers(1) \
            copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
        {
            #pragma acc loop gang
            for (int i = 0; i < SIZE; i++) {
                for (int j = 0; j < n * 64; j++) {
                    if (j < SIZE) dst[i][j] = src[i][j] + 1;
                }
            }
        }
    } else {
        #pragma acc parallel num_gangs(1) num_workers(8) \
            copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
        {
            #pragma acc loop worker
            for (int i = 0; i < SIZE; i++) {
                for (int j = 0; j < n * 64; j++) {
                    if (j < SIZE) dst[i][j] = src[i][j] - 1;
                }
            }
        }
    }
}
#endif

#ifdef VARIANT_B
void compute_variant_b(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    /* Collapsed loops with different partitioning */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 0; i < SIZE/2; i++) {
            for (int j = 0; j < n * 32; j++) {
                if (j < SIZE/2) {
                    dst[i*2][j*2] = src[i*2][j*2] * 3;
                }
            }
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    /* Initialize arrays with pattern */
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    volatile int checksum = 0; /* volatile to prevent optimization */
    
    /* Initialize source array */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = (i * 17 + j * 13) % 100;
        }
    }
    
    /* Read partitioning mode from input */
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 8;
    } else {
        printf("Enter partitioning mode (0-7): ");
        scanf("%d", &mode);
        mode = mode % 8;
    }
    
    /* Clear destination array */
    memset(dst, 0, sizeof(dst));
    
    /* Select partitioning function based on mode */
    switch (mode) {
        case 0: compute_gang_redundant(mode, src, dst); break;
        case 1: compute_gang_partitioned(mode, src, dst); break;
        case 2: compute_worker_partitioned(mode, src, dst); break;
        case 3: compute_gang_worker_partitioned(mode, src, dst); break;
        case 4: compute_vector_partitioned(mode, src, dst); break;
        case 5: compute_gang_vector_partitioned(mode, src, dst); break;
        case 6: compute_worker_vector_partitioned(mode, src, dst); break;
        case 7: compute_fully_partitioned(mode, src, dst); break;
    }
    
    /* Conditional compilation variants */
    #ifdef VARIANT_A
    compute_variant_a(mode, src, dst);
    #endif
    
    #ifdef VARIANT_B
    compute_variant_b(mode, src, dst);
    #endif
    
    /* Calculate checksum to prevent optimization */
    for (int i = 0; i < SIZE; i += 64) {
        for (int j = 0; j < SIZE; j += 64) {
            checksum += dst[i][j];
        }
    }
    
    /* Print result to create side effect */
    printf("Checksum: %d (mode: %d)\n", checksum, mode);
    
    return 0;
}
