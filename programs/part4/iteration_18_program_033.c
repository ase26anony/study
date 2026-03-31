/* OpenACC program designed to trigger partitioning classification logic
   in GCC's omp-oacc-neuter-broadcast.cc, specifically lines 335-343 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SMALL_SIZE 256

/* Variant 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
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

/* Variant 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
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

/* Variant 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + j;
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
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3;
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
        #pragma acc loop seq
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - 5;
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
        #pragma acc loop gang vector
        for (i = 0; i < n; i++) {
            #pragma acc loop seq
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker vector
        for (i = 0; i < n; i++) {
            #pragma acc loop seq
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i + j;
            }
        }
    }
}

/* Variant 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Conditional compilation variants */
#ifdef VARIANT_A
#define NUM_GANGS 2
#define NUM_WORKERS 4
#elif defined(VARIANT_B)
#define NUM_GANGS 4
#define NUM_WORKERS 2
#else
#define NUM_GANGS 1
#define NUM_WORKERS 1
#endif

/* Function with conditional compilation for different partitioning */
void compute_variant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc kernels num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) vector_length(8) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop collapse(2)
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 7;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int i, j, n, variant;
    long long checksum = 0;
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = (i * 3 + j * 7) % 100;
        }
    }
    
    /* Read runtime parameter for loop bounds */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SMALL_SIZE) n = SMALL_SIZE;
    } else {
        printf("Enter problem size (1-%d): ", SMALL_SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SMALL_SIZE) n = SMALL_SIZE;
    }
    
    /* Read variant selection */
    if (argc > 2) {
        variant = atoi(argv[2]);
    } else {
        printf("Enter variant (0-8): ");
        scanf("%d", &variant);
    }
    
    /* Call different functions based on variant to trigger different partitioning */
    switch (variant % 9) {
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
        case 8:
            compute_variant(n, src, dst);
            break;
    }
    
    /* Calculate checksum to prevent optimization */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    /* Use volatile to ensure side effects */
    volatile long long final_checksum = checksum;
    printf("Checksum: %lld\n", final_checksum);
    
    return 0;
}
