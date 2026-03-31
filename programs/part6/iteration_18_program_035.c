/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables to create complex scoping */
int global_redundant = 42;
float global_matrix[N][M];
double global_partial_sums[GANGS][WORKERS];

/* Function to initialize data */
void init_data() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) * 0.001f;
        }
    }
}

/* Main function with complex nested parallelism */
int main() {
    int i, j, g, w, v;
    float local_scalar = 3.14f;
    double reduction_result = 0.0;
    
    /* 2D arrays for stencil operations */
    float input_grid[N][M];
    float output_grid[N][M];
    float temp_buffer[N][M];
    
    /* Initialize all arrays */
    init_data();
    
    #pragma acc data copyin(global_matrix) \
                     copyout(output_grid) \
                     create(input_grid, temp_buffer)
    {
        /* Copy to local array */
        #pragma acc parallel loop gang vector collapse(2)
        for (i = 0; i < N; i++) {
            for (j = 0; j < M; j++) {
                input_grid[i][j] = global_matrix[i][j];
            }
        }
        
        /* ============================================
           CASE 0: "gang redundant"
           Global scalar that's read-only for all gangs
           ============================================ */
        #pragma acc parallel num_gangs(GANGS) \
                             copy(reduction_result) \
                             present(input_grid)
        {
            int gang_id = __pgi_gangidx();
            int private_gang_var = gang_id * 100;
            
            /* global_redundant is gang redundant (case 0) */
            private_gang_var += global_redundant;
            
            #pragma acc loop reduction(+:reduction_result)
            for (i = 0; i < N/GANGS; i++) {
                int global_i = gang_id * (N/GANGS) + i;
                reduction_result += input_grid[global_i][0];
            }
        }
        
        /* ============================================
           CASE 1: "gang partitioned"
           Scalar partitioned at gang level only
           ============================================ */
        #pragma acc parallel num_gangs(GANGS) \
                             present(input_grid, output_grid)
        {
            int gang_partitioned = __pgi_gangidx();  /* Case 1 */
            float gang_private_accum = 0.0f;
            
            #pragma acc loop gang
            for (i = 0; i < N; i++) {
                gang_private_accum += input_grid[i][gang_partitioned % M];
            }
            
            /* Use the result to prevent optimization */
            if (gang_partitioned == 0) {
                output_grid[0][0] = gang_private_accum;
            }
        }
        
        /* ============================================
           CASE 2: "worker partitioned" 
           CASE 3: "gang+worker partitioned"
           ============================================ */
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
                             present(input_grid, temp_buffer)
        {
            int gang_id = __pgi_gangidx();
            int worker_id = __pgi_workeridx();
            
            /* worker_partitioned is worker partitioned (case 2) */
            int worker_partitioned = worker_id * 10;
            
            /* gw_partitioned is gang+worker partitioned (case 3) */
            int gw_partitioned = gang_id * 1000 + worker_id * 100;
            
            #pragma acc loop worker
            for (j = 0; j < M/WORKERS; j++) {
                int global_j = worker_id * (M/WORKERS) + j;
                float worker_sum = 0.0f;
                
                #pragma acc loop vector
                for (i = 0; i < N; i++) {
                    worker_sum += input_grid[i][global_j];
                }
                
                temp_buffer[worker_id][global_j % N] = worker_sum + 
                                                       worker_partitioned + 
                                                       gw_partitioned;
            }
        }
        
        /* ============================================
           CASE 4: "vector partitioned"
           CASE 5: "gang+vector partitioned"
           ============================================ */
        #pragma acc kernels present(input_grid, output_grid)
        {
            /* vector_partitioned is vector partitioned (case 4) */
            #pragma acc loop gang vector
            for (i = 0; i < N; i++) {
                float vector_partitioned[N];  /* Each vector gets its own */
                
                #pragma acc loop vector
                for (j = 0; j < M; j++) {
                    vector_partitioned[j % N] = input_grid[i][j] * 2.0f;
                    output_grid[i][j] = vector_partitioned[j % N];
                }
            }
            
            /* gv_partitioned is gang+vector partitioned (case 5) */
            #pragma acc loop gang(static:1) vector
            for (i = 0; i < N; i++) {
                int gv_partitioned = i * VECTOR_LEN;
                
                #pragma acc loop vector
                for (j = 0; j < M; j++) {
                    output_grid[i][j] += sinf(gv_partitioned + j) * 0.1f;
                }
            }
        }
        
        /* ============================================
           CASE 6: "worker+vector partitioned"
           CASE 7: "fully partitioned"
           ============================================ */
        #pragma acc parallel num_gangs(GANGS) \
                             num_workers(WORKERS) \
                             vector_length(VECTOR_LEN) \
                             present(input_grid, output_grid, temp_buffer)
        {
            int gang_id = __pgi_gangidx();
            int worker_id = __pgi_workeridx();
            int vector_id = __pgi_vectoridx();
            
            /* wv_partitioned is worker+vector partitioned (case 6) */
            int wv_partitioned[VECTOR_LEN];
            wv_partitioned[vector_id] = worker_id * 1000 + vector_id;
            
            /* fully_partitioned is fully partitioned (case 7) */
            int fully_partitioned = gang_id * 1000000 + 
                                   worker_id * 10000 + 
                                   vector_id * 100;
            
            /* Complex stencil computation with multi-dimensional access */
            #pragma acc loop gang worker vector collapse(2)
            for (i = 1; i < N-1; i++) {
                for (j = 1; j < M-1; j++) {
                    /* 5-point stencil */
                    float stencil_sum = 
                        input_grid[i-1][j] + input_grid[i+1][j] +
                        input_grid[i][j-1] + input_grid[i][j+1] +
                        input_grid[i][j];
                    
                    /* Use all partitioned variables to influence computation */
                    stencil_sum += wv_partitioned[vector_id % VECTOR_LEN] * 0.0001f;
                    stencil_sum += fully_partitioned * 0.000001f;
                    
                    /* Conditional partitioning based on runtime values */
                    if (stencil_sum > 100.0f) {
                        temp_buffer[i][j] = stencil_sum * 0.5f;
                    } else {
                        temp_buffer[i][j] = stencil_sum * 2.0f;
                    }
                    
                    output_grid[i][j] = temp_buffer[i][j] / 4.0f;
                }
            }
            
            /* Nested parallelism with different clauses */
            #pragma acc loop gang worker
            for (g = 0; g < GANGS; g++) {
                float gang_worker_private = g * worker_id * 1.5f;
                
                #pragma acc loop vector
                for (v = 0; v < VECTOR_LEN/2; v++) {
                    int idx = (gang_id * WORKERS + worker_id) * VECTOR_LEN + v;
                    if (idx < N && idx < M) {
                        output_grid[idx][idx] += gang_worker_private + v;
                    }
                }
            }
        }
        
        /* ============================================
           Mixed directives for comprehensive coverage
           ============================================ */
        #pragma acc kernels loop gang worker vector \
                               independent collapse(3) \
                               present(output_grid)
        for (int g = 0; g < GANGS; g++) {
            for (int w = 0; w < WORKERS; w++) {
                for (int v = 0; v < VECTOR_LEN; v++) {
                    int idx = (g * WORKERS + w) * VECTOR_LEN + v;
                    if (idx < N && idx < M) {
                        /* Access pattern that forces different partitioning */
                        output_grid[idx][idx % M] *= 
                            (1.0f + (g * 0.01f) + (w * 0.001f) + (v * 0.0001f));
                    }
                }
            }
        }
    }
    
    /* Final reduction on host to verify computation */
    double final_sum = 0.0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            final_sum += output_grid[i][j];
        }
    }
    
    printf("Final checksum: %f\n", final_sum);
    printf("Reduction result: %f\n", reduction_result);
    
    return 0;
}
