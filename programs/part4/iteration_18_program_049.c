#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define BLOCK 32

/* Function prototypes for different partitioning scenarios */
void gang_redundant_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void worker_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_worker_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void worker_vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void fully_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int variant = 1;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
            dst[i][j] = 0;
        }
    }
    
    /* Read variant from input or command line */
    if (argc > 1) {
        variant = atoi(argv[1]);
    } else {
        printf("Enter variant (1-8): ");
        scanf("%d", &variant);
    }
    
    /* Runtime-dependent selection of partitioning function */
    switch (variant) {
        case 1:
            gang_redundant_compute(variant, src, dst);
            break;
        case 2:
            gang_partitioned_compute(variant, src, dst);
            break;
        case 3:
            worker_partitioned_compute(variant, src, dst);
            break;
        case 4:
            gang_worker_partitioned_compute(variant, src, dst);
            break;
        case 5:
            vector_partitioned_compute(variant, src, dst);
            break;
        case 6:
            gang_vector_partitioned_compute(variant, src, dst);
            break;
        case 7:
            worker_vector_partitioned_compute(variant, src, dst);
            break;
        case 8:
            fully_partitioned_compute(variant, src, dst);
            break;
        default:
            gang_redundant_compute(1, src, dst);
            break;
    }
    
    /* Validation checksum to prevent optimization */
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Case 0: gang redundant */
void gang_redundant_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    /* Runtime-dependent loop bounds */
    int rows = SIZE;
    int cols = SIZE;
    if (n > 0) {
        rows = SIZE / n;
        cols = SIZE / n;
    }
    
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop worker
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Case 1: gang partitioned */
void gang_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int rows = SIZE;
    int cols = SIZE;
    if (n > 0) {
        rows = SIZE / (n % 8 + 1);
        cols = SIZE / (n % 8 + 1);
    }
    
    #pragma acc kernels num_gangs(8) copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
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

/* Case 2: worker partitioned */
void worker_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int rows = SIZE;
    int cols = SIZE;
    if (n > 0) {
        rows = SIZE / (n % 4 + 1);
        cols = SIZE / (n % 4 + 1);
    }
    
    #pragma acc parallel num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop worker
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] * 3 - j;
            }
        }
    }
}

/* Case 3: gang+worker partitioned */
void gang_worker_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int rows = SIZE;
    int cols = SIZE;
    if (n > 0) {
        rows = SIZE / (n % 16 + 1);
        cols = SIZE / (n % 16 + 1);
    }
    
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop worker
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] + (i * j) % 256;
            }
        }
    }
}

/* Case 4: vector partitioned */
void vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int rows = SIZE;
    int cols = SIZE;
    if (n > 0) {
        rows = SIZE / (n % 32 + 1);
        cols = SIZE / (n % 32 + 1);
    }
    
    #pragma acc kernels vector_length(32) copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop vector
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] << 1;
            }
        }
    }
}

/* Case 5: gang+vector partitioned */
void gang_vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int rows = SIZE;
    int cols = SIZE;
    if (n > 0) {
        rows = SIZE / (n % 64 + 1);
        cols = SIZE / (n % 64 + 1);
    }
    
    #pragma acc parallel num_gangs(8) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop vector
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Case 6: worker+vector partitioned */
void worker_vector_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int rows = SIZE;
    int cols = SIZE;
    if (n > 0) {
        rows = SIZE / (n % 128 + 1);
        cols = SIZE / (n % 128 + 1);
    }
    
    #pragma acc parallel num_workers(4) vector_length(64) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] | 0xFF;
            }
        }
    }
}

/* Case 7: fully partitioned */
void fully_partitioned_compute(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int rows = SIZE;
    int cols = SIZE;
    if (n > 0) {
        rows = SIZE / (n % 256 + 1);
        cols = SIZE / (n % 256 + 1);
    }
    
    /* Conditional compilation for path variation */
    #ifdef VARIANT_A
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    #else
    #pragma acc kernels num_gangs(4) num_workers(4) vector_length(16) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    #endif
    {
        #ifdef VARIANT_A
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
        #else
        #pragma acc loop gang
        for (int i = 0; i < rows; i++) {
            #pragma acc loop worker
            for (int j = 0; j < cols; j++) {
                #pragma acc loop vector
                for (int k = 0; k < BLOCK; k++) {
                    dst[i][j] += src[i][j] * k;
                }
            }
        }
        #endif
    }
}
