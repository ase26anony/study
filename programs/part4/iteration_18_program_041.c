/* This program is designed to trigger the partitioning classification logic
   in GCC's OpenACC neutering/broadcast analysis, specifically targeting the
   switch statement that maps integer values 0-7 to descriptive strings. */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define SMALL_SIZE 128

/* Variant 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
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
            #pragma acc loop worker
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
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
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
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * (i + j);
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
            for (j = 0; j < n; j++) {
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
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | 0x01;
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
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] & 0xFF;
            }
        }
    }
}

/* Variant 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3 + i - j;
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
    #pragma acc kernels num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop collapse(2) gang worker
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 2 + 1;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int i, j, n;
    volatile int checksum = 0;
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
        }
    }
    
    /* Read runtime-dependent size from input */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SMALL_SIZE) n = SMALL_SIZE;
    } else {
        printf("Enter size (1-%d): ", SMALL_SIZE);
        if (scanf("%d", &n) != 1 || n <= 0 || n > SMALL_SIZE) {
            n = SMALL_SIZE;
        }
    }
    
    /* Call different functions to trigger various partitioning scenarios */
    compute_gang_redundant(n, src, dst);
    checksum += dst[n/2][n/2];
    
    compute_gang_partitioned(n, src, dst);
    checksum += dst[n/3][n/3];
    
    compute_worker_partitioned(n, src, dst);
    checksum += dst[n/4][n/4];
    
    compute_gang_worker_partitioned(n, src, dst);
    checksum += dst[n/5][n/5];
    
    compute_vector_partitioned(n, src, dst);
    checksum += dst[n/6][n/6];
    
    compute_gang_vector_partitioned(n, src, dst);
    checksum += dst[n/7][n/7];
    
    compute_worker_vector_partitioned(n, src, dst);
    checksum += dst[n/8][n/8];
    
    compute_fully_partitioned(n, src, dst);
    checksum += dst[n/9][n/9];
    
    /* Call variant function */
    compute_variant(n, src, dst);
    checksum += dst[n/10][n/10];
    
    /* Final validation with side effects */
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
