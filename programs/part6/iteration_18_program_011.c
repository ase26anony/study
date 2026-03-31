/* test_partitioning_states.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables to create complex scoping scenarios */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Multi-dimensional array */
static int module_scalar = 100;

/* Function to initialize data */
void init_data() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j);
        }
    }
}

/* Main function with complex nested parallelism */
int main() {
    int host_scalar = 10;
    float host_array[N];
    int gang_partitioned = 0;            /* Should become gang partitioned (case 1) */
    float worker_partitioned[M];         /* Should become worker partitioned (case 2) */
    int vector_partitioned = 0;          /* Should become vector partitioned (case 4) */
    
    /* Initialize data */
    init_data();
    
    /* Initialize local arrays */
    for (int i = 0; i < N; i++) host_array[i] = 0.0f;
    for (int j = 0; j < M; j++) worker_partitioned[j] = (float)j;
    
    printf("Starting OpenACC partitioning tests...\n");
    
    /* ============================================
       CASE 0: Gang Redundant
       Global variable accessed by all gangs
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copyout(host_array[0:N]) \
        present(global_matrix)
    {
        int gang_id = __pgi_gangidx();
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            /* global_gang_redundant is accessible by all gangs - should be gang redundant */
            host_array[i] = global_matrix[i][0] + global_gang_redundant;
        }
    }
    
    /* ============================================
       CASE 1: Gang Partitioned
       Variable private to each gang
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) private(gang_partitioned) \
        copy(gang_partitioned)
    {
        gang_partitioned = __pgi_gangidx() * 100;
        /* Each gang has its own copy */
    }
    
    /* ============================================
       CASE 2: Worker Partitioned  
       Variable partitioned across workers
       ============================================ */
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) \
        copy(worker_partitioned[0:M])
    {
        int worker_id = __pgi_workeridx();
        #pragma acc loop worker
        for (int j = 0; j < M; j++) {
            /* Each worker processes a portion */
            worker_partitioned[j] += (float)worker_id;
        }
    }
    
    /* ============================================
       CASE 3: Gang+Worker Partitioned
       Nested gang-worker parallelism
       ============================================ */
    {
        int gw_partitioned = 0;  /* Should become gang+worker partitioned */
        #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
            private(gw_partitioned)
        {
            int gang_id = __pgi_gangidx();
            int worker_id = __pgi_workeridx();
            gw_partitioned = gang_id * 1000 + worker_id * 100;
            
            #pragma acc loop gang worker
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < M; j++) {
                    /* Complex access pattern */
                    if ((i + j) % 32 == 0) {
                        global_matrix[i][j] += (float)gw_partitioned;
                    }
                }
            }
        }
    }
    
    /* ============================================
       CASE 4: Vector Partitioned
       Vector-level parallelism only
       ============================================ */
    #pragma acc parallel vector_length(VECTOR_LEN) \
        private(vector_partitioned)
    {
        vector_partitioned = __pgi_vectoridx();
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            host_array[i] += (float)vector_partitioned;
        }
    }
    
    /* ============================================
       CASE 5: Gang+Vector Partitioned
       Combined gang and vector parallelism
       ============================================ */
    {
        float gv_partitioned[N];  /* Should become gang+vector partitioned */
        #pragma acc data create(gv_partitioned[0:N])
        {
            #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN)
            {
                int gang_id = __pgi_gangidx();
                int vector_id = __pgi_vectoridx();
                
                #pragma acc loop gang vector
                for (int i = 0; i < N; i++) {
                    gv_partitioned[i] = global_matrix[i][0] * 
                                       (float)(gang_id * VECTOR_LEN + vector_id);
                }
            }
            
            /* Use the result to prevent dead code elimination */
            #pragma acc parallel loop
            for (int i = 0; i < N; i++) {
                host_array[i] += gv_partitioned[i];
            }
        }
    }
    
    /* ============================================
       CASE 6: Worker+Vector Partitioned
       Combined worker and vector parallelism
       ============================================ */
    {
        int wv_partitioned[M];  /* Should become worker+vector partitioned */
        #pragma acc data create(wv_partitioned[0:M])
        {
            #pragma acc parallel num_workers(WORKERS) vector_length(VECTOR_LEN)
            {
                int worker_id = __pgi_workeridx();
                int vector_id = __pgi_vectoridx();
                
                #pragma acc loop worker vector
                for (int j = 0; j < M; j++) {
                    wv_partitioned[j] = worker_id * 100 + vector_id;
                }
            }
        }
    }
    
    /* ============================================
       CASE 7: Fully Partitioned
       Gang+Worker+Vector partitioning
       ============================================ */
    {
        /* Complex nested parallelism with all levels */
        #pragma acc kernels
        {
            float fully_partitioned[N][M];  /* Should become fully partitioned */
            
            #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
                vector_length(VECTOR_LEN) copyout(fully_partitioned[0:N][0:M])
            {
                int gang_id = __pgi_gangidx();
                int worker_id = __pgi_workeridx();
                int vector_id = __pgi_vectoridx();
                
                /* Triple-nested loops with explicit partitioning */
                #pragma acc loop gang independent
                for (int i = 0; i < N; i++) {
                    #pragma acc loop worker independent
                    for (int j = 0; j < M; j++) {
                        /* Stencil computation to complicate dependency analysis */
                        float sum = 0.0f;
                        #pragma acc loop vector reduction(+:sum)
                        for (int k = 0; k < 3; k++) {
                            int idx_i = (i + k - 1 + N) % N;
                            int idx_j = (j + k - 1 + M) % M;
                            sum += global_matrix[idx_i][idx_j];
                        }
                        fully_partitioned[i][j] = sum * 
                            (float)(gang_id + worker_id + vector_id);
                    }
                }
            }
            
            /* Conditional partitioning based on runtime values */
            int dynamic_partition = host_scalar > 5 ? 1 : 0;
            if (dynamic_partition) {
                #pragma acc parallel loop gang worker vector collapse(2)
                for (int i = 0; i < N; i++) {
                    for (int j = 0; j < M; j++) {
                        /* Additional computation with mixed attributes */
                        global_matrix[i][j] = fully_partitioned[i][j] * 0.5f;
                    }
                }
            }
        }
    }
    
    /* Final computation and verification */
    float checksum = 0.0f;
    #pragma acc parallel loop reduction(+:checksum) \
        copyin(global_matrix) copy(checksum)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += global_matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Partitioning test completed.\n");
    
    return 0;
}
