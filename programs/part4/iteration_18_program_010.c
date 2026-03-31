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
        for (i = 0; i < SIZE; i += n) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j += n) {
                int ii, jj;
                for (ii = i; ii < i + n && ii < SIZE; ii++) {
                    for (jj = j; jj < j + n && jj < SIZE; jj++) {
                        dst[ii][jj] = src[ii][jj] * 2;
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
        for (i = 0; i < SIZE; i += BLOCK) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j += BLOCK) {
                int ii, jj;
                for (ii = i; ii < i + BLOCK && ii < SIZE; ii++) {
                    for (jj = j; jj < j + BLOCK && jj < SIZE; jj++) {
                        dst[ii][jj] = src[ii][jj] + n;
                    }
                }
            }
        }
    }
}

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc kernels num_gangs(4) num_workers(2) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i += n) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j += n) {
                dst[i][j] = src[i][j] * 3;
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] - n;
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
        for (i = 0; i < SIZE; i += n) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] / (n + 1);
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc kernels num_gangs(1) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop worker
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + i + j;
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
                dst[i][j] = src[i][j] * (n + 2);
            }
        }
    }
}

/* Variant with different partitioning */
#ifdef VARIANT
void compute_variant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copy(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i++) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] * 4;
            }
        }
    }
}
#endif

/* Another variant with collapsed loops */
#ifdef VARIANT2
void compute_variant2(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector collapse(2)
        for (i = 0; i < SIZE; i++) {
            for (j = 0; j < SIZE; j++) {
                dst[i][j] = src[i][j] + (i * j) % (n + 1);
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
            src[i][j] = (i * 3 + j * 7) % 100;
        }
    }
    
    /* Read runtime-dependent parameter */
    if (argc > 1) {
        n = atoi(argv[1]);
    } else {
        printf("Enter partitioning mode (1-8): ");
        scanf("%d", &n);
    }
    
    if (n < 1) n = 1;
    if (n > 8) n = 8;
    
    /* Call different functions based on input to trigger different partitioning cases */
    switch (n) {
        case 1:
            compute_gang_redundant(src, dst, BLOCK);
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
    
    /* Conditional compilation for variant paths */
    #ifdef VARIANT
    compute_variant(src, dst, n);
    #endif
    
    #ifdef VARIANT2
    compute_variant2(src, dst, n);
    #endif
    
    /* Calculate checksum to prevent optimization */
    for (i = 0; i < SIZE; i += 64) {
        for (j = 0; j < SIZE; j += 64) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Force side effects */
    printf("Result at [0][0]: %d\n", dst[0][0]);
    printf("Result at [%d][%d]: %d\n", SIZE-1, SIZE-1, dst[SIZE-1][SIZE-1]);
    
    return 0;
}
