/* This program is designed to trigger the OpenACC neutering/broadcast
   analysis logic, specifically the switch statement that maps partitioning
   types to descriptive strings (cases 0-7). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SIZE 1024
#define PATTERN 7

/* Function 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang redundant: only num_gangs(1) specified */
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang partitioned: only num_gangs specified */
    #pragma acc parallel num_gangs(8) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i;
            }
        }
    }
}

/* Function 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Worker partitioned: only num_workers specified */
    #pragma acc kernels num_workers(4) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - j;
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang+worker partitioned */
    #pragma acc parallel num_gangs(4) num_workers(2) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Vector partitioned: only vector_length specified */
    #pragma acc kernels vector_length(32) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / 2;
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang+vector partitioned */
    #pragma acc parallel num_gangs(8) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | PATTERN;
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Worker+vector partitioned */
    #pragma acc kernels num_workers(2) vector_length(128) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] & PATTERN;
            }
        }
    }
}

/* Function 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Fully partitioned: all three clauses specified */
    #pragma acc parallel num_gangs(16) num_workers(4) vector_length(256) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] ^ PATTERN;
            }
        }
    }
}

/* Main computational kernel with conditional compilation */
void process_data(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE], int variant) {
    int i, j;
    
#ifdef VARIANT1
    /* Variant 1: gang+worker partitioned with collapsed loops */
    #pragma acc parallel num_gangs(8) num_workers(2) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker collapse(2)
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3 + variant;
            }
        }
    }
#elif defined(VARIANT2)
    /* Variant 2: worker+vector partitioned with different nesting */
    #pragma acc kernels num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - variant * 2;
            }
        }
    }
#else
    /* Default variant: fully partitioned */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + variant * 5;
            }
        }
    }
#endif
}

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int i, j, n, variant;
    volatile int checksum = 0; /* volatile to prevent optimization */
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = (i * SIZE + j) % 256;
        }
    }
    
    /* Read runtime-dependent parameter */
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = 64; /* default */
    } else {
        printf("Enter problem size (1-1024): ");
        if (scanf("%d", &n) != 1) n = 64;
    }
    
    if (argc > 2) {
        variant = atoi(argv[2]);
    } else {
        variant = 0;
    }
    
    /* Call different functions to trigger various partitioning analyses */
    switch (variant % 8) {
        case 0:
            compute_gang_redundant(n, src, dst);
            break;
        case 1:
            compute_gang_partitioned(n, src, dst);
            break;
        case 2:
            compute_worker_partitioned(n, src, dst);
            break;
        case 3:
            compute_gang_worker_partitioned(n, src, dst);
            break;
        case 4:
            compute_vector_partitioned(n, src, dst);
            break;
        case 5:
            compute_gang_vector_partitioned(n, src, dst);
            break;
        case 6:
            compute_worker_vector_partitioned(n, src, dst);
            break;
        case 7:
            compute_fully_partitioned(n, src, dst);
            break;
    }
    
    /* Also call the conditional compilation kernel */
    process_data(n, src, dst, variant);
    
    /* Validation checksum to prevent dead code elimination */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}
