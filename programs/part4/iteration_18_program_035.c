#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITERS 8

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang partitioned */
void compute_gang_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc kernels num_gangs(8) copy(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc kernels num_gangs(8) vector_length(64) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] | 0xFF;
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] & 0x7F;
            }
        }
    }
}

/* Function 8: Fully partitioned */
void compute_fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                #pragma acc loop vector
                for (int k = 0; k < ITERS; k++) {
                    dst[i][j] += src[i][j] * k;
                }
            }
        }
    }
}

/* Variant with different partitioning */
#ifdef VARIANT
void compute_variant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(16) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] ^ 0x55;
            }
        }
    }
}
#endif

/* Another variant with collapsed loops */
#ifdef VARIANT2
void compute_variant2(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(64) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < n; j++) {  /* Runtime-dependent bound */
                dst[i][j] = src[i][j] << 2;
            }
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int n, variant;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
        }
    }
    
    /* Read runtime parameter for loop bounds */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE / 2;
    } else {
        printf("Enter loop bound (1-%d): ", SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SIZE) n = SIZE / 2;
    }
    
    /* Read variant selector */
    if (argc > 2) {
        variant = atoi(argv[2]);
    } else {
        printf("Enter variant (0-7): ");
        scanf("%d", &variant);
    }
    
    /* Call different functions based on variant */
    switch (variant % 8) {
        case 0:
            compute_gang_redundant(src, dst, n);
            break;
        case 1:
            compute_gang_partitioned(src, dst, n);
            break;
        case 2:
            compute_worker_partitioned(src, dst, n);
            break;
        case 3:
            compute_gang_worker_partitioned(src, dst, n);
            break;
        case 4:
            compute_vector_partitioned(src, dst, n);
            break;
        case 5:
            compute_gang_vector_partitioned(src, dst, n);
            break;
        case 6:
            compute_worker_vector_partitioned(src, dst, n);
            break;
        case 7:
            compute_fully_partitioned(src, dst, n);
            break;
    }
    
#ifdef VARIANT
    compute_variant(src, dst, n);
#endif
    
#ifdef VARIANT2
    compute_variant2(src, dst, n);
#endif
    
    /* Validation checksum to prevent optimization */
    volatile long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    /* Force side effects */
    printf("Result[0][0] = %d\n", dst[0][0]);
    printf("Result[%d][%d] = %d\n", SIZE-1, n-1, dst[SIZE-1][n-1]);
    
    return 0;
}
