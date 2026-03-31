/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 4
#define WORKERS 8
#define VECTOR_LEN 32

/* Global variables at different scopes */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Multi-dimensional array */
static int module_level_var = 100;

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) / (N * M);
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    /* Local variables with different types */
    int host_scalar = 10;
    float host_array[N];
    double host_matrix[N][M];
    int conditional_flag = 1;
    
    /* Initialize data */
    init_matrices();
    
    for (int i = 0; i < N; i++) {
        host_array[i] = sinf(i * 0.01f);
        for (int j = 0; j < M; j++) {
            host_matrix[i][j] = cos(i * 0.02 + j * 0.01);
        }
    }
    
    printf("Starting OpenACC/OpenMP partitioning tests...\n");
    
    /* ============================================
       CASE 0: Gang Redundant
       Global variable accessed by all gangs
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copyout(host_array[0:N])
    {
        int gang_id = __pgi_gangidx();
        /* global_gang_redundant is accessible by all gangs without partitioning */
        host_array[gang_id] = global_gang_redundant + gang_id;
    }
    
    /* ============================================
       CASE 1: Gang Partitioned
       Variable partitioned across gangs only
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) private(host_scalar) \
        copy(host_scalar) copyin(host_array[0:N])
    {
        int gang_id = __pgi_gangidx();
        /* host_scalar is private to each gang - gang partitioned */
        host_scalar = gang_id * 100;
        
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            if (i % GANGS == gang_id) {
                host_array[i] += host_scalar;
            }
        }
    }
    
    /* ============================================
       CASE 2: Worker Partitioned
       Variable partitioned across workers only
       ============================================ */
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) \
        copy(host_matrix[0:N][0:M])
    {
        int worker_id = __pgi_workeridx();
        float worker_private;  /* Worker partitioned variable */
        
        #pragma acc loop worker
        for (int i = 0; i < N; i++) {
            worker_private = worker_id * 10.0f + i;
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                host_matrix[i][j] += worker_private;
            }
        }
    }
    
    /* ============================================
       CASE 3: Gang+Worker Partitioned
       Variable partitioned across both gangs and workers
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copy(global_matrix[0:N][0:M])
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        
        /* This variable is partitioned across both gangs and workers */
        float gw_partitioned = gang_id * 1000.0f + worker_id * 100.0f;
        
        #pragma acc loop gang worker
        for (int i = gang_id; i < N; i += GANGS) {
            #pragma acc loop vector
            for (int j = worker_id; j < M; j += WORKERS) {
                global_matrix[i][j] = gw_partitioned + i * M + j;
            }
        }
    }
    
    /* ============================================
       CASE 4: Vector Partitioned
       Variable partitioned across vector lanes
       ============================================ */
    #pragma acc kernels copyout(host_array[0:N])
    {
        #pragma acc loop independent vector(VECTOR_LEN)
        for (int i = 0; i < N; i++) {
            float vector_private = i * 2.0f;  /* Vector partitioned */
            host_array[i] = vector_private * vector_private;
        }
    }
    
    /* ============================================
       CASE 5: Gang+Vector Partitioned
       Variable partitioned across gangs and vectors
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
        copy(host_matrix[0:N][0:M])
    {
        int gang_id = __pgi_gangidx();
        int vector_id = __pgi_vectoridx();
        
        /* Partitioned across gangs and vectors */
        double gv_partitioned = gang_id * 500.0 + vector_id * 50.0;
        
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                if ((i + j) % (GANGS * VECTOR_LEN) == (gang_id * VECTOR_LEN + vector_id)) {
                    host_matrix[i][j] = gv_partitioned;
                }
            }
        }
    }
    
    /* ============================================
       CASE 6: Worker+Vector Partitioned
       Variable partitioned across workers and vectors
       ============================================ */
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(global_matrix[0:N][0:M])
    {
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        /* Worker+vector partitioned variable */
        float wv_partitioned = worker_id * 200.0f + vector_id * 20.0f;
        
        #pragma acc loop worker vector
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                /* Stencil computation to complicate analysis */
                if (i > 0 && j > 0 && i < N-1 && j < M-1) {
                    global_matrix[i][j] = wv_partitioned + 
                        (global_matrix[i-1][j] + global_matrix[i+1][j] +
                         global_matrix[i][j-1] + global_matrix[i][j+1]) * 0.25f;
                }
            }
        }
    }
    
    /* ============================================
       CASE 7: Fully Partitioned
       Variable partitioned across gangs, workers, and vectors
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(host_matrix[0:N][0:M])
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        /* Fully partitioned variable */
        double fully_partitioned = gang_id * 10000.0 + 
                                  worker_id * 1000.0 + 
                                  vector_id * 100.0;
        
        /* Conditional partitioning based on runtime value */
        if (conditional_flag) {
            /* Complex nested loops with mixed partitioning */
            #pragma acc loop gang
            for (int g = 0; g < GANGS; g++) {
                float gang_local = g * 100.0f;
                #pragma acc loop worker
                for (int w = 0; w < WORKERS; w++) {
                    float worker_local = w * 10.0f;
                    #pragma acc loop vector
                    for (int v = 0; v < VECTOR_LEN; v++) {
                        int idx = (g * WORKERS * VECTOR_LEN) + 
                                 (w * VECTOR_LEN) + v;
                        if (idx < N * M) {
                            int i = idx / M;
                            int j = idx % M;
                            host_matrix[i][j] = fully_partitioned + 
                                               gang_local + 
                                               worker_local + 
                                               v;
                        }
                    }
                }
            }
        }
    }
    
    /* ============================================
       Mixed OpenMP/OpenACC constructs for additional coverage
       ============================================ */
    #pragma acc parallel loop gang worker vector \
        copy(host_array[0:N]) copyin(module_level_var)
    for (int i = 0; i < N; i++) {
        /* Mixed attributes in same region */
        int private_scalar = i;
        static int persistent_var = 0;  /* Static variables add complexity */
        
        #pragma acc atomic update
        persistent_var++;
        
        host_array[i] = private_scalar + module_level_var + persistent_var;
    }
    
    /* Nested parallelism with data clauses */
    #pragma acc data copy(global_matrix[0:N][0:M]) create(host_array[0:N])
    {
        #pragma acc parallel num_gangs(2)
        {
            int outer_gang_var = __pgi_gangidx() * 1000;
            
            #pragma acc loop gang
            for (int i = 0; i < N/2; i++) {
                float loop_private = i * 3.14f;
                
                #pragma acc parallel num_workers(4) vector_length(16) \
                    private(outer_gang_var)
                {
                    int inner_worker_var = __pgi_workeridx() * 100;
                    int inner_vector_var = __pgi_vectoridx() * 10;
                    
                    /* Complex expression with mixed partitioning */
                    float computed = outer_gang_var + inner_worker_var + 
                                    inner_vector_var + loop_private;
                    
                    #pragma acc loop worker vector
                    for (int j = 0; j < M; j++) {
                        int idx = i * M + j;
                        if (idx < N * M) {
                            global_matrix[i][j] += computed;
                        }
                    }
                }
            }
        }
    }
    
    /* Final computation and verification */
    double checksum = 0.0;
    #pragma acc parallel loop reduction(+:checksum) \
        copyin(global_matrix[0:N][0:M])
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += global_matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Partitioning test completed.\n");
    
    return 0;
}
