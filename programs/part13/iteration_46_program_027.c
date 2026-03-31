/* test-omp-oacc-neuter-broadcast.c
 * Designed to trigger execution of the partitioning type string lookup function
 * in omp-oacc-neuter-broadcast.cc (lines 335-343) through complex OpenACC usage.
 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to stress declare directive partitioning */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP to increase analysis complexity */
void process_data(int N, int M, int use_condition, int argc) {
    int i, j, k;
    volatile int dynamic_bound = N; /* Prevent constant folding */
    
    /* 2D arrays for nested loop computations */
    int src[512][512];
    int dst[512][512];
    int tmp_sum = 0;
    
    /* Initialize arrays with pattern */
    #pragma omp parallel for collapse(2) private(i, j)
    for (i = 0; i < dynamic_bound; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = i * M + j;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:N][0:M], dst[0:N][0:M]) copyin(dynamic_bound)
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning scenario
         * Multiple clauses to force partitioning analysis */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) reduction(+:tmp_sum) \
                firstprivate(use_condition) async(1)
        for (i = 1; i < dynamic_bound-1; i++) {
            for (j = 1; j < M-1; j++) {
                int temp = 0; /* private variable */
                
                /* Conditional data access to force broadcast/neutering decisions */
                if (use_condition && (i % 2 == 0)) {
                    /* Access pattern that may require different partitioning */
                    temp = src[i-1][j] + src[i+1][j] + src[i][j-1] + src[i][j+1];
                } else {
                    temp = src[i][j] * 2;
                }
                
                /* Reduction with conditional */
                if (temp > 100) tmp_sum += 1;
                
                /* Write to dst with another condition */
                dst[i][j] = (argc > 2) ? temp : temp / 2;
                
                /* Access global declared data */
                #pragma acc atomic update
                global_sum += (temp % 10);
            }
        }
        
        /* Wait for first kernel then launch second with DIFFERENT partitioning */
        #pragma acc wait(1)
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning approach
         * Only gang+vector, no worker partitioning */
        #pragma acc kernels loop gang vector collapse(2) \
                reduction(max:tmp_sum) copy(tmp_sum)
        for (i = 0; i < dynamic_bound; i += 2) {
            for (j = 0; j < M; j += 2) {
                int local_max = 0;
                
                /* Nested loop inside parallel region */
                for (k = 0; k < 2; k++) {
                    if (i+k < dynamic_bound && j+k < M) {
                        local_max = (dst[i+k][j+k] > local_max) ? 
                                   dst[i+k][j+k] : local_max;
                    }
                }
                
                /* Conditional update */
                if (local_max > tmp_sum) {
                    tmp_sum = local_max;
                }
                
                /* Access global matrix */
                if (i < 256 && j < 256) {
                    #pragma acc atomic update
                    global_matrix[i][j] += local_max;
                }
            }
        }
        
        /* THIRD: Worker-only partitioning scenario */
        int worker_local[128];
        #pragma acc parallel loop worker private(i) firstprivate(dynamic_bound)
        for (i = 0; i < 128; i++) {
            worker_local[i] = 0;
            if (i < dynamic_bound) {
                for (j = 0; j < M; j++) {
                    worker_local[i] += src[i][j] % 256;
                }
            }
        }
        
        /* Use worker results */
        #pragma acc parallel loop gang vector reduction(+:tmp_sum)
        for (i = 0; i < 128; i++) {
            tmp_sum += worker_local[i];
        }
    }
    
    /* Mixed OpenMP outside OpenACC region */
    int verify_array[512];
    #pragma omp parallel for private(i)
    for (i = 0; i < dynamic_bound; i++) {
        verify_array[i] = 0;
        for (j = 0; j < M; j++) {
            verify_array[i] += dst[i][j];
        }
    }
    
    /* Final verification */
    int final_check = 0;
    for (i = 0; i < dynamic_bound; i++) {
        final_check += verify_array[i];
    }
    printf("Intermediate checksum: %d\n", final_check);
}

/* Another function with vector partitioning focus */
void vector_heavy_computation(int N) {
    float array[1024];
    float result[1024];
    int i;
    
    /* Initialize */
    for (i = 0; i < 1024; i++) {
        array[i] = i * 0.5f;
    }
    
    /* Vector partitioned loop */
    #pragma acc parallel loop vector_length(32) \
                private(i) copy(array, result)
    for (i = 0; i < 1024; i++) {
        float temp = array[i];
        /* Simulated vector operation */
        for (int lane = 0; lane < 4; lane++) {
            temp += array[(i + lane) % 1024] * 0.1f;
        }
        result[i] = temp;
        
        /* Conditional that may affect partitioning */
        if (i % N == 0) {
            result[i] *= 2.0f;
        }
    }
}

int main(int argc, char **argv) {
    int N = 512, M = 512;
    
    /* Dynamic bounds from arguments to prevent constant propagation */
    if (argc > 1) N = atoi(argv[1]);
    if (argc > 2) M = atoi(argv[2]);
    if (N > 512) N = 512;
    if (M > 512) M = 512;
    
    printf("Testing OpenACC partitioning with N=%d, M=%d\n", N, M);
    
    /* Initialize global declared data */
    #pragma acc parallel loop collapse(2) copy(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = i + j;
        }
    }
    
    /* Process with different conditionals */
    process_data(N, M, argc > 3, argc);
    
    /* Additional vector-heavy computation */
    vector_heavy_computation(N);
    
    /* Final reduction on global data */
    int final_global_sum = 0;
    #pragma acc parallel loop reduction(+:final_global_sum) \
                copy(final_global_sum) copyin(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            final_global_sum += global_matrix[i][j];
        }
    }
    
    printf("Final global sum: %d\n", final_global_sum);
    printf("Test completed.\n");
    
    return 0;
}
