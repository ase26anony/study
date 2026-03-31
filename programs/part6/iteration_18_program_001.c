/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables to create complex scoping scenarios */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Base for multi-dimensional access */
static int module_scalar = 100;

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) * 0.001f;
        }
    }
}

/* Main function with complex nested parallelism */
int main() {
    /* Local variables with different scopes */
    int host_scalar = 10;
    float host_array[N];
    double host_matrix[N][M];
    int result = 0;
    
    /* Initialize data */
    init_matrices();
    
    /* Initialize host arrays */
    for (int i = 0; i < N; i++) {
        host_array[i] = (float)i * 0.5f;
        for (int j = 0; j < M; j++) {
            host_matrix[i][j] = (double)(i * j) * 0.01;
        }
    }
    
    printf("Starting OpenACC/OpenMP partitioning tests...\n");
    
    /* ============================================
       CASE 0: Gang Redundant
       A variable that is read-only and same for all gangs
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copyout(result) \
        copyin(global_gang_redundant)
    {
        int gang_id = __pgi_gangidx();
        /* global_gang_redundant should be marked as gang redundant */
        result = global_gang_redundant * (gang_id + 1);
    }
    printf("Case 0 (gang redundant) result: %d\n", result);
    
    /* ============================================
       CASE 1: Gang Partitioned
       Variables private to each gang
       ============================================ */
    int gang_partitioned_var = 0;
    #pragma acc parallel num_gangs(GANGS) copy(gang_partitioned_var)
    {
        int gang_private = __pgi_gangidx() * 10;  /* Should be gang partitioned */
        gang_partitioned_var += gang_private;
        
        /* Simple computation to keep variable alive */
        for (int i = 0; i < 10; i++) {
            gang_private += i;
        }
    }
    printf("Case 1 (gang partitioned) result: %d\n", gang_partitioned_var);
    
    /* ============================================
       CASE 2: Worker Partitioned  
       Using worker-only parallelism
       ============================================ */
    float worker_results[WORKERS] = {0};
    #pragma acc parallel num_workers(WORKERS) vector_length(1) \
        copyout(worker_results[0:WORKERS])
    {
        int worker_id = __pgi_workeridx();
        float worker_private = worker_id * 1.5f;  /* Should be worker partitioned */
        
        /* Worker-specific computation */
        for (int i = 0; i < 100; i++) {
            worker_private += sinf((float)i * 0.1f);
        }
        worker_results[worker_id] = worker_private;
    }
    printf("Case 2 (worker partitioned) first result: %f\n", worker_results[0]);
    
    /* ============================================
       CASE 4: Vector Partitioned
       Vector-only parallelism
       ============================================ */
    float vector_array[VECTOR_LEN];
    #pragma acc parallel vector_length(VECTOR_LEN) \
        copyout(vector_array[0:VECTOR_LEN])
    {
        int vector_id = __pgi_vectoridx();
        float vector_private = vector_id * 2.0f;  /* Should be vector partitioned */
        
        /* Vector-specific computation */
        vector_private = vector_private * vector_private;
        vector_array[vector_id] = vector_private;
    }
    printf("Case 4 (vector partitioned) sample: %f\n", vector_array[VECTOR_LEN/2]);
    
    /* ============================================
       CASE 3: Gang+Worker Partitioned
       Nested gang-worker parallelism
       ============================================ */
    int gw_matrix[GANGS][WORKERS] = {{0}};
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(1) \
        copyout(gw_matrix)
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        
        /* This variable should be gang+worker partitioned */
        int gw_private = gang_id * 100 + worker_id * 10;
        
        /* Complex access pattern */
        for (int i = 0; i < 50; i++) {
            gw_private += (gang_id * worker_id + i) % 7;
        }
        gw_matrix[gang_id][worker_id] = gw_private;
    }
    printf("Case 3 (gang+worker partitioned) [0][0]: %d\n", gw_matrix[0][0]);
    
    /* ============================================
       CASE 5: Gang+Vector Partitioned
       Using kernels with gang and vector loops
       ============================================ */
    float gv_results[N] = {0};
    #pragma acc kernels copyin(global_matrix) copyout(gv_results)
    {
        /* Gang partitioned dimension */
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            float gang_accum = 0.0f;
            
            /* Vector partitioned dimension */
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                /* This access creates gang+vector partitioning needs */
                float element = global_matrix[i][j];
                gang_accum += element * element;
            }
            
            /* Variable with mixed partitioning */
            float gv_mixed = gang_accum * (i + 1);  /* Should be gang+vector partitioned */
            gv_results[i] = gv_mixed;
        }
    }
    printf("Case 5 (gang+vector partitioned) sum: %f\n", gv_results[N/2]);
    
    /* ============================================
       CASE 6: Worker+Vector Partitioned
       Worker-vector nested parallelism
       ============================================ */
    float wv_array[WORKERS][VECTOR_LEN];
    #pragma acc parallel num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copyout(wv_array)
    {
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        /* Worker+vector partitioned variable */
        float wv_private = worker_id * 20.0f + vector_id * 0.5f;
        
        /* Stencil-like computation to complicate analysis */
        for (int iter = 0; iter < 10; iter++) {
            wv_private = sinf(wv_private) + cosf(wv_private * 0.1f);
        }
        wv_array[worker_id][vector_id] = wv_private;
    }
    printf("Case 6 (worker+vector partitioned) [0][0]: %f\n", wv_array[0][0]);
    
    /* ============================================
       CASE 7: Fully Partitioned (Gang+Worker+Vector)
       Triple-nested parallelism with complex data access
       ============================================ */
    int fully_partitioned[GANGS][WORKERS][VECTOR_LEN] = {{{0}}};
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        vector_length(VECTOR_LEN) copyout(fully_partitioned)
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        /* Base variable - should be fully partitioned */
        int fully_private = gang_id * 1000 + worker_id * 100 + vector_id * 1;
        
        /* Conditional partitioning based on runtime values */
        int conditional_var;
        if (gang_id % 2 == 0) {
            conditional_var = worker_id * 50;  /* Different partitioning path */
        } else {
            conditional_var = vector_id * 30;  /* Alternative partitioning path */
        }
        
        /* Multi-dimensional array access with dependencies */
        for (int k = 0; k < 5; k++) {
            fully_private += conditional_var * k;
            
            /* Access neighboring elements (stencil) */
            if (vector_id > 0 && vector_id < VECTOR_LEN - 1) {
                fully_private += (conditional_var % 7);
            }
        }
        
        fully_partitioned[gang_id][worker_id][vector_id] = fully_private;
    }
    printf("Case 7 (fully partitioned) [0][0][0]: %d\n", fully_partitioned[0][0][0]);
    
    /* ============================================
       Additional complex scenario: Mixed directives
       Using both OpenACC kernels and parallel with data clauses
       ============================================ */
    float final_result = 0.0f;
    
    /* OpenACC kernels with explicit data management */
    #pragma acc data copyin(host_matrix) copy(final_result)
    {
        #pragma acc kernels
        {
            float temp_accum = 0.0f;
            
            /* Mixed loop directives */
            #pragma acc loop gang reduction(+:temp_accum)
            for (int i = 1; i < N-1; i++) {
                #pragma acc loop worker
                for (int j = 1; j < M-1; j++) {
                    float worker_temp = 0.0f;
                    
                    #pragma acc loop vector reduction(+:worker_temp)
                    for (int k = 0; k < 3; k++) {
                        /* Complex 3D stencil computation */
                        worker_temp += host_matrix[i-1][j] * 0.25f +
                                      host_matrix[i][j-1] * 0.25f +
                                      host_matrix[i][j] * 0.5f;
                    }
                    
                    temp_accum += worker_temp;
                }
            }
            
            final_result = temp_accum;
        }
    }
    
    printf("Final stencil result: %f\n", final_result);
    printf("All partitioning tests completed.\n");
    
    return 0;
}
