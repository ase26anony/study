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
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += n) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j += n) {
                int ii, jj;
                for (ii = i; ii < i + n && ii < SIZE; ii++) {
                    for (jj = j; jj < j + n && jj < SIZE; jj++) {
                        dst[ii][jj] = src[ii][jj] - n;
                    }
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
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += BLOCK) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j += BLOCK) {
                int ii, jj;
                for (ii = i; ii < i + BLOCK && ii < SIZE; ii++) {
                    for (jj = j; jj < j + BLOCK && jj < SIZE; jj++) {
                        dst[ii][jj] = src[ii][jj] * src[ii][jj];
                    }
                }
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
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += n) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j += n) {
                int ii, jj;
                for (ii = i; ii < i + n && ii < SIZE; ii++) {
                    for (jj = j; jj < j + n && jj < SIZE; jj++) {
                        dst[ii][jj] = src[ii][jj] / (n + 1);
                    }
                }
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
        for (i = 0; i < SIZE; i += BLOCK) {
            #pragma acc loop vector
            for (j = 0; j < SIZE; j += BLOCK) {
                int ii, jj;
                for (ii = i; ii < i + BLOCK && ii < SIZE; ii++) {
                    for (jj = j; jj < j + BLOCK && jj < SIZE; jj++) {
                        dst[ii][jj] = src[ii][jj] + (ii * jj);
                    }
                }
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
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += n) {
            #pragma acc loop worker vector
            for (j = 0; j < SIZE; j += n) {
                int ii, jj;
                for (ii = i; ii < i + n && ii < SIZE; ii++) {
                    for (jj = j; jj < j + n && jj < SIZE; jj++) {
                        dst[ii][jj] = src[ii][jj] | (ii & jj);
                    }
                }
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
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += BLOCK) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j += BLOCK) {
                int ii, jj;
                #pragma acc loop vector
                for (ii = i; ii < i + BLOCK && ii < SIZE; ii++) {
                    for (jj = j; jj < j + BLOCK && jj < SIZE; jj++) {
                        dst[ii][jj] = src[ii][jj] * 3 + n;
                    }
                }
            }
        }
    }
}

/* Variant using kernels construct */
#ifdef USE_KERNELS
void compute_kernels_variant(int src[SIZE][SIZE], int dst[SIZE][SIZE], int n) {
    int i, j;
    
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < SIZE; i += n) {
            #pragma acc loop worker
            for (j = 0; j < SIZE; j += n) {
                int ii, jj;
                for (ii = i; ii < i + n && ii < SIZE; ii++) {
                    for (jj = j; jj < j + n && jj < SIZE; jj++) {
                        dst[ii][jj] = src[ii][jj] << 1;
                    }
                }
            }
        }
    }
}
#endif

/* Main function with runtime-dependent execution */
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
        if (n < 1) n = 1;
        if (n > 8) n = 8;
    } else {
        printf("Enter block size (1-8): ");
        scanf("%d", &n);
    }
    
    /* Read variant selector */
    if (argc > 2) {
        variant = atoi(argv[2]);
    } else {
        printf("Enter variant (0-7): ");
        scanf("%d", &variant);
    }
    
    /* Clear destination array */
    memset(dst, 0, sizeof(dst));
    
    /* Select different partitioning scenarios based on variant */
    switch (variant % 8) {
        case 0:
            compute_gang_redundant(src, dst, n);
            printf("Executed: gang redundant\n");
            break;
        case 1:
            compute_gang_partitioned(src, dst, n);
            printf("Executed: gang partitioned\n");
            break;
        case 2:
            compute_worker_partitioned(src, dst, n);
            printf("Executed: worker partitioned\n");
            break;
        case 3:
            compute_gang_worker_partitioned(src, dst, n);
            printf("Executed: gang+worker partitioned\n");
            break;
        case 4:
            compute_vector_partitioned(src, dst, n);
            printf("Executed: vector partitioned\n");
            break;
        case 5:
            compute_gang_vector_partitioned(src, dst, n);
            printf("Executed: gang+vector partitioned\n");
            break;
        case 6:
            compute_worker_vector_partitioned(src, dst, n);
            printf("Executed: worker+vector partitioned\n");
            break;
        case 7:
            compute_fully_partitioned(src, dst, n);
            printf("Executed: fully partitioned\n");
            break;
    }
    
#ifdef USE_KERNELS
    /* Alternate variant using kernels construct */
    if (variant > 7) {
        compute_kernels_variant(src, dst, n);
        printf("Executed: kernels variant\n");
    }
#endif
    
    /* Calculate checksum to prevent optimization */
    for (i = 0; i < SIZE; i += n) {
        for (j = 0; j < SIZE; j += n) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Force side effect */
    printf("Result[0][0] = %d\n", dst[0][0]);
    printf("Result[%d][%d] = %d\n", SIZE-1, SIZE-1, dst[SIZE-1][SIZE-1]);
    
    return 0;
}
