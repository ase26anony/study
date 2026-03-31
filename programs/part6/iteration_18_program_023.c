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
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Base for various partitionings */
static int file_static_counter = 0;      /* Another scope level */

/* Function to initialize data */
void init_data() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) * 0.1f;
        }
    }
}

/* Main function with complex nested parallelism */
int main() {
    /* Local variables with different lifetimes */
    int host_scalar = 100;
    float host_array[N][M];
    int gang_private_var;                /* Should become gang partitioned (case 1) */
    float worker_partitioned[M];         /* Should become worker partitioned (case 2) */
    double gang_worker_shared[GANGS][WORKERS]; /* Gang+worker partitioned (case 3) */
    
    /* Initialize data */
    init_data();
    
    /* Copy initial data to host array */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            host_array[i][j] = global_matrix[i][j];
        }
    }
    
    printf("Starting OpenACC partitioning tests...\n");
    
    /* ============================================
       CASE 0: Gang Redundant
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) copy(host_scalar) \
        copyin(global_gang_redundant)
    {
        /* global_gang_redundant should be marked as gang redundant */
        int gang_id = __pgi_gangidx();
        if (gang_id == 0) {
            global_gang_redundant += 1;  /* All gangs see same value */
        }
        host_scalar = global_gang_redundant * gang_id;
    }
    
    /* ============================================
       CASE 1: Gang Partitioned  
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) private(gang_private_var) \
        copy(host_array[0:N][0:M])
    {
        gang_private_var = __pgi_gangidx() * 100;
        
        #pragma acc loop gang independent
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector independent
            for (int j = 0; j < M; j++) {
                host_array[i][j] += gang_private_var;
            }
        }
    }
    
    /* ============================================
       CASE 2: Worker Partitioned
       CASE 4: Vector Partitioned
       ============================================ */
    #pragma acc kernels copy(worker_partitioned[0:M]) \
        copy(host_array[0:N][0:M])
    {
        /* worker_partitioned array - each worker gets its own copy */
        #pragma acc loop worker independent
        for (int w = 0; w < M; w++) {
            worker_partitioned[w] = __pgi_workeridx() * 10.0f + w;
        }
        
        /* Vector-only partitioning */
        float vector_private[VECTOR_LEN];  /* Should become vector partitioned (case 4) */
        
        #pragma acc loop vector independent
        for (int v = 0; v < VECTOR_LEN; v++) {
            vector_private[v] = __pgi_vectoridx() * 1.5f;
            
            /* Use in computation to prevent optimization */
            if (v < N && v < M) {
                host_array[v][v] += vector_private[v];
            }
        }
    }
    
    /* ============================================
       CASE 3: Gang+Worker Partitioned
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copy(gang_worker_shared[0:GANGS][0:WORKERS])
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        
        /* This 2D array is partitioned across both gangs and workers */
        gang_worker_shared[gang_id][worker_id] = 
            (double)gang_id * 1000.0 + (double)worker_id * 100.0;
        
        /* Nested computation to ensure partitioning analysis */
        #pragma acc loop worker independent
        for (int i = 0; i < WORKERS; i++) {
            double temp = gang_worker_shared[gang_id][i];
            gang_worker_shared[gang_id][i] = sqrt(temp + 1.0);
        }
    }
    
    /* ============================================
       CASE 5: Gang+Vector Partitioned
       CASE 6: Worker+Vector Partitioned  
       CASE 7: Fully Partitioned
       ============================================ */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        vector_length(VECTOR_LEN) copy(host_array[0:N][0:M])
    {
        /* Mixed partitioning variables */
        int gang_vector_local[VECTOR_LEN];      /* Gang+vector partitioned (case 5) */
        float worker_vector_local[WORKERS][VECTOR_LEN]; /* Worker+vector (case 6) */
        double fully_partitioned[GANGS][WORKERS][VECTOR_LEN]; /* Fully (case 7) */
        
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        int vector_id = __pgi_vectoridx();
        
        /* Initialize gang+vector partitioned array */
        #pragma acc loop vector independent
        for (int v = 0; v < VECTOR_LEN; v++) {
            gang_vector_local[v] = gang_id * VECTOR_LEN + v;
        }
        
        /* Initialize worker+vector partitioned array */
        #pragma acc loop worker independent
        for (int w = 0; w < WORKERS; w++) {
            #pragma acc loop vector independent
            for (int v = 0; v < VECTOR_LEN; v++) {
                worker_vector_local[w][v] = w * 50.0f + v * 0.5f;
            }
        }
        
        /* Initialize fully partitioned array */
        fully_partitioned[gang_id][worker_id][vector_id] = 
            gang_id * 10000.0 + worker_id * 1000.0 + vector_id * 10.0;
        
        /* Complex stencil computation to force dependency analysis */
        #pragma acc loop gang independent
        for (int i = 1; i < N-1; i++) {
            #pragma acc loop worker independent
            for (int j = 1; j < M-1; j++) {
                float sum = 0.0f;
                
                #pragma acc loop vector independent reduction(+:sum)
                for (int v = 0; v < VECTOR_LEN && v < 3; v++) {
                    /* Access neighboring elements (stencil pattern) */
                    sum += host_array[i-1][j] * 0.25f +
                           host_array[i][j-1] * 0.25f +
                           host_array[i][j] * 0.5f +
                           host_array[i][j+1] * 0.25f +
                           host_array[i+1][j] * 0.25f;
                    
                    /* Use partitioned variables in computation */
                    sum += gang_vector_local[v % VECTOR_LEN] * 0.01f;
                    sum += worker_vector_local[worker_id % WORKERS][v % VECTOR_LEN];
                    sum += fully_partitioned[gang_id % GANGS][worker_id % WORKERS][vector_id % VECTOR_LEN] * 0.001f;
                }
                
                /* Conditional update based on runtime values */
                if (sum > 100.0f) {
                    host_array[i][j] = sum / (VECTOR_LEN * 5.0f);
                } else {
                    host_array[i][j] = sum / (VECTOR_LEN * 3.0f);
                }
            }
        }
    }
    
    /* ============================================
       Additional test: Kernels with explicit clauses
       ============================================ */
    float reduction_result = 0.0f;
    
    #pragma acc kernels copyin(global_matrix) copy(reduction_result)
    {
        #pragma acc loop gang reduction(+:reduction_result)
        for (int i = 0; i < N; i++) {
            float row_sum = 0.0f;
            
            #pragma acc loop worker reduction(+:row_sum)
            for (int j = 0; j < M; j++) {
                float element = global_matrix[i][j];
                
                #pragma acc loop vector reduction(+:element)
                for (int k = 0; k < 4; k++) {
                    element += k * 0.1f;
                }
                
                row_sum += element;
            }
            
            reduction_result += row_sum;
        }
    }
    
    /* Final computation and output to prevent dead code elimination */
    float checksum = 0.0f;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += host_array[i][j];
        }
    }
    
    checksum += reduction_result;
    
    printf("Final checksum: %f\n", checksum);
    printf("Global gang redundant value: %d\n", global_gang_redundant);
    
    /* Print gang-worker shared values */
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            printf("gang_worker_shared[%d][%d] = %f\n", 
                   g, w, gang_worker_shared[g][w]);
        }
    }
    
    return 0;
}
