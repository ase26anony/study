#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define BLOCK 32

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += BLOCK) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                int ii, jj;
                int limit_i = i + BLOCK < SIZE ? i + BLOCK : SIZE;
                int limit_j = j + n;  /* Runtime-dependent bound */
                if (limit_j > SIZE) limit_j = SIZE;
                
                for (ii = i; ii < limit_i; ii++) {
                    for (jj = j; jj < limit_j; jj++) {
                        dst[ii][jj] = src[ii][jj] * 2 + 1;
                    }
                }
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
            for (j = 0; j < SIZE; j += n) {  /* Runtime-dependent stride */
                int jj;
                int limit = j + n;
                if (limit > SIZE) limit = SIZE;
                
                for (jj = j; jj < limit; jj++) {
                    dst[i][jj] = src[i][jj] + i - jj;
                }
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
                int k;
                int iterations = n % 8 + 1;  /* Runtime-dependent */
                
                for (k = 0; k < iterations; k++) {
                    dst[i][j] += src[i][j] * k;
                }
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
                int block = n % 16 + 1;  /* Runtime-dependent */
                int start = j * block;
                int end = start + block;
                
                if (end > SIZE) end = SIZE;
                if (start < SIZE) {
                    dst[i][j] = src[i][start] * 3;
                }
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(64) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                int stride = n % 4 + 1;  /* Runtime-dependent */
                dst[i][j] = src[i][j] * stride;
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc parallel num_gangs(8) num_workers(1) vector_length(32) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                int repeat = n % 5 + 1;  /* Runtime-dependent */
                int val = src[i][j];
                
                for (int r = 0; r < repeat; r++) {
                    val = val * 2 - 1;
                }
                dst[i][j] = val;
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(32) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                int offset = n % 10;  /* Runtime-dependent */
                int idx = (j + offset) % SIZE;
                dst[i][j] = src[i][idx] + src[i][j];
            }
        }
    }
}

/* Function 8: Fully partitioned */
void compute_fully_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                int factor = n % 7 + 1;  /* Runtime-dependent */
                dst[i][j] = src[i][j] * factor + i - j;
            }
        }
    }
}

/* Variant using kernels construct */
#ifdef USE_KERNELS
void compute_kernels_variant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc kernels num_gangs(4) num_workers(2) vector_length(16) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                int block = n % 12 + 1;  /* Runtime-dependent */
                int sum = 0;
                
                for (int b = 0; b < block && (j + b) < SIZE; b++) {
                    sum += src[i][j + b];
                }
                dst[i][j] = sum;
            }
        }
    }
}
#endif

/* Variant with collapsed loops */
#ifdef COLLAPSE_LOOPS
void compute_collapsed(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
                copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector collapse(2)
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                int mod = n % 6 + 1;  /* Runtime-dependent */
                dst[i][j] = (src[i][j] % mod) * 100;
            }
        }
    }
}
#endif

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
    
    /* Read runtime parameter for loop bounds */
    if (argc > 1) {
        n = atoi(argv[1]) % 20;
        if (n <= 0) n = 5;
    } else {
        printf("Enter a number (1-20): ");
        scanf("%d", &n);
        n = n % 20;
        if (n <= 0) n = 5;
    }
    
    /* Clear destination array */
    memset(dst, 0, sizeof(dst));
    
    /* Call different functions based on n to trigger various partitioning cases */
    switch (n % 8) {
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
    /* Alternate variant using kernels construct */
    if (n % 3 == 0) {
        compute_kernels_variant(src, dst, n);
    }
#endif
    
#ifdef COLLAPSE_LOOPS
    /* Variant with collapsed loops */
    if (n % 4 == 0) {
        compute_collapsed(src, dst, n);
    }
#endif
    
    /* Compute checksum to prevent optimization */
    for (i = 0; i < SIZE; i += 16) {
        for (j = 0; j < SIZE; j += 16) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d (n=%d)\n", checksum, n);
    
    /* Force side effect */
    volatile int dummy = checksum;
    
    return 0;
}
