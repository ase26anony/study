/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables at different scopes */
int global_scalar = 42;
float global_array[N][M];
static int file_static = 100;

/* Function to initialize arrays */
void init_arrays() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_array[i][j] = (float)(i * M + j);
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    int host_scalar = 10;
    float host_array[N][M];
    int reduction_result = 0;
    
    /* Initialize data */
    init_arrays();
    memcpy(host_array, global_array, sizeof(global_array));
    
    printf("Starting OpenACC partitioning tests...\n");
    
    /* ============================================
       CASE 0: Gang Redundant
       A variable that is the same for all gangs
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copyin(global_scalar) \
        copyout(host_array[0:N][0:M])
    {
        int gang_id = __pgi_gangidx();
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* global_scalar is gang redundant - same value for all gangs */
            float scale_factor = global_scalar + gang_id;
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                host_array[i][j] = global_array[i][j] * scale_factor;
            }
        }
    }
    
    /* ============================================
       CASE 1: Gang Partitioned  
       Variable partitioned across gangs only
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) \
        private(host_scalar) copy(host_array[0:N][0:M])
    {
        /* host_scalar becomes gang partitioned */
        host_scalar = __pgi_gangidx() * 100;
        
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                host_array[i][j] += host_scalar;
            }
        }
    }
    
    /* ============================================
       CASE 2: Worker Partitioned
       Using worker-only parallelism
       ============================================ */
    int worker_var = 50;
    #pragma acc parallel num_workers(WORKERS) vector_length(1) \
        copy(worker_var) copy(host_array[0:N][0:M])
    {
        /* worker_var becomes worker partitioned */
        worker_var = __pgi_workeridx();
        
        #pragma acc loop worker
        for (int i = 0; i < N; i += WORKERS) {
            int idx = i + worker_var;
            if (idx < N) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    host_array[idx][j] += worker_var;
                }
            }
        }
    }
    
    /* ============================================
       CASE 3: Gang+Worker Partitioned
       Nested gang-worker parallelism
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        vector_length(VECTOR_LEN) copy(host_array[0:N][0:M])
    {
        int gw_partitioned = __pgi_gangidx() * 1000 + __pgi_workeridx();
        
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                int start_i = (g * WORKERS + w) * (N / (GANGS * WORKERS));
                int end_i = start_i + (N / (GANGS * WORKERS));
                
                #pragma acc loop vector
                for (int i = start_i; i < end_i && i < N; i++) {
                    #pragma acc loop seq
                    for (int j = 0; j < M; j++) {
                        host_array[i][j] += gw_partitioned;
                    }
                }
            }
        }
    }
    
    /* ============================================
       CASE 4: Vector Partitioned
       Vector-only parallelism
       ============================================ */
    float vector_array[VECTOR_LEN];
    #pragma acc parallel vector_length(VECTOR_LEN) \
        copy(vector_array[0:VECTOR_LEN]) copy(host_array[0:VECTOR_LEN][0:M])
    {
        int vector_idx = __pgi_vectoridx();
        
        /* vector_array is vector partitioned */
        vector_array[vector_idx] = vector_idx * 1.5f;
        
        #pragma acc loop vector
        for (int j = 0; j < M; j++) {
            host_array[vector_idx][j] += vector_array[vector_idx];
        }
    }
    
    /* ============================================
       CASE 5: Gang+Vector Partitioned
       Gang and vector parallelism without workers
       ============================================ */
    #pragma acc kernels copy(host_array[0:N][0:M])
    {
        int gv_partitioned;
        
        #pragma acc loop gang vector private(gv_partitioned)
        for (int i = 0; i < N; i++) {
            gv_partitioned = i * VECTOR_LEN + __pgi_vectoridx();
            
            #pragma acc loop seq
            for (int j = 0; j < M; j++) {
                host_array[i][j] += gv_partitioned * 0.1f;
            }
        }
    }
    
    /* ============================================
       CASE 6: Worker+Vector Partitioned
       Worker and vector parallelism
       ============================================ */
    #pragma acc parallel num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(host_array[0:N][0:M])
    {
        int wv_partitioned = __pgi_workeridx() * VECTOR_LEN + __pgi_vectoridx();
        
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN; v++) {
                int idx = w * VECTOR_LEN + v;
                if (idx < N) {
                    #pragma acc loop seq
                    for (int j = 0; j < M; j++) {
                        host_array[idx][j] += wv_partitioned * 0.01f;
                    }
                }
            }
        }
    }
    
    /* ============================================
       CASE 7: Fully Partitioned
       Gang+Worker+Vector partitioning
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        vector_length(VECTOR_LEN) copy(host_array[0:N][0:M]) \
        copy(reduction_result)
    {
        /* This variable should be fully partitioned */
        int fully_partitioned = __pgi_gangidx() * 10000 + 
                               __pgi_workeridx() * 100 + 
                               __pgi_vectoridx();
        
        int local_sum = 0;
        
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                #pragma acc loop vector reduction(+:local_sum)
                for (int v = 0; v < VECTOR_LEN; v++) {
                    int idx = (g * WORKERS * VECTOR_LEN + 
                              w * VECTOR_LEN + v) % N;
                    
                    /* Stencil computation to complicate analysis */
                    if (idx > 0 && idx < N-1) {
                        float neighbor_sum = host_array[idx-1][v % M] + 
                                           host_array[idx+1][v % M];
                        host_array[idx][v % M] = 
                            (host_array[idx][v % M] + neighbor_sum) * 0.5f;
                    }
                    
                    local_sum += fully_partitioned;
                }
            }
        }
        
        #pragma acc atomic update
        reduction_result += local_sum;
    }
    
    /* Conditional partitioning based on runtime values */
    int dynamic_condition = reduction_result % 2;
    
    #pragma acc kernels copy(host_array[0:N][0:M]) if(dynamic_condition)
    {
        int conditional_var;
        
        if (dynamic_condition) {
            /* Different partitioning based on condition */
            #pragma acc loop gang worker vector private(conditional_var)
            for (int i = 0; i < N; i++) {
                conditional_var = i;
                #pragma acc loop seq
                for (int j = 0; j < M; j++) {
                    host_array[i][j] += conditional_var;
                }
            }
        } else {
            #pragma acc loop gang vector private(conditional_var)
            for (int i = 0; i < N; i++) {
                conditional_var = i * 2;
                #pragma acc loop seq
                for (int j = 0; j < M; j++) {
                    host_array[i][j] += conditional_var;
                }
            }
        }
    }
    
    /* Final reduction to compute checksum */
    float final_checksum = 0.0f;
    #pragma acc parallel loop gang vector reduction(+:final_checksum) \
        copyin(host_array[0:N][0:M])
    for (int i = 0; i < N; i++) {
        #pragma acc loop seq
        for (int j = 0; j < M; j++) {
            final_checksum += host_array[i][j];
        }
    }
    
    printf("Final checksum: %f\n", final_checksum);
    printf("Reduction result: %d\n", reduction_result);
    
    return 0;
}
