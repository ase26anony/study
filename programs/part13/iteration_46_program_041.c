/* This program is designed to trigger execution of the partitioning type
   string lookup function in GCC's omp-oacc-neuter-broadcast.cc.
   It uses complex OpenACC constructs with various data clauses and
   partitioning scenarios to force the compiler to analyze and represent
   data partitioning strategies. */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to test declare directive */
#pragma acc declare create(global_matrix)
static int global_matrix[256][256];

/* Function with mixed OpenACC/OpenMP constructs */
void compute_kernel(int N, int M, int argc) {
    /* Use argc to prevent constant folding */
    volatile int dynamic_bound = (argc > 1) ? atoi("512") : 256;
    int actual_N = (N < dynamic_bound) ? N : dynamic_bound;
    
    /* Multi-dimensional arrays */
    int src[512][512];
    int dst[512][512];
    int tmp_array[512][512];
    
    /* Reduction variables */
    int total_sum = 0;
    int row_sums[512] = {0};
    
    /* Initialize arrays */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < actual_N; i++) {
        for (int j = 0; j < actual_N; j++) {
            src[i][j] = i * actual_N + j;
            dst[i][j] = 0;
            tmp_array[i][j] = 0;
            global_matrix[i][j] = (i + j) % 256;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src, dst) copyin(global_matrix) create(tmp_array)
    {
        /* First OpenACC parallel region with complex partitioning */
        /* This should trigger analysis of gang+worker+vector partitioning */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(tmp_array) reduction(+:total_sum) \
                firstprivate(actual_N)
        for (int i = 1; i < actual_N-1; i++) {
            for (int j = 1; j < actual_N-1; j++) {
                /* Private temporary variable */
                int temp = 0;
                
                /* Conditional data access pattern */
                if ((i + j) % 3 == 0) {
                    /* Stencil operation with data-dependent access */
                    temp = src[i-1][j] + src[i+1][j] + 
                           src[i][j-1] + src[i][j+1];
                } else if ((i * j) % 5 == 0) {
                    /* Different access pattern */
                    temp = src[i][j] * 2;
                } else {
                    temp = src[i][j];
                }
                
                /* Store to private array then to shared */
                tmp_array[i][j] = temp;
                dst[i][j] = tmp_array[i][j] + global_matrix[i][j];
                
                /* Reduction with conditional */
                if (dst[i][j] > 100) {
                    total_sum += dst[i][j];
                }
            }
        }
        
        /* Second OpenACC kernels region with different partitioning */
        /* This should trigger gang+vector partitioning analysis */
        #pragma acc kernels loop gang vector collapse(2) \
                private(row_sums) firstprivate(actual_N)
        for (int i = 0; i < actual_N; i++) {
            for (int j = 0; j < actual_N; j++) {
                /* Worker-like computation without explicit worker clause */
                int local_sum = 0;
                
                /* Nested loop within parallel region */
                #pragma acc loop seq
                for (int k = 0; k < 4; k++) {
                    local_sum += dst[i][j] >> k;
                }
                
                /* Conditional update */
                if (local_sum % 2 == 0) {
                    dst[i][j] = local_sum;
                }
                
                /* Simulate row-wise reduction */
                #pragma acc atomic update
                row_sums[i] += dst[i][j];
            }
        }
        
        /* Third region: vector-only partitioning scenario */
        #pragma acc parallel loop vector collapse(2) \
                firstprivate(actual_N)
        for (int i = 0; i < actual_N; i++) {
            for (int j = 0; j < actual_N; j++) {
                /* Vector partitioned operation */
                dst[i][j] = dst[i][j] * 2 - src[i][j];
                
                /* Access global declared data */
                global_matrix[i][j] += dst[i][j] % 100;
            }
        }
    }
    
    /* Mixed OpenMP region outside OpenACC */
    int host_array[512];
    #pragma omp parallel for private(total_sum)
    for (int i = 0; i < actual_N; i++) {
        int local_sum = 0;
        for (int j = 0; j < actual_N; j++) {
            local_sum += dst[i][j];
        }
        host_array[i] = local_sum;
    }
    
    /* Final verification */
    int final_check = 0;
    for (int i = 0; i < actual_N; i++) {
        final_check += host_array[i];
    }
    
    printf("Computation complete. Checksum: %d, Total sum: %d\n", 
           final_check, total_sum);
}

/* Helper function with another partitioning scenario */
void nested_partitioning(int size) {
    int array[256][256];
    
    #pragma acc parallel loop gang worker collapse(2) \
            firstprivate(size)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* Worker partitioned computation */
            int val = i * j;
            
            /* Vector-style operation within worker */
            #pragma acc loop vector
            for (int k = 0; k < 8; k++) {
                val += k;
            }
            
            array[i][j] = val;
            
            /* Conditional with complex data flow */
            if ((i + j) % 7 == 0) {
                #pragma acc atomic update
                array[i][j] *= 2;
            }
        }
    }
    
    /* Fully partitioned scenario */
    #pragma acc parallel loop gang worker vector collapse(2) \
            firstprivate(size)
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            /* Each iteration fully independent */
            array[i][j] = (array[i][j] * 3) / 2;
        }
    }
}

int main(int argc, char **argv) {
    /* Use argc to make bounds dynamic */
    int N = (argc > 1) ? atoi(argv[1]) : 256;
    if (N > 512) N = 512;
    if (N < 64) N = 64;
    
    printf("Starting OpenACC partitioning test with N=%d\n", N);
    
    /* Call main computation kernel */
    compute_kernel(N, N, argc);
    
    /* Call helper function for additional scenarios */
    nested_partitioning(N/2);
    
    /* Final OpenMP region */
    #pragma omp parallel
    {
        int tid = omp_get_thread_num();
        #pragma omp single
        printf("OpenMP thread %d: Test complete.\n", tid);
    }
    
    return 0;
}
