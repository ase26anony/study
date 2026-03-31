/* Complex OpenACC program designed to trigger execution of the partitioning
   type string lookup function in omp-oacc-neuter-broadcast.cc (lines 335-343).
   This program uses multiple OpenACC constructs with explicit data clauses
   and partitioning scenarios to force the compiler to analyze and represent
   various data partitioning strategies. */

#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data to test declare directive */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP and complex data access patterns */
void compute_kernel(int N, int M, int use_condition, int *argc_ptr) {
    int i, j, k;
    volatile int dynamic_bound = *argc_ptr; /* Prevent constant folding */
    int actual_N = N + (dynamic_bound % 4);
    
    /* 2D arrays for stencil operation */
    int src[512][512];
    int dst[512][512];
    int tmp_sum = 0;
    
    /* Initialize arrays with pattern */
    #pragma omp parallel for collapse(2) private(i, j)
    for (i = 0; i < actual_N; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = i * M + j;
            dst[i][j] = 0;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:actual_N][0:M], dst[0:actual_N][0:M]) \
                     copyin(actual_N, M) copy(tmp_sum)
    {
        /* FIRST OPENACC PARALLEL REGION: Complex partitioning scenario
           This uses gang+worker+vector with private and reduction clauses */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) reduction(+:tmp_sum) \
                present(src, dst, actual_N, M)
        for (i = 1; i < actual_N-1; i++) {
            for (j = 1; j < M-1; j++) {
                int temp = 0; /* private variable */
                
                /* Conditional data access pattern */
                if (use_condition && (i % 3 == 0)) {
                    /* Complex stencil with data-dependent access */
                    temp += src[i-1][j] * 2;
                    temp += src[i+1][j] * 2;
                    temp += src[i][j-1] * 3;
                    temp += src[i][j+1] * 3;
                    temp /= 10;
                } else {
                    /* Different access pattern */
                    for (k = -1; k <= 1; k++) {
                        temp += src[i+k][j] + src[i][j+k];
                    }
                    temp /= 6;
                }
                
                dst[i][j] = temp;
                tmp_sum += temp; /* reduction */
            }
        }
        
        /* SECOND OPENACC KERNELS REGION: Different partitioning approach
           Only gang+vector partitioning (no worker) */
        #pragma acc kernels loop gang vector collapse(2) \
                present(dst, actual_N, M)
        for (i = 0; i < actual_N; i++) {
            for (j = 0; j < M; j++) {
                /* Another conditional operation */
                if (j % 2 == 0) {
                    dst[i][j] += i * j;
                } else {
                    dst[i][j] -= i;
                }
            }
        }
        
        /* THIRD: Worker-only partitioning scenario on a 1D slice */
        #pragma acc parallel loop worker private(i, j) \
                present(dst, actual_N, M)
        for (i = 0; i < actual_N; i++) {
            int row_sum = 0;
            #pragma acc loop vector reduction(+:row_sum)
            for (j = 0; j < M; j++) {
                row_sum += dst[i][j];
            }
            /* Store result in global matrix (uses declare directive) */
            if (i < 256 && M >= 256) {
                #pragma acc atomic update
                global_matrix[i][i % 256] += row_sum;
            }
        }
    }
    
    /* Update global sum with local result */
    #pragma acc atomic update
    global_sum += tmp_sum;
}

/* Function using global declared data */
void use_declared_data(int N) {
    int i, j;
    
    /* Access declared device data with full partitioning */
    #pragma acc parallel loop gang worker vector collapse(2) \
            present(global_matrix, global_sum)
    for (i = 0; i < N && i < 256; i++) {
        for (j = 0; j < N && j < 256; j++) {
            int val = global_matrix[i][j];
            /* Conditional update with complex expression */
            if ((i + j) % 7 == 0) {
                #pragma acc atomic update
                global_sum += val * 2;
            } else if ((i * j) % 5 == 0) {
                #pragma acc atomic update
                global_sum -= val;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int N = 512;
    int M = 512;
    
    /* Dynamic bounds based on argc to prevent constant folding */
    if (argc > 1) {
        N = atoi(argv[1]);
        if (N < 64) N = 64;
        if (N > 512) N = 512;
    }
    if (argc > 2) {
        M = atoi(argv[2]);
        if (M < 64) M = 64;
        if (M > 512) M = 512;
    }
    
    printf("Starting OpenACC partitioning test with N=%d, M=%d\n", N, M);
    
    /* Initialize global matrix on host */
    #pragma omp parallel for collapse(2)
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = i + j * 2;
        }
    }
    
    /* Copy global data to device */
    #pragma acc update device(global_matrix[0:256][0:256], global_sum)
    
    /* First computation with condition */
    compute_kernel(N, M, 1, &argc);
    
    /* Second computation without condition (different path) */
    compute_kernel(N/2, M/2, 0, &argc);
    
    /* Use declared data with different partitioning */
    use_declared_data(N);
    
    /* Mixed OpenMP region outside OpenACC */
    int host_array[1000];
    #pragma omp parallel for
    for (int i = 0; i < 1000; i++) {
        host_array[i] = i * omp_get_thread_num();
    }
    
    /* Final verification */
    #pragma acc update host(global_sum)
    
    /* Simple checksum to prevent dead code elimination */
    int final_check = 0;
    #pragma acc parallel loop reduction(+:final_check) \
            present(global_matrix)
    for (int i = 0; i < 256 && i < N; i++) {
        for (int j = 0; j < 256 && j < M; j++) {
            final_check += global_matrix[i][j] % 100;
        }
    }
    
    printf("Results: global_sum=%d, final_check=%d\n", global_sum, final_check);
    
    return 0;
}
