#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define BLOCK 32

// Function 1: Gang redundant partitioning
void compute_gang_redundant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i += n) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j += n) {
                int sum = 0;
                for (int k = 0; k < n && (i + k) < SIZE; k++) {
                    for (int l = 0; l < n && (j + l) < SIZE; l++) {
                        sum += src[i + k][j + l];
                    }
                }
                dst[i][j] = sum;
            }
        }
    }
}

// Function 2: Gang partitioned
void compute_gang_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                int val = src[i][j];
                // Runtime-dependent computation
                for (int k = 0; k < (n % 8); k++) {
                    val = (val * 3 + 1) % 256;
                }
                dst[i][j] = val;
            }
        }
    }
}

// Function 3: Worker partitioned
void compute_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * n;
            }
        }
    }
}

// Function 4: Gang+worker partitioned
void compute_gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                int temp = src[i][j];
                for (int k = 0; k < (n % 4); k++) {
                    temp = temp ^ (temp >> 1);
                }
                dst[i][j] = temp;
            }
        }
    }
}

// Function 5: Vector partitioned
void compute_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + (j % n);
            }
        }
    }
}

// Function 6: Gang+vector partitioned
void compute_gang_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * (i % n) + (j % n);
            }
        }
    }
}

// Function 7: Worker+vector partitioned
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                int val = src[i][j];
                #ifdef VARIANT_A
                val = val * 2 + n;
                #else
                val = val / (n + 1);
                #endif
                dst[i][j] = val;
            }
        }
    }
}

// Function 8: Fully partitioned
void compute_fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                #ifdef VARIANT_B
                dst[i][j] = src[i][j] + i * n + j;
                #else
                dst[i][j] = src[i][j] - i * n + j;
                #endif
            }
        }
    }
}

// Alternative using kernels construct
void compute_kernels_variant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                int sum = 0;
                for (int k = 0; k < (n % 5); k++) {
                    sum += src[i][j] >> k;
                }
                dst[i][j] = sum;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    
    // Initialize source array with pattern
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = (i * 17 + j * 13) % 256;
        }
    }
    
    // Read partitioning mode from input
    int mode = 0;
    if (argc > 1) {
        mode = atoi(argv[1]) % 8;
    } else {
        printf("Enter partitioning mode (0-7): ");
        scanf("%d", &mode);
        mode = mode % 8;
    }
    
    // Runtime-dependent parameter
    int n = (mode * 7 + 11) % 16 + 1;
    
    // Select computation based on mode
    switch (mode) {
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
        case 4:
            compute_vector_partitioned(src, dst, n);
            break;
        case 5:
            compute_gang_vector_partitioned(src, dst, n);
            break;
        case 6:
            compute_worker_vector_partitioned(src, dst, n);
            break;
        case 7:
            compute_fully_partitioned(src, dst, n);
            break;
    }
    
    // Also call kernels variant for additional coverage
    if (mode % 2 == 0) {
        compute_kernels_variant(src, dst, n);
    }
    
    // Validation checksum with side effects
    volatile long checksum = 0;
    for (int i = 0; i < SIZE; i += 64) {
        for (int j = 0; j < SIZE; j += 64) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
