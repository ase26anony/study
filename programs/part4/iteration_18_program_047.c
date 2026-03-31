#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define BLOCK 32

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += n) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j += n) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang partitioned */
void compute_gang_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + i + j;
            }
        }
    }
}

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * (n + 1);
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] - (i % n) - (j % n);
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] / (n > 0 ? n : 1);
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + (i * j) % n;
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] | (n & 0xFF);
            }
        }
    }
}

/* Function 8: Fully partitioned */
void compute_fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] ^ (i * n + j);
            }
        }
    }
}

/* Variant with conditional compilation */
#ifdef VARIANT_A
void compute_variant_a(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc kernels num_gangs(8) num_workers(2) vector_length(16) \
        copy(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop collapse(2)
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * 3 + n;
            }
        }
    }
}
#elif defined(VARIANT_B)
void compute_variant_b(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i += 2) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j += 2) {
                dst[i][j] = src[i][j] - n;
                if (i + 1 < SIZE && j + 1 < SIZE) {
                    dst[i+1][j+1] = src[i+1][j+1] + n;
                }
            }
        }
    }
}
#else
void compute_default_variant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    #pragma acc parallel num_gangs(4) num_workers(4) vector_length(8) \
        copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * (2 + (n % 3));
            }
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int i, j, n, variant;
    volatile int checksum = 0;
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
        }
    }
    
    /* Read runtime parameter for loop bounds */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 1;
        if (n > 8) n = 8;
    } else {
        printf("Enter partitioning mode (1-8): ");
        scanf("%d", &n);
        if (n < 1 || n > 8) n = 1;
    }
    
    /* Read variant selection */
    if (argc > 2) {
        variant = atoi(argv[2]);
    } else {
        printf("Enter variant (0-2): ");
        scanf("%d", &variant);
    }
    
    /* Call different functions based on input to trigger various partitioning cases */
    switch (n) {
        case 1:
            compute_gang_redundant(src, dst, n);
            break;
        case 2:
            compute_gang_partitioned(src, dst, n);
            break;
        case 3:
            compute_worker_partitioned(src, dst, n);
            break;
        case 4:
            compute_gang_worker_partitioned(src, dst, n);
            break;
        case 5:
            compute_vector_partitioned(src, dst, n);
            break;
        case 6:
            compute_gang_vector_partitioned(src, dst, n);
            break;
        case 7:
            compute_worker_vector_partitioned(src, dst, n);
            break;
        case 8:
            compute_fully_partitioned(src, dst, n);
            break;
    }
    
    /* Call variant function based on selection */
    switch (variant) {
        case 0:
            compute_default_variant(src, dst, n);
            break;
        case 1:
            /* Compile with -DVARIANT_A for this path */
            #ifdef VARIANT_A
            compute_variant_a(src, dst, n);
            #else
            compute_default_variant(src, dst, n);
            #endif
            break;
        case 2:
            /* Compile with -DVARIANT_B for this path */
            #ifdef VARIANT_B
            compute_variant_b(src, dst, n);
            #else
            compute_default_variant(src, dst, n);
            #endif
            break;
    }
    
    /* Calculate checksum to prevent optimization */
    for (i = 0; i < SIZE; i += BLOCK) {
        for (j = 0; j < SIZE; j += BLOCK) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Force side effect */
    printf("Result[0][0] = %d, Result[%d][%d] = %d\n", 
           dst[0][0], SIZE-1, SIZE-1, dst[SIZE-1][SIZE-1]);
    
    return 0;
}
