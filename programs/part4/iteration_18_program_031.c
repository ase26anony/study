/* This program is designed to trigger the OpenACC neutering/broadcast
   analysis logic in GCC, specifically the switch statement that maps
   partitioning types to descriptive strings (cases 0-7). */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define SMALL_SIZE 128

/* Variant 1: Gang redundant (case 0) */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Only num_gangs(1) -> gang redundant */
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

/* Variant 2: Gang partitioned (case 1) */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* num_gangs > 1, workers=1, vector=1 -> gang partitioned */
    #pragma acc kernels num_gangs(8) num_workers(1) vector_length(1) \
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

/* Variant 3: Worker partitioned (case 2) */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* num_gangs=1, num_workers>1, vector=1 -> worker partitioned */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
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

/* Variant 4: Gang+worker partitioned (case 3) */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Both gangs and workers > 1 -> gang+worker partitioned */
    #pragma acc kernels num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Variant 5: Vector partitioned (case 4) */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Only vector_length > 1 -> vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
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

/* Variant 6: Gang+vector partitioned (case 5) */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gangs > 1, vector > 1 -> gang+vector partitioned */
    #pragma acc kernels num_gangs(4) num_workers(1) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + j * 3;
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned (case 6) */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Workers > 1, vector > 1 -> worker+vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(128) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - i * 2;
            }
        }
    }
}

/* Variant 8: Fully partitioned (case 7) */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* All three dimensions > 1 -> fully partitioned */
    #pragma acc kernels num_gangs(8) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3 + j;
            }
        }
    }
}

/* Conditional compilation for path variation */
#ifdef VARIANT_A
#define NUM_GANGS 2
#define NUM_WORKERS 1
#define VECTOR_LEN 1
#elif defined(VARIANT_B)
#define NUM_GANGS 1
#define NUM_WORKERS 4
#define VECTOR_LEN 1
#else
#define NUM_GANGS 2
#define NUM_WORKERS 2
#define VECTOR_LEN 32
#endif

/* Function with conditional compilation variants */
void compute_variant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    #pragma acc parallel num_gangs(NUM_GANGS) num_workers(NUM_WORKERS) vector_length(VECTOR_LEN) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + (i * j);
            }
        }
    }
}

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int i, j, n;
    long checksum = 0;
    volatile long vol_checksum = 0; /* Prevent optimization */
    
    /* Initialize with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = (i * 17 + j * 13) % 100;
        }
    }
    
    /* Runtime-dependent loop bounds */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SMALL_SIZE) n = SMALL_SIZE;
    } else {
        printf("Enter problem size (1-%d): ", SMALL_SIZE);
        if (scanf("%d", &n) != 1) n = SMALL_SIZE;
        if (n <= 0 || n > SMALL_SIZE) n = SMALL_SIZE;
    }
    
    /* Call different functions based on input to trigger various cases */
    switch (n % 8) {
        case 0: compute_gang_redundant(n, src, dst); break;
        case 1: compute_gang_partitioned(n, src, dst); break;
        case 2: compute_worker_partitioned(n, src, dst); break;
        case 3: compute_gang_worker_partitioned(n, src, dst); break;
        case 4: compute_vector_partitioned(n, src, dst); break;
        case 5: compute_gang_vector_partitioned(n, src, dst); break;
        case 6: compute_worker_vector_partitioned(n, src, dst); break;
        case 7: compute_fully_partitioned(n, src, dst); break;
    }
    
    /* Also call the variant function */
    compute_variant(n, src, dst);
    
    /* Validation checksum with side effects */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    vol_checksum = checksum;
    printf("Checksum: %ld\n", vol_checksum);
    
    return 0;
}
