/* Complex OpenACC program to trigger partitioning type string lookups */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Persistent device data - may trigger declare-time partitioning analysis */
#pragma acc declare create(global_matrix, global_sum)
static int global_matrix[256][256];
static int global_sum = 0;

/* Function with mixed OpenACC/OpenMP constructs */
void process_data(int N, int M, int use_conditional) {
    int i, j, k;
    int local_sum = 0;
    volatile int volatile_bound = M; /* Prevent constant folding */
    
    /* 2D arrays with different access patterns */
    int src[512][512];
    int dst[512][512];
    int tmp_buffer[512][512];
    
    /* Initialize arrays with pattern */
    #pragma omp parallel for collapse(2) private(i, j)
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            src[i][j] = i * M + j;
            dst[i][j] = 0;
            tmp_buffer[i][j] = -1;
        }
    }
    
    /* OpenACC data region with copy clauses */
    #pragma acc data copy(src[0:N][0:M], dst[0:N][0:M]) \
                     copyin(tmp_buffer[0:N][0:M]) \
                     copy(global_matrix[0:256][0:256]) \
                     copyout(local_sum)
    {
        /* Complex parallel region with multiple clauses - likely to trigger partitioning analysis */
        #pragma acc parallel loop gang worker vector collapse(2) \
                private(k) firstprivate(volatile_bound) reduction(+:local_sum) \
                present(src, dst, tmp_buffer, global_matrix)
        for (i = 0; i < N; i++) {
            for (j = 0; j < volatile_bound; j++) {
                int temp = 0;
                
                /* Conditional data access - may affect partitioning decisions */
                if (use_conditional && (i % 2 == 0)) {
                    /* Different access pattern for even rows */
                    for (k = 0; k < 4; k++) {
                        if (j + k < M) {
                            temp += src[i][j + k] * global_matrix[k % 256][j % 256];
                        }
                    }
                    dst[i][j] = temp;
                } else {
                    /* Stencil operation for odd rows */
                    int neighbors = 0;
                    int count = 0;
                    
                    #pragma acc loop vector reduction(+:neighbors, count)
                    for (k = -1; k <= 1; k++) {
                        if (i + k >= 0 && i + k < N) {
                            neighbors += src[i + k][j];
                            count++;
                        }
                    }
                    dst[i][j] = (count > 0) ? neighbors / count : 0;
                }
                
                /* Reduction with conditional */
                if (dst[i][j] > 100) {
                    local_sum += dst[i][j];
                }
                
                /* Worker-private temporary storage */
                tmp_buffer[i][j] = dst[i][j] * 2;
            }
        }
        
        /* Second kernels region with different partitioning approach */
        int row_sums[512] = {0};
        
        #pragma acc kernels loop gang vector independent \
                copy(row_sums[0:N]) present(dst, global_matrix)
        for (i = 0; i < N; i++) {
            int row_sum = 0;
            int private_temp[16]; /* Array private to each iteration */
            
            #pragma acc loop vector reduction(+:row_sum)
            for (j = 0; j < M; j++) {
                /* Complex access pattern mixing arrays */
                row_sum += dst[i][j];
                
                /* Conditional update of global matrix */
                if (dst[i][j] % 3 == 0) {
                    #pragma acc atomic update
                    global_matrix[i % 256][j % 256] += dst[i][j] % 256;
                }
                
                /* Use private array */
                private_temp[j % 16] = dst[i][j] % 16;
                row_sum += private_temp[j % 16];
            }
            
            row_sums[i] = row_sum;
            
            /* Nested parallelism with different clause combination */
            #pragma acc loop worker
            for (j = 0; j < 8; j++) {
                int worker_local = 0;
                #pragma acc loop vector reduction(+:worker_local)
                for (k = 0; k < 8; k++) {
                    worker_local += private_temp[(j + k) % 16];
                }
                /* Potential gang+worker+vector partitioning scenario */
                row_sums[i] += worker_local;
            }
        }
        
        /* Third region: vector-only partitioning scenario */
        int final_checksum = 0;
        
        #pragma acc parallel loop vector_length(32) \
                private(i, j) reduction(+:final_checksum) \
                present(row_sums, dst)
        for (i = 0; i < N * M; i++) {
            int idx_i = i / M;
            int idx_j = i % M;
            
            /* Fully partitioned access pattern */
            final_checksum += dst[idx_i][idx_j] + row_sums[idx_i];
            
            /* Conditional that may require broadcast */
            if (idx_j % 8 == 0) {
                final_checksum += global_matrix[idx_i % 256][idx_j % 256];
            }
        }
        
        local_sum += final_checksum;
    }
    
    /* Update global sum with atomic to prevent optimization */
    #pragma omp atomic
    global_sum += local_sum;
}

