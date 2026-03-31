/* This program is designed to trigger the OpenACC neutering/broadcast
   analysis logic for different partitioning scenarios, specifically
   targeting the switch statement in omp-oacc-neuter-broadcast.cc
   that maps integer codes to partitioning type strings. */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define SMALL_SIZE 128

/* Variant 1: Gang redundant partitioning */
void compute_gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang redundant - only 1 gang */
    #pragma acc parallel num_gangs(1) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Variant 2: Gang partitioned */
void compute_gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang partitioned - multiple gangs, default workers/vectors */
    #pragma acc parallel num_gangs(8) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i + j;
            }
        }
    }
}

/* Variant 3: Worker partitioned */
void compute_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Worker partitioned - 1 gang, multiple workers */
    #pragma acc parallel num_gangs(1) num_workers(4) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 3 - j;
            }
        }
    }
}

/* Variant 4: Gang+worker partitioned */
void compute_gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang+worker partitioned */
    #pragma acc parallel num_gangs(4) num_workers(2) copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] / 2 + i * j;
            }
        }
    }
}

/* Variant 5: Vector partitioned */
void compute_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Vector partitioned - 1 gang, 1 worker, multiple vectors */
    #pragma acc parallel num_gangs(1) num_workers(1) vector_length(32) \
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

/* Variant 6: Gang+vector partitioned */
void compute_gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Gang+vector partitioned */
    #pragma acc parallel num_gangs(4) num_workers(1) vector_length(64) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + (i << 2) - (j >> 1);
            }
        }
    }
}

/* Variant 7: Worker+vector partitioned */
void compute_worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Worker+vector partitioned */
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] | (i & j);
            }
        }
    }
}

/* Variant 8: Fully partitioned */
void compute_fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Fully partitioned - gangs, workers, and vectors all specified */
    #pragma acc parallel num_gangs(8) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                /* Additional inner loop to create more nesting */
                int k;
                int temp = 0;
                #pragma acc loop vector reduction(+:temp)
                for (k = 0; k < 4; k++) {
                    temp += src[i][j] + k;
                }
                dst[i][j] = temp;
            }
        }
    }
}

/* Alternative implementation using kernels construct */
void compute_kernels_variant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    int i, j;
    /* Kernels construct with different partitioning */
    #pragma acc kernels num_gangs(2) num_workers(2) vector_length(16) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker vector
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 5 - 3;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    static int src[SIZE][SIZE];
    static int dst[SIZE][SIZE];
    int i, j;
    int n = SMALL_SIZE;  /* Base size */
    int variant = 0;
    volatile int checksum = 0;  /* Prevent optimization */
    
    /* Initialize source array with pattern */
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            src[i][j] = i * 1000 + j;
        }
    }
    
    /* Read variant selection from command line or stdin */
    if (argc > 1) {
        variant = atoi(argv[1]);
        if (variant < 0 || variant > 8) variant = 0;
    } else {
        printf("Enter variant (0-8): ");
        scanf("%d", &variant);
        if (variant < 0 || variant > 8) variant = 0;
    }
    
    /* Make loop bounds runtime-dependent */
    n = SMALL_SIZE + (variant * 10);
    if (n > SIZE) n = SIZE;
    
    /* Select different partitioning scenarios based on variant */
    switch (variant) {
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
        case 8:
            compute_kernels_variant(n, src, dst);
            break;
    }
    
    /* Conditional compilation for additional path variation */
    #ifdef VARIANT_A
    /* Alternative: collapsed loops */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(32) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang worker vector collapse(2)
        for (i = 0; i < n; i++) {
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 7;
            }
        }
    }
    #endif
    
    #ifdef VARIANT_B
    /* Alternative: different clause combination */
    #pragma acc parallel num_gangs(1) num_workers(8) vector_length(1) \
        copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE])
    {
        #pragma acc loop gang
        for (i = 0; i < n; i++) {
            #pragma acc loop worker
            for (j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - 100;
            }
        }
    }
    #endif
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < n; i++) {
        for (j = 0; j < n; j++) {
            checksum += dst[i][j];
        }
    }
    
    /* Print result to create side effect */
    printf("Checksum: %d (variant %d, n=%d)\n", checksum, variant, n);
    
    return 0;
}
