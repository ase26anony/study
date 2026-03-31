#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define BLOCK 32

/* Variant 1: Gang redundant partitioning */
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

/* Variant 2: Gang partitioned */
void compute_gang_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

/* Variant 3: Worker partitioned */
void compute_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

/* Variant 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Variant 5: Vector partitioned */
void compute_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop vector
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop seq
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Variant 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop seq
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | 0x0F;
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop seq
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] & 0xFF;
            }
        }
    }
}

/* Variant 8: Fully partitioned */
void compute_fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(8) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop seq
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] ^ 0x55;
            }
        }
    }
}

/* Alternative implementation using kernels construct */
#ifdef USE_KERNELS
void compute_kernels_variant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3;
            }
        }
    }
}
#endif

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int n, variant;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = i * SIZE + j;
        }
    }
    
    /* Read runtime parameter for loop bounds */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE;
    } else {
        printf("Enter loop bound (1-%d): ", SIZE);
        scanf("%d", &n);
        if (n <= 0 || n > SIZE) n = SIZE;
    }
    
    /* Read variant selection */
    if (argc > 2) {
        variant = atoi(argv[2]);
    } else {
        printf("Enter variant (0-7): ");
        scanf("%d", &variant);
    }
    
    /* Select partitioning variant based on input */
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
    
#ifdef USE_KERNELS
    /* Alternate path using kernels construct */
    compute_kernels_variant(src, dst, n / 2);
#endif
    
    /* Validation checksum to prevent optimization */
    volatile long checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %ld\n", checksum);
    
    return 0;
}
