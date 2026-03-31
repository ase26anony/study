#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define BLOCK 32

/* Variant 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Variant 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

/* Variant 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

/* Variant 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Variant 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Variant 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] | 0xFF;
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] & 0x7F;
            }
        }
    }
}

/* Variant 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + i - j;
            }
        }
    }
}

/* Alternative implementation using kernels construct */
#ifdef USE_KERNELS
void compute_kernels_variant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * 3;
            }
        }
    }
}
#endif

/* Function with runtime-dependent loop bounds */
void compute_dynamic_bounds(int rows, int cols, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    
    /* Runtime-dependent bounds force dynamic partitioning analysis */
    #pragma acc parallel num_gangs(rows/16) num_workers(2) vector_length(32) \
        copyin(src[0:rows][0:cols]) copyout(dst[0:rows][0:cols])
    {
        #pragma acc loop gang
        for (i = 0; i < rows; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < cols; j++) {
                dst[i][j] = src[i][j] * (i + j);
            }
        }
    }
}

/* Checksum function to prevent optimization */
volatile int checksum_result;
int compute_checksum(int arr[SIZE][SIZE], int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < SIZE; j++) {
            sum += arr[i][j];
        }
    }
    checksum_result = sum;  /* Volatile write creates side effect */
    return sum;
}

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int variant = 1;
    int dynamic_size = 256;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
        }
    }
    
    /* Read variant from input or command line */
    if (argc > 1) {
        variant = atoi(argv[1]);
    } else {
        printf("Enter variant (1-8): ");
        scanf("%d", &variant);
    }
    
    /* Read dynamic size for runtime-dependent bounds */
    printf("Enter size for dynamic computation (1-1024): ");
    scanf("%d", &dynamic_size);
    if (dynamic_size > SIZE) dynamic_size = SIZE;
    if (dynamic_size < 1) dynamic_size = 1;
    
    /* Clear destination array */
    memset(dst, 0, sizeof(dst));
    
    /* Select variant based on input */
    switch (variant) {
        case 1:
            compute_gang_redundant(dynamic_size, src, dst);
            break;
        case 2:
            compute_gang_partitioned(dynamic_size, src, dst);
            break;
        case 3:
            compute_worker_partitioned(dynamic_size, src, dst);
            break;
        case 4:
            compute_gang_worker_partitioned(dynamic_size, src, dst);
            break;
        case 5:
            compute_vector_partitioned(dynamic_size, src, dst);
            break;
        case 6:
            compute_gang_vector_partitioned(dynamic_size, src, dst);
            break;
        case 7:
            compute_worker_vector_partitioned(dynamic_size, src, dst);
            break;
        case 8:
            compute_fully_partitioned(dynamic_size, src, dst);
            break;
        default:
            printf("Invalid variant, using default (fully partitioned)\n");
            compute_fully_partitioned(dynamic_size, src, dst);
            break;
    }
    
    /* Also call the dynamic bounds function to ensure runtime analysis */
    compute_dynamic_bounds(dynamic_size, dynamic_size, src, dst);
    
    #ifdef USE_KERNELS
    compute_kernels_variant(dynamic_size, src, dst);
    #endif
    
    /* Compute and print checksum to prevent optimization */
    int sum = compute_checksum(dst, dynamic_size);
    printf("Checksum: %d\n", sum);
    
    /* Additional printf to create side effects */
    printf("Computation complete for variant %d with size %d\n", variant, dynamic_size);
    
    return 0;
}
