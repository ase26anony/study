/* This program is designed to trigger the OpenACC neutering/broadcast analysis
   logic that maps partitioning scenarios to descriptive string labels. */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define MAX_GANGS 8
#define MAX_WORKERS 4
#define VECTOR_LEN 32

/* Initialize arrays with pattern */
void init_arrays(int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
            dst[i][j] = 0;
        }
    }
}

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) copyin(src) copyout(dst)
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang+worker partitioned */
void compute_gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(MAX_GANGS) num_workers(MAX_WORKERS) vector_length(1) \
                copyin(src) copy(dst)
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] += src[i][j] * 3;
            }
        }
    }
}

/* Function 3: Fully partitioned (gang+worker+vector) */
void compute_fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(MAX_GANGS) num_workers(MAX_WORKERS) vector_length(VECTOR_LEN) \
                copyin(src) copy(dst)
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (int j = 0; j < SIZE; j++) {
                #pragma acc loop vector
                for (int k = 0; k < n; k++) {  /* Runtime-dependent bound */
                    dst[i][j] += src[i][j] * (k + 1);
                }
            }
        }
    }
}

/* Function 4: Worker+vector partitioned */
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc kernels num_workers(MAX_WORKERS) vector_length(VECTOR_LEN) \
                copyin(src) copy(dst)
    {
        #pragma acc loop worker
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] - 5;
            }
        }
    }
}

/* Function 5: Gang+vector partitioned */
void compute_gang_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(MAX_GANGS) vector_length(VECTOR_LEN) \
                copyin(src) copy(dst)
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] + 10;
            }
        }
    }
}

/* Conditional compilation variants */
#ifdef VARIANT1
#define VAR_GANGS 4
#define VAR_WORKERS 2
#else
#define VAR_GANGS 2
#define VAR_WORKERS 4
#endif

/* Function with variant-dependent partitioning */
void compute_variant_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(VAR_GANGS) num_workers(VAR_WORKERS) vector_length(16) \
                copyin(src) copy(dst)
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (int j = 0; j < SIZE; j++) {
                for (int k = 0; k < n; k++) {  /* Runtime-dependent bound */
                    dst[i][j] += src[i][j] * (i + j + k);
                }
            }
        }
    }
}

/* Checksum validation */
int checksum(int arr[SIZE][SIZE]) {
    int sum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            sum += arr[i][j];
        }
    }
    return sum;
}

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int variant = 0;
    
    /* Read partitioning variant from input */
    if (argc > 1) {
        variant = atoi(argv[1]);
        if (variant < 0 || variant > 7) variant = 0;
    } else {
        printf("Enter variant (0-7): ");
        scanf("%d", &variant);
    }
    
    /* Runtime-dependent loop bound */
    int n = (variant % 8) + 1;
    
    init_arrays(src, dst);
    
    /* Call different functions based on variant to trigger different partitioning */
    switch (variant % 5) {
        case 0:
            compute_gang_redundant(src, dst, n);
            break;
        case 1:
            compute_gang_worker_partitioned(src, dst, n);
            break;
        case 2:
            compute_fully_partitioned(src, dst, n);
            break;
        case 3:
            compute_worker_vector_partitioned(src, dst, n);
            break;
        case 4:
            compute_gang_vector_partitioned(src, dst, n);
            break;
    }
    
    /* Also call variant function */
    compute_variant_partitioned(src, dst, n);
    
    /* Force side effects and validation */
    volatile int result = checksum(dst);
    printf("Checksum: %d\n", result);
    
    return 0;
}
