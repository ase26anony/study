#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define BLOCK 32

// Function 1: Gang redundant partitioning
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

// Function 2: Gang partitioned
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

// Function 3: Worker partitioned
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc kernels num_gangs(1) num_workers(4) vector_length(1) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

// Function 4: Gang+worker partitioned
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

// Function 5: Vector partitioned
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc kernels num_gangs(1) num_workers(1) vector_length(64) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

// Function 6: Gang+vector partitioned
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] | 0xFF;
            }
        }
    }
}

// Function 7: Worker+vector partitioned
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc kernels num_gangs(1) num_workers(2) vector_length(64) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] & 0x7F;
            }
        }
    }
}

// Function 8: Fully partitioned
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * 3 + j;
            }
        }
    }
}

// Variant with different partitioning using conditional compilation
#ifdef VARIANT
void compute_variant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(2) num_workers(8) vector_length(16) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < n; i++) {
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] ^ 0xAA;
            }
        }
    }
}
#endif

// Another variant with collapsed loops
#ifdef VARIANT2
void compute_collapsed(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang collapse(2)
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] << 1;
            }
        }
    }
}
#endif

// Helper function for checksum
int calculate_checksum(int arr[SIZE][SIZE], int n) {
    int sum = 0;
    volatile int force_compute = 1; // Prevent optimization
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < SIZE; j++) {
            if (force_compute) {
                sum += arr[i][j];
            }
        }
    }
    return sum;
}

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int n = 256; // Default value
    
    // Read runtime-dependent loop bound
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = 256;
    } else {
        printf("Enter loop bound (1-%d): ", SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SIZE) n = 256;
    }
    
    // Initialize source array with pattern
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
        }
    }
    
    // Clear destination array
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            dst[i][j] = 0;
        }
    }
    
    int mode = n % 8; // Use input to select different partitioning modes
    
    // Call different functions based on mode to trigger various partitioning cases
    switch (mode) {
        case 0:
            compute_gang_redundant(n, src, dst);
            break;
        case 1:
            compute_gang_partitioned(n, src, dst);
            break;
        case 2:
            compute_worker_partitioned(n, src, dst);
            break;
        case 3:
            compute_gang_worker_partitioned(n, src, dst);
            break;
        case 4:
            compute_vector_partitioned(n, src, dst);
            break;
        case 5:
            compute_gang_vector_partitioned(n, src, dst);
            break;
        case 6:
            compute_worker_vector_partitioned(n, src, dst);
            break;
        case 7:
            compute_fully_partitioned(n, src, dst);
            break;
    }
    
#ifdef VARIANT
    // Additional variant call
    compute_variant(n, src, dst);
#endif
    
#ifdef VARIANT2
    // Another variant call
    compute_collapsed(n, src, dst);
#endif
    
    // Final validation with checksum
    int checksum = calculate_checksum(dst, n);
    printf("Checksum: %d\n", checksum);
    
    // Force side effects
    volatile int dummy = checksum;
    
    return 0;
}
