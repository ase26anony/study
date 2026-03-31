/* This program is designed to trigger the OpenACC neutering/broadcast
   analysis logic for different partitioning scenarios, specifically
   targeting the switch statement in omp-oacc-neuter-broadcast.cc
   that maps integer case values (0-7) to partitioning type strings. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define ITER 8

/* Variant 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
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
        for (i = 0; i < SIZE; i++) {
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
    #pragma acc kernels num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

/* Variant 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Variant 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc kernels num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / 3;
            }
        }
    }
}

/* Variant 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | 0xFF;
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc kernels num_gangs(1) num_workers(2) vector_length(128) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
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
        #pragma acc loop gang worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] ^ 0x55;
            }
        }
    }
}

/* Conditional compilation for path variation */
#ifdef VARIANT_A
#define NUM_GANGS 16
#define NUM_WORKERS 1
#define VECTOR_LEN 32
#elif defined(VARIANT_B)
#define NUM_GANGS 1
#define NUM_WORKERS 16
#define VECTOR_LEN 32
#else
#define NUM_GANGS 8
#define NUM_WORKERS 4
#define VECTOR_LEN 64
#endif

/* Additional function with conditional compilation */
void compute_variant_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) vector_length(VECTOR_LEN) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3 - 2;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int i, j, n, variant;
    volatile int checksum = 0;  /* Prevent optimization */
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = (i * 17 + j * 13) % 100;
        }
    }
    
    /* Read runtime parameter for loop bounds and variant selection */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE / 2;
    } else {
        printf("Enter loop bound (1-%d): ", SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SIZE) n = SIZE / 2;
    }
    
    if (argc > 2) {
        variant = atoi(argv[2]);
    } else {
        printf("Enter variant (0-8): ");
        scanf("%d", &variant);
    }
    
    /* Call different functions based on variant to trigger different partitioning */
    switch (variant % 9) {
        case 0: compute_gang_redundant(n, src, dst); break;
        case 1: compute_gang_partitioned(n, src, dst); break;
        case 2: compute_worker_partitioned(n, src, dst); break;
        case 3: compute_gang_worker_partitioned(n, src, dst); break;
        case 4: compute_vector_partitioned(n, src, dst); break;
        case 5: compute_gang_vector_partitioned(n, src, dst); break;
        case 6: compute_worker_vector_partitioned(n, src, dst); break;
        case 7: compute_fully_partitioned(n, src, dst); break;
        case 8: compute_variant_partitioned(n, src, dst); break;
    }
    
    /* Validation checksum to prevent dead code elimination */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional side effects */
    for (i = 0; i < ITER; i++) {
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(16) \
            copy(dst[0:SIZE][0:SIZE])
        {
            #pragma acc loop gang worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] += i;
            }
        }
    }
    
    return 0;
}
