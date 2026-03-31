#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define BLOCK 32

// Function prototypes for different partitioning scenarios
void gang_redundant_partition(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n);
void gang_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n);
void worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n);
void gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n);
void vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n);
void gang_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n);
void worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n);
void fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n);

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int n = 1;
    
    // Initialize arrays with pattern-based data
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
            dst[i][j] = 0;
        }
    }
    
    // Read partitioning mode from input or command line
    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        printf("Enter partitioning mode (0-7): ");
        scanf("%d", &n);
    }
    
    // Call different functions based on mode to trigger different partitioning cases
    switch (n % 8) {
        case 0:
            gang_redundant_partition(src, dst, n);
            break;
        case 1:
            gang_partitioned(src, dst, n);
            break;
        case 2:
            worker_partitioned(src, dst, n);
            break;
        case 3:
            gang_worker_partitioned(src, dst, n);
            break;
        case 4:
            vector_partitioned(src, dst, n);
            break;
        case 5:
            gang_vector_partitioned(src, dst, n);
            break;
        case 6:
            worker_vector_partitioned(src, dst, n);
            break;
        case 7:
            fully_partitioned(src, dst, n);
            break;
    }
    
    // Validation checksum to prevent optimization
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

// Case 0: gang redundant
void gang_redundant_partition(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    // Runtime-dependent loop bounds
    int rows = SIZE - (n % 100);
    int cols = SIZE - (n % 50);
    
    #pragma acc kernels num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #ifdef VARIANT1
        #pragma acc loop independent
        #endif
        for (int i = 0; i < rows; i++) {
            #ifdef VARIANT2
            #pragma acc loop vector(32)
            #endif
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

// Case 1: gang partitioned
void gang_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int rows = SIZE - (n % 64);
    int cols = SIZE - (n % 32);
    
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop seq
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] + i + j;
            }
        }
    }
}

// Case 2: worker partitioned
void worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int rows = SIZE - (n % 128);
    int cols = SIZE - (n % 64);
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (int i = 0; i < rows; i++) {
            #pragma acc loop seq
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

// Case 3: gang+worker partitioned
void gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int rows = SIZE - (n % 96);
    int cols = SIZE - (n % 48);
    
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #ifdef VARIANT1
        #pragma acc loop gang worker
        #else
        #pragma acc loop gang
        #endif
        for (int i = 0; i < rows; i++) {
            #ifdef VARIANT1
            #pragma acc loop seq
            #else
            #pragma acc loop worker
            #endif
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] / (1 + (i % 16));
            }
        }
    }
}

// Case 4: vector partitioned
void vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int rows = SIZE - (n % 256);
    int cols = SIZE - (n % 128);
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop seq
        for (int i = 0; i < rows; i++) {
            #pragma acc loop vector
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] - (j % 8);
            }
        }
    }
}

// Case 5: gang+vector partitioned
void gang_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int rows = SIZE - (n % 192);
    int cols = SIZE - (n % 96);
    
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop vector
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] | (i & 0xFF);
            }
        }
    }
}

// Case 6: worker+vector partitioned
void worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int rows = SIZE - (n % 160);
    int cols = SIZE - (n % 80);
    
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(8) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (int i = 0; i < rows; i++) {
            #pragma acc loop vector
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] & (j % 16);
            }
        }
    }
}

// Case 7: fully partitioned
void fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int rows = SIZE - (n % 224);
    int cols = SIZE - (n % 112);
    
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #ifdef VARIANT1
        #pragma acc loop gang worker vector
        #else
        #pragma acc loop gang
        #endif
        for (int i = 0; i < rows; i++) {
            #ifdef VARIANT1
            #pragma acc loop seq
            #else
            #pragma acc loop worker vector
            #endif
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] * 3 + (i * j) % 256;
            }
        }
    }
}
