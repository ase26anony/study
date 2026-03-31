#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define BLOCK 32

/* Variant 1: Gang redundant partitioning */
void compute_gang_redundant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (int i = 0; i < SIZE; i += BLOCK) {
            int limit_i = i + BLOCK < SIZE ? i + BLOCK : SIZE;
            #pragma acc loop vector
            for (int j = 0; j < SIZE; j++) {
                for (int k = 0; k < n; k++) {  /* Runtime-dependent inner loop */
                    dst[i + k % (limit_i - i)][j] = src[i][j] * 2 + k;
                }
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
            int inner_limit = n + (i % 4);  /* Runtime-dependent */
            #pragma acc loop worker
            for (int j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * inner_limit;
            }
        }
    }
}

/* Variant 3: Worker partitioned */
void compute_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j += n) {  /* Runtime-dependent stride */
                int end_j = j + n < SIZE ? j + n : SIZE;
                #pragma acc loop vector
                for (int k = j; k < end_j; k++) {
                    dst[i][k] = src[i][k] + i * j;
                }
            }
        }
    }
}

/* Variant 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc kernels num_gangs(4) num_workers(2) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (int i = 0; i < SIZE; i++) {
            int chunk = n * 2;  /* Runtime-dependent */
            for (int j = 0; j < SIZE; j += chunk) {
                int end_j = j + chunk < SIZE ? j + chunk : SIZE;
                for (int k = j; k < end_j; k++) {
                    dst[i][k] = src[i][k] * (i + k % n);
                }
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
            for (int j = 0; j < SIZE; j++) {
                int iterations = n + (j % 8);  /* Runtime-dependent */
                for (int k = 0; k < iterations; k++) {
                    dst[i][j] += src[i][j] * k;
                }
            }
        }
    }
}

/* Variant 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(64) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < SIZE; i++) {
            int stride = n * 4;  /* Runtime-dependent */
            for (int j = 0; j < SIZE; j += stride) {
                int end_j = j + stride < SIZE ? j + stride : SIZE;
                for (int k = j; k < end_j; k++) {
                    dst[i][k] = src[i][k] * (i % n + 1);
                }
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc kernels num_gangs(1) num_workers(4) vector_length(128) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker vector
        for (int i = 0; i < SIZE; i++) {
            int block_size = n * 8;  /* Runtime-dependent */
            for (int j = 0; j < SIZE; j += block_size) {
                int end_j = j + block_size < SIZE ? j + block_size : SIZE;
                for (int k = j; k < end_j; k++) {
                    dst[i][k] = src[i][k] + (k % n) * 3;
                }
            }
        }
    }
}

/* Variant 8: Fully partitioned */
void compute_fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    #pragma acc parallel num_gangs(16) num_workers(8) vector_length(256) \
        copy(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (int i = 0; i < SIZE; i++) {
            int dynamic_bound = n * 16;  /* Runtime-dependent */
            for (int j = 0; j < SIZE; j++) {
                int limit = j + dynamic_bound < SIZE ? dynamic_bound : SIZE - j;
                for (int k = 0; k < limit; k++) {
                    dst[i][j + k] = src[i][j + k] * (n + i % 8 + j % 4);
                }
                j += limit - 1;
            }
        }
    }
}

/* Conditional compilation for different partitioning strategies */
#ifdef VARIANT_A
#define SELECTED_VARIANT compute_gang_redundant
#elif defined(VARIANT_B)
#define SELECTED_VARIANT compute_gang_partitioned
#elif defined(VARIANT_C)
#define SELECTED_VARIANT compute_worker_partitioned
#elif defined(VARIANT_D)
#define SELECTED_VARIANT compute_gang_worker_partitioned
#elif defined(VARIANT_E)
#define SELECTED_VARIANT compute_vector_partitioned
#elif defined(VARIANT_F)
#define SELECTED_VARIANT compute_gang_vector_partitioned
#elif defined(VARIANT_G)
#define SELECTED_VARIANT compute_worker_vector_partitioned
#else
#define SELECTED_VARIANT compute_fully_partitioned
#endif

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int n = 4;  /* Default runtime parameter */
    
    /* Read runtime parameter */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 1) n = 1;
        if (n > 32) n = 32;
    } else {
        printf("Enter a number (1-32): ");
        scanf("%d", &n);
        if (n < 1) n = 1;
        if (n > 32) n = 32;
    }
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = (i * 17 + j * 13) % 100;
        }
    }
    
    /* Clear destination array */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            dst[i][j] = 0;
        }
    }
    
    /* Call different functions based on input to trigger various partitioning cases */
    volatile int selector = n % 8;
    
    switch (selector) {
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
    
    /* Also call the conditionally compiled variant */
    SELECTED_VARIANT(src, dst, n);
    
    /* Validation checksum to prevent optimization */
    long long checksum = 0;
    for (int i = 0; i < SIZE; i += 64) {
        for (int j = 0; j < SIZE; j += 64) {
            checksum += dst[i][j];
        }
    }
    
    /* Use volatile to ensure side effects */
    volatile long long final_result = checksum;
    printf("Checksum: %lld\n", (long long)final_result);
    
    return 0;
}
