/* This program is designed to trigger the partitioning classification logic
   in GCC's OpenACC neutering/broadcast analysis, specifically targeting the
   switch statement that maps integer codes to partitioning type strings. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define SMALL_SIZE 256

/* Variant 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang redundant - only num_gangs(1) specified */
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
    /* Gang partitioned - only num_gangs specified */
    #pragma acc parallel num_gangs(8) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

/* Variant 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Worker partitioned - only num_workers specified */
    #pragma acc parallel num_workers(4) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

/* Variant 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang+worker partitioned */
    #pragma acc parallel num_gangs(8) num_workers(2) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Variant 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Vector partitioned - only vector_length specified */
    #pragma acc parallel vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Variant 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang+vector partitioned */
    #pragma acc parallel num_gangs(8) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] | 0x1;
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Worker+vector partitioned */
    #pragma acc parallel num_workers(4) vector_length(128) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] & 0xFF;
            }
        }
    }
}

/* Variant 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Fully partitioned - all three clauses specified */
    #pragma acc parallel num_gangs(16) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] * 3 + j;
            }
        }
    }
}

/* Alternative implementation using kernels construct */
#ifdef USE_KERNELS
void compute_kernels_variant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Kernels with different partitioning */
    #pragma acc kernels num_gangs(8) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] << 1;
            }
        }
    }
}
#endif

/* Function with collapsed loops to trigger different analysis */
void compute_collapsed(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Collapsed loops with gang+worker partitioning */
    #pragma acc parallel num_gangs(8) num_workers(2) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker collapse(2)
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] + src[i][j];
            }
        }
    }
}

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int i, j, n;
    long long checksum = 0;
    volatile long long vol_checksum = 0; /* Prevent optimization */
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = (i * 17 + j * 13) % 100;
        }
    }
    
    /* Read runtime-dependent loop bound */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE / 2;
    } else {
        printf("Enter loop bound (1-%d): ", SIZE);
        if (scanf("%d", &n) != 1) n = SIZE / 2;
        if (n <= 0 || n > SIZE) n = SIZE / 2;
    }
    
    /* Clear destination array */
    memset(dst, 0, sizeof(dst));
    
    /* Select different partitioning variants based on input */
    switch (n % 8) {
        case 0:
            compute_gang_redundant(n, src, dst);
            printf("Executed: gang redundant\n");
            break;
        case 1:
            compute_gang_partitioned(n, src, dst);
            printf("Executed: gang partitioned\n");
            break;
        case 2:
            compute_worker_partitioned(n, src, dst);
            printf("Executed: worker partitioned\n");
            break;
        case 3:
            compute_gang_worker_partitioned(n, src, dst);
            printf("Executed: gang+worker partitioned\n");
            break;
        case 4:
            compute_vector_partitioned(n, src, dst);
            printf("Executed: vector partitioned\n");
            break;
        case 5:
            compute_gang_vector_partitioned(n, src, dst);
            printf("Executed: gang+vector partitioned\n");
            break;
        case 6:
            compute_worker_vector_partitioned(n, src, dst);
            printf("Executed: worker+vector partitioned\n");
            break;
        case 7:
            compute_fully_partitioned(n, src, dst);
            printf("Executed: fully partitioned\n");
            break;
    }
    
    /* Conditional compilation for additional variants */
#ifdef VARIANT1
    compute_collapsed(n, src, dst);
    printf("Executed collapsed variant\n");
#endif
    
#ifdef USE_KERNELS
    compute_kernels_variant(n, src, dst);
    printf("Executed kernels variant\n");
#endif
    
    /* Calculate checksum to prevent optimization */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    vol_checksum = checksum;
    printf("Checksum: %lld\n", vol_checksum);
    
    return 0;
}
