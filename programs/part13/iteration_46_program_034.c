/* Complex OpenACC program designed to trigger partitioning analysis
   and reach the uncovered switch statement in omp-oacc-neuter-broadcast.cc
   lines 335-343 */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to force declare analysis */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP and complex data clauses */
void compute_kernel(int N, int M, int use_condition, int argc) {
    /* Dynamic bounds to prevent constant folding */
    volatile int dyn_N = N;
    volatile int dyn_M = M;
    int actual_N = dyn_N;
    int actual_M = dyn_M;
    
    /* Multi-dimensional arrays for complex access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp[512][512];
    
    /* Reduction variables with different scopes */
    int total_sum = 0;
    int row_sums[512] = {0};
    int col_sums[512] = {0};
    
    /* Initialize arrays */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            src[i][j] = (i * actual_M + j) % 97;
            dst[i][j] = 0;
            tmp[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:actual_N][0:actual_M], \
                          dst[0:actual_N][0:actual_M], \
                          total_sum) \
                   copyin(tmp[0:actual_N][0:actual_M]) \
                   copyout(row_sums[0:actual_N])
    {
        /* FIRST OPENACC REGION: Complex partitioning with gang+worker+vector */
        /* This should trigger analysis of multiple partitioning types */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp) reduction(+:total_sum) \
                firstprivate(actual_N, actual_M) \
                async(1)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < actual_M-1; j++) {
                /* Conditional data access to force broadcast analysis */
                int temp = 0;
                if (use_condition && (i % 2 == 0)) {
                    /* Access with stride pattern */
                    temp = src[i-1][j] + src[i+1][j] + src[i][j-1] + src[i][j+1];
                } else {
                    /* Different access pattern */
                    temp = src[i][j] * 2;
                }
                
                /* Reduction with conditional */
                if (temp > 100) {
                    total_sum += temp;
                }
                
                /* Store to private array then to shared */
                tmp = temp;
                dst[i][j] = tmp;
                
                /* Access global declared data */
                #pragma acc atomic update
                global_matrix[i % 256][j % 256] += temp % 7;
            }
        }
        
        /* Wait for first kernel */
        #pragma acc wait(1)
        
        /* SECOND OPENACC REGION: Different partitioning - gang+vector only */
        /* This should trigger different partitioning code paths */
        #pragma acc kernels loop gang vector collapse(2) \
                reduction(+:global_sum) \
                copy(col_sums[0:actual_M]) \
                present(global_matrix)
        for (int i = 0; i < actual_N; i += 2) {
            for (int j = 0; j < actual_M; j += 2) {
                /* Different computation pattern */
                int local_sum = 0;
                for (int di = 0; di < 2 && i+di < actual_N; di++) {
                    for (int dj = 0; dj < 2 && j+dj < actual_M; dj++) {
                        local_sum += dst[i+di][j+dj];
                    }
                }
                
                /* Conditional update based on argc (dynamic) */
                if (argc > 2) {
                    col_sums[j % actual_M] += local_sum;
                }
                
                /* Update global sum */
                #pragma acc atomic
                global_sum += local_sum % 13;
            }
        }
        
        /* THIRD OPENACC REGION: Worker+vector partitioning */
        /* Nested loops with manual partitioning */
        #pragma acc parallel loop gang
        for (int block = 0; block < 4; block++) {
            int start = block * (actual_N / 4);
            int end = (block == 3) ? actual_N : (block + 1) * (actual_N / 4);
            
            #pragma acc loop worker vector reduction(+:row_sums[:actual_N])
            for (int i = start; i < end; i++) {
                int row_sum = 0;
                #pragma acc loop vector
                for (int j = 0; j < actual_M; j++) {
                    /* Complex conditional access */
                    if ((i + j) % 3 == 0) {
                        row_sum += src[i][j];
                    } else if ((i * j) % 5 == 0) {
                        row_sum += dst[i][j];
                    }
                }
                row_sums[i] = row_sum;
            }
        }
    }
    
    /* Mixed OpenMP outside OpenACC region */
    #pragma omp parallel for reduction(+:total_sum)
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_M; j++) {
            /* Final host-side computation */
            total_sum += dst[i][j] % 11;
        }
    }
    
    /* Verification output */
    printf("Checksum: total_sum=%d, global_sum=%d, row_sums[0]=%d, col_sums[0]=%d\n",
           total_sum, global_sum, 
           actual_N > 0 ? row_sums[0] : 0,
           actual_M > 0 ? col_sums[0] : 0);
}

/* Helper function with another OpenACC region */
void helper_compute(int size) {
    int arr[256][256];
    int local_reduction = 0;
    
    #pragma acc parallel loop collapse(2) gang worker \
            private(arr) reduction(+:local_reduction) \
            copyin(size)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* Fully partitioned access pattern */
            int val = (i * size + j) % 19;
            arr[i][j] = val;
            local_reduction += val;
            
            /* Access global data with complex indexing */
            if ((i + j) % 7 == 0) {
                #pragma acc atomic
                global_matrix[i % 256][j % 256] += val;
            }
        }
    }
    
    printf("Helper computed: %d\n", local_reduction);
}

int main(int argc, char **argv) {
    /* Dynamic sizes based on arguments */
    int N = argc > 1 ? atoi(argv[1]) : 512;
    int M = argc > 2 ? atoi(argv[2]) : 512;
    int use_condition = argc > 3 ? atoi(argv[3]) : 1;
    
    /* Initialize global data */
    #pragma acc parallel loop collapse(2) present(global_matrix)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = 0;
        }
    }
    
    /* Main computation with complex partitioning scenarios */
    compute_kernel(N, M, use_condition, argc);
    
    /* Additional computation with different parameters */
    helper_compute(N % 256);
    
    /* Final OpenMP region */
    int final_check = 0;
    #pragma omp parallel for reduction(+:final_check)
    for (int i = 0; i < 1000; i++) {
        final_check += i % 17;
    }
    
    printf("Final check: %d\n", final_check);
    
    return 0;
}
