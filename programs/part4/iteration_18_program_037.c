/* This program is designed to trigger the partitioning classification logic
   in GCC's OpenACC neutering/broadcast analysis, specifically the switch
   statement that maps integer codes to descriptive strings for different
   gang/worker/vector partitioning scenarios. */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 1024
#define ITER 8

/* Function prototypes for different partitioning configurations */
void gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);
void fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]);

int main(int argc, char *argv[]) {
    int src[SIZE][SIZE];
    int dst[SIZE][SIZE];
    int variant = 0;
    
    /* Initialize source array with pattern */
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            src[i][j] = i * 1000 + j;
        }
    }
    
    /* Read variant selection from command line or stdin */
    if (argc > 1) {
        variant = atoi(argv[1]) % 8;
    } else {
        printf("Enter variant (0-7): ");
        scanf("%d", &variant);
        variant = variant % 8;
    }
    
    /* Runtime-dependent loop bound */
    int loop_bound = SIZE - variant * 128;
    if (loop_bound < 256) loop_bound = 256;
    
    /* Call different functions based on variant to trigger different
       partitioning classifications in the compiler */
    switch (variant) {
        case 0:
            gang_redundant(loop_bound, src, dst);
            break;
        case 1:
            gang_partitioned(loop_bound, src, dst);
            break;
        case 2:
            worker_partitioned(loop_bound, src, dst);
            break;
        case 3:
            gang_worker_partitioned(loop_bound, src, dst);
            break;
        case 4:
            vector_partitioned(loop_bound, src, dst);
            break;
        case 5:
            gang_vector_partitioned(loop_bound, src, dst);
            break;
        case 6:
            worker_vector_partitioned(loop_bound, src, dst);
            break;
        case 7:
            fully_partitioned(loop_bound, src, dst);
            break;
    }
    
    /* Validation checksum to prevent optimization */
    volatile int checksum = 0;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            checksum += dst[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

/* Function 1: Gang redundant (num_gangs=1, no workers/vectors specified) */
void gang_redundant(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    /* Use kernels construct with only num_gangs(1) */
    #pragma acc kernels copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE]) num_gangs(1)
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * 2;
            }
        }
    }
}

/* Function 2: Gang partitioned (only gangs specified) */
void gang_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    /* Use parallel construct with only num_gangs */
    #pragma acc parallel copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE]) \
        num_gangs(8)
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] + i - j;
            }
        }
    }
}

/* Function 3: Worker partitioned (only workers specified) */
void worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    /* Use kernels construct with only num_workers */
    #pragma acc kernels copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE]) \
        num_workers(4)
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[j][i];  /* Transpose operation */
            }
        }
    }
}

/* Function 4: Gang+worker partitioned */
void gang_worker_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #ifdef VARIANT_A
    /* Variant A: More gangs, fewer workers */
    #pragma acc parallel copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE]) \
        num_gangs(16) num_workers(2)
    #else
    /* Variant B: Fewer gangs, more workers */
    #pragma acc parallel copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE]) \
        num_gangs(4) num_workers(8)
    #endif
    {
        for (int i = 0; i < n; i++) {
            int temp = 0;
            for (int j = 0; j < n; j++) {
                temp += src[i][j];
            }
            for (int j = 0; j < n; j++) {
                dst[i][j] = temp / n;
            }
        }
    }
}

/* Function 5: Vector partitioned */
void vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    /* Only vector_length specified */
    #pragma acc kernels copyin(src[0:SIZE][0:SIZE]) copyout(dst[0:SIZE][0:SIZE]) \
        vector_length(32)
    {
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] * src[i][j];
            }
        }
    }
}

/* Function 6: Gang+vector partitioned */
void gang_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc parallel copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE]) \
        num_gangs(8) vector_length(64)
    {
        #ifdef COLLAPSE
        #pragma acc loop collapse(2)
        #endif
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                /* Complex enough to require analysis */
                dst[i][j] = (src[i][j] * 3) / 2 + (i * j) % 256;
            }
        }
    }
}

/* Function 7: Worker+vector partitioned */
void worker_vector_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    #pragma acc kernels copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE]) \
        num_workers(4) vector_length(128)
    {
        for (int i = 0; i < n; i++) {
            int row_sum = 0;
            for (int j = 0; j < n; j++) {
                row_sum += src[i][j];
            }
            for (int j = 0; j < n; j++) {
                dst[i][j] = src[i][j] - row_sum / n;
            }
        }
    }
}

/* Function 8: Fully partitioned (gangs+workers+vectors) */
void fully_partitioned(int n, int src[SIZE][SIZE], int dst[SIZE][SIZE]) {
    /* Multiple variants to trigger different analysis paths */
    #ifdef VARIANT_1
    #pragma acc parallel copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE]) \
        num_gangs(8) num_workers(2) vector_length(32)
    #elif defined(VARIANT_2)
    #pragma acc kernels copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE]) \
        num_gangs(4) num_workers(4) vector_length(64)
    #else
    #pragma acc parallel copy(src[0:SIZE][0:SIZE], dst[0:SIZE][0:SIZE]) \
        num_gangs(16) num_workers(1) vector_length(16)
    #endif
    {
        /* Triple nested loop for complex partitioning analysis */
        for (int iter = 0; iter < ITER; iter++) {
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < n; j++) {
                    dst[i][j] = (dst[i][j] + src[i][j]) % 1000;
                }
            }
        }
    }
}
