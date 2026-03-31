/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define GANGS 4
#define WORKERS 8
#define VECTOR_LEN 32

/* Global variables at different scopes */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_partial_partitioned[16];    /* Will be partitioned differently */

/* Function with complex partitioning scenarios */
void compute_partitioned_states() {
    int i, j, k;
    
    /* 2D arrays for multi-dimensional access */
    float matrix_a[N][M];
    float matrix_b[N][M];
    float matrix_c[N][M];
    
    /* Initialize matrices */
    for (i = 0; i < N; i++) {
        for (j = 0; j < M; j++) {
            matrix_a[i][j] = (float)(i + j);
            matrix_b[i][j] = (float)(i * j);
        }
    }
    
    /* ============================================
       CASE 0: Gang Redundant
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copyin(global_gang_redundant) \
        copy(matrix_a, matrix_b) copyout(matrix_c)
    {
        /* global_gang_redundant is read-only and same for all gangs */
        int gang_id = __pgi_gangidx();
        #pragma acc loop gang independent
        for (i = 0; i < N; i++) {
            #pragma acc loop worker independent
            for (j = 0; j < M; j++) {
                matrix_c[i][j] = matrix_a[i][j] + matrix_b[i][j] + global_gang_redundant;
            }
        }
    }
    
    /* ============================================
       CASE 1: Gang Partitioned
       ============================================ */
    int gang_private_var = 0;  /* Will be gang partitioned */
    #pragma acc parallel num_gangs(GANGS) private(gang_private_var) \
        copy(matrix_a, matrix_b) copyout(matrix_c)
    {
        gang_private_var = __pgi_gangidx();  /* Different for each gang */
        
        #pragma acc loop gang independent
        for (i = 0; i < N; i++) {
            #pragma acc loop worker independent
            for (j = 0; j < M; j++) {
                matrix_c[i][j] = matrix_a[i][j] * gang_private_var + matrix_b[i][j];
            }
        }
    }
    
    /* ============================================
       CASE 2: Worker Partitioned
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(matrix_a, matrix_b) copyout(matrix_c)
    {
        int worker_local;  /* Worker partitioned */
        
        #pragma acc loop gang independent
        for (i = 0; i < N; i++) {
            #pragma acc loop worker independent private(worker_local)
            for (j = 0; j < M; j++) {
                worker_local = __pgi_workeridx();
                #pragma acc loop vector independent
                for (k = 0; k < 4; k++) {
                    matrix_c[i][j] += matrix_a[i][j] * worker_local + k;
                }
            }
        }
    }
    
    /* ============================================
       CASE 3: Gang+Worker Partitioned
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copy(matrix_a, matrix_b) copyout(matrix_c)
    {
        int gw_partitioned;  /* Gang+Worker partitioned */
        
        #pragma acc loop gang independent private(gw_partitioned)
        for (i = 0; i < N; i++) {
            gw_partitioned = __pgi_gangidx() * 100;
            #pragma acc loop worker independent
            for (j = 0; j < M; j++) {
                gw_partitioned += __pgi_workeridx();
                #pragma acc loop vector independent
                for (k = 0; k < 8; k++) {
                    matrix_c[i][j] += matrix_a[i][j] * gw_partitioned;
                }
            }
        }
    }
    
    /* ============================================
       CASE 4: Vector Partitioned
       ============================================ */
    #pragma acc kernels copy(matrix_a, matrix_b) copyout(matrix_c)
    {
        #pragma acc loop independent
        for (i = 0; i < N; i++) {
            float vector_private;  /* Vector partitioned */
            #pragma acc loop vector private(vector_private)
            for (j = 0; j < M; j++) {
                vector_private = (float)__pgi_vectoridx();
                matrix_c[i][j] = matrix_a[i][j] + vector_private;
            }
        }
    }
    
    /* ============================================
       CASE 5: Gang+Vector Partitioned
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
        copy(matrix_a, matrix_b) copyout(matrix_c)
    {
        int gv_partitioned;  /* Gang+Vector partitioned */
        
        #pragma acc loop gang independent private(gv_partitioned)
        for (i = 0; i < N; i++) {
            gv_partitioned = __pgi_gangidx();
            #pragma acc loop vector independent
            for (j = 0; j < M; j++) {
                gv_partitioned += __pgi_vectoridx();
                matrix_c[i][j] = matrix_a[i][j] * gv_partitioned;
            }
        }
    }
    
    /* ============================================
       CASE 6: Worker+Vector Partitioned
       ============================================ */
    #pragma acc parallel num_gangs(2) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(matrix_a, matrix_b) copyout(matrix_c)
    {
        int wv_partitioned;  /* Worker+Vector partitioned */
        
        #pragma acc loop gang independent
        for (i = 0; i < N; i++) {
            #pragma acc loop worker independent private(wv_partitioned)
            for (j = 0; j < M; j++) {
                wv_partitioned = __pgi_workeridx();
                #pragma acc loop vector independent
                for (k = 0; k < 16; k++) {
                    wv_partitioned += __pgi_vectoridx();
                    matrix_c[i][j] += matrix_b[i][j] * wv_partitioned;
                }
            }
        }
    }
    
    /* ============================================
       CASE 7: Fully Partitioned (Gang+Worker+Vector)
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(matrix_a, matrix_b) copyout(matrix_c)
    {
        int fully_partitioned;  /* Fully partitioned across all levels */
        
        #pragma acc loop gang independent private(fully_partitioned)
        for (i = 0; i < N; i++) {
            fully_partitioned = __pgi_gangidx() * 1000;
            #pragma acc loop worker independent
            for (j = 0; j < M; j++) {
                fully_partitioned += __pgi_workeridx() * 100;
                #pragma acc loop vector independent
                for (k = 0; k < 8; k++) {
                    fully_partitioned += __pgi_vectoridx();
                    /* Stencil-like access pattern */
                    int idx_i = (i + k) % N;
                    int idx_j = (j + k) % M;
                    matrix_c[idx_i][idx_j] = matrix_a[i][j] * fully_partitioned + 
                                            matrix_b[(i+1)%N][j] + 
                                            matrix_b[i][(j+1)%M];
                }
            }
        }
    }
    
    /* Conditional partitioning based on runtime values */
    int dynamic_condition = matrix_c[0][0] > 1000 ? 1 : 0;
    
    #pragma acc parallel num_gangs(GANGS) copy(matrix_c) if(dynamic_condition)
    {
        int conditional_var;  /* Partitioning depends on runtime condition */
        
        #pragma acc loop gang independent private(conditional_var)
        for (i = 0; i < N; i++) {
            conditional_var = dynamic_condition ? __pgi_gangidx() : 0;
            #pragma acc loop worker independent
            for (j = 0; j < M; j++) {
                matrix_c[i][j] += conditional_var;
            }
        }
    }
    
    /* Final reduction to prevent dead code elimination */
    float checksum = 0.0f;
    #pragma acc parallel loop reduction(+:checksum) copyin(matrix_c)
    for (i = 0; i < N; i++) {
        #pragma acc loop reduction(+:checksum)
        for (j = 0; j < M; j++) {
            checksum += matrix_c[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
}

/* Additional function with nested parallelism */
void nested_partitioning() {
    int outer_array[GANGS][WORKERS];
    int inner_array[VECTOR_LEN];
    
    #pragma acc parallel num_gangs(GANGS) copyout(outer_array)
    {
        int gang_local = __pgi_gangidx();
        
        #pragma acc loop gang independent
        for (int g = 0; g < GANGS; g++) {
            /* Nested worker parallelism */
            #pragma acc loop worker independent
            for (int w = 0; w < WORKERS; w++) {
                outer_array[g][w] = gang_local * 100 + w;
                
                /* Vector parallelism inside worker */
                #pragma acc loop vector independent
                for (int v = 0; v < VECTOR_LEN; v++) {
                    inner_array[v] = g * 1000 + w * 100 + v;
                }
            }
        }
    }
}

int main() {
    printf("Testing OpenACC variable partitioning states...\n");
    
    /* Initialize global array */
    for (int i = 0; i < 16; i++) {
        global_partial_partitioned[i] = (float)i * 1.5f;
    }
    
    compute_partitioned_states();
    nested_partitioning();
    
    printf("Test completed.\n");
    return 0;
}