/* Another function with different OpenACC construct nesting */
void nested_partitioning_test(int size) {
    int matrix[128][128];
    int i, j;
    
    /* Initialize */
    #pragma acc parallel loop collapse(2) gang worker
    for (i = 0; i < 128; i++) {
        for (j = 0; j < 128; j++) {
            matrix[i][j] = i * 128 + j;
        }
    }
    
    /* Complex reduction with multiple private variables */
    int total = 0;
    int partial_sums[16] = {0};
    
    #pragma acc data copy(matrix) create(partial_sums) copyout(total)
    {
        /* This construct may trigger gang+worker partitioned analysis */
        #pragma acc parallel loop gang worker num_gangs(4) num_workers(8) \
                private(i, j) reduction(+:total)
        for (int g = 0; g < 4; g++) {
            for (int w = 0; w < 8; w++) {
                int worker_sum = 0;
                int private_matrix[8][8];
                
                #pragma acc loop vector collapse(2)
                for (i = 0; i < 8; i++) {
                    for (j = 0; j < 8; j++) {
                        int gi = g * 32 + w * 4 + i;
                        int gj = w * 16 + j;
                        if (gi < 128 && gj < 128) {
                            private_matrix[i][j] = matrix[gi][gj];
                            worker_sum += private_matrix[i][j];
                        }
                    }
                }
                
                /* Worker reduction with private array */
                int vector_sum = 0;
                #pragma acc loop vector reduction(+:vector_sum)
                for (i = 0; i < 8; i++) {
                    for (j = 0; j < 8; j++) {
                        vector_sum += private_matrix[i][j];
                    }
                }
                
                partial_sums[g * 4 + w % 4] = worker_sum + vector_sum;
                total += worker_sum;
            }
        }
        
        /* Final reduction across partial sums */
        #pragma acc parallel loop vector reduction(+:total)
        for (i = 0; i < 16; i++) {
            total += partial_sums[i];
        }
    }
    
    printf("Nested test total: %d\n", total);
}

int main(int argc, char **argv) {
    /* Dynamic bounds to prevent compile-time optimization */
    int N = argc > 1 ? atoi(argv[1]) : 256;
    int M = argc > 2 ? atoi(argv[2]) : 256;
    int use_cond = argc > 3 ? atoi(argv[3]) : 1;
    
    printf("Starting OpenACC partitioning test with N=%d, M=%d\n", N, M);
    
    /* Initialize global matrix */
    #pragma acc parallel loop collapse(2) gang vector
    for (int i = 0; i < 256; i++) {
        for (int j = 0; j < 256; j++) {
            global_matrix[i][j] = (i + j) % 100;
        }
    }
    
    /* Process with different partitioning scenarios */
    process_data(N, M, use_cond);
    
    /* Additional test with different parameters */
    process_data(N/2, M/2, 0);
    
    /* Nested partitioning test */
    nested_partitioning_test(128);
    
    /* Pure OpenMP section to mix pragmas */
    int omp_array[1000];
    #pragma omp parallel for
    for (int i = 0; i < 1000; i++) {
        omp_array[i] = i * 2;
    }
    
    /* Final verification output */
    printf("Global checksum: %d\n", global_sum);
    
    /* Use result to prevent dead code elimination */
    if (global_sum > 1000000) {
        printf("Result is large\n");
    }
    
    return 0;
}
