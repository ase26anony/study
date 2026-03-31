#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define BLOCK 32

// Function prototypes
void gang_redundant_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void fully_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int variant = 1;
    
    // Read variant from command line or stdin
    if (argc > 1) {
        variant = atoi(argv[1]);
    } else {
        printf("Enter variant (1-4): ");
        scanf("%d", &variant);
    }
    
    // Initialize arrays with pattern
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
            dst[i][j] = 0;
        }
    }
    
    // Runtime-dependent loop bounds
    int dynamic_bound = SIZE;
    if (variant % 2 == 0) {
        dynamic_bound = SIZE / 2;
    }
    
    // Call different functions based on variant
    switch (variant) {
        case 1:
            gang_redundant_compute(dynamic_bound, src, dst);
            break;
        case 2:
            gang_partitioned_compute(dynamic_bound, src, dst);
            break;
        case 3:
            worker_vector_partitioned(dynamic_bound, src, dst);
            break;
        case 4:
            fully_partitioned_compute(dynamic_bound, src, dst);
            break;
        default:
            gang_redundant_compute(SIZE, src, dst);
    }
    
    // Validation checksum
    volatile long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    return 0;
}

// Case 0: gang redundant
void gang_redundant_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    // Single gang, redundant across gangs
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

// Case 1: gang partitioned
void gang_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    // Multiple gangs, each gets portion of work
    #pragma acc kernels num_gangs(8) copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #ifdef VARIANT_A
            #pragma acc loop worker
            #else
            #pragma acc loop seq
            #endif
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

// Case 2: worker partitioned
void worker_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    // Single gang, workers partitioned
    #pragma acc parallel num_gangs(1) num_workers(4) \
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

// Case 3: gang+worker partitioned
void gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    // Both gangs and workers partitioned
    #pragma acc parallel num_gangs(4) num_workers(2) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
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

// Case 4: vector partitioned
void vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    // Vector-level partitioning
    #pragma acc parallel vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

// Case 5: gang+vector partitioned
void gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    // Gangs and vectors partitioned
    #pragma acc parallel num_gangs(4) vector_length(64) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #ifdef VARIANT_B
            #pragma acc loop worker vector
            #else
            #pragma acc loop vector
            #endif
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] | 0xFF;
            }
        }
    }
}

// Case 6: worker+vector partitioned (called from main as case 3)
void worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    // Workers and vectors partitioned
    #pragma acc parallel num_workers(4) vector_length(32) \
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

// Case 7: fully partitioned
void fully_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    // All three levels partitioned
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop worker
            for (int j = 0; j < SIZE; j++) {
                #pragma acc loop vector
                for (int k = 0; k < BLOCK; k++) {
                    int idx = j * BLOCK + k;
                    if (idx < SIZE) {
                        dst[i][idx] = src[i][idx] + i + j + k;
                    }
                }
            }
        }
    }
}
