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
float global_matrix[N][M];               /* Base for various partitionings */
static int file_static_counter = 0;      /* Static storage duration variable */

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
    /* Local variables with different lifetimes */
    int host_scalar = 100;
    float host_array[N][M];
    int gang_private_scalar;             /* Target: gang partitioned (case 1) */
    float worker_partitioned[M];         /* Target: worker partitioned (case 2) */
    double gang_worker_shared;           /* Target: gang+worker partitioned (case 3) */
    int vector_only_var;                 /* Target: vector partitioned (case 4) */
    float gang_vector_array[GANGS][VECTOR_LEN]; /* Target: gang+vector partitioned (case 5) */
    int worker_vector_reduction;         /* Target: worker+vector partitioned (case 6) */
    double fully_partitioned[GANGS][WORKERS][VECTOR_LEN]; /* Target: fully partitioned (case 7) */
    
    /* Initialize data */
    init_matrices();
    
    /* Copy to local array for modification */
    #pragma acc parallel loop gang collapse(2) copy(global_matrix) copyout(host_array)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            host_array[i][j] = global_matrix[i][j];
        }
    }
    
    /* =========================================== */
    /* CASE 0: Gang Redundant Variable */
    /* Global variable accessed by all gangs */
    /* =========================================== */
    #pragma acc parallel num_gangs(GANGS) copy(global_gang_redundant)
    {
        int local_gang_id = __pgi_gangidx();
        /* All gangs read the same global variable */
        int temp = global_gang_redundant + local_gang_id;
        /* Prevent dead code elimination */
        if (temp < 0) global_gang_redundant = 0;
    }
    
    /* =========================================== */
    /* CASE 1: Gang Partitioned Variable */
    /* Variable private to each gang */
    /* =========================================== */
    #pragma acc parallel num_gangs(GANGS) private(gang_private_scalar) \
        copyin(host_scalar) copy(gang_private_scalar)
    {
        gang_private_scalar = __pgi_gangidx() * 10 + host_scalar;
        
        /* Nested worker loop to create partitioning context */
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            /* Access gang-private variable */
            int temp = gang_private_scalar + w;
            if (temp < 0) gang_private_scalar = 0;
        }
    }
    
    /* =========================================== */
    /* CASE 2: Worker Partitioned Variable */
    /* Variable partitioned across workers only */
    /* =========================================== */
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) \
        copyin(worker_partitioned) copyout(worker_partitioned)
    {
        int worker_id = __pgi_workeridx();
        
        /* Each worker works on its own slice */
        #pragma acc loop vector
        for (int i = 0; i < M; i++) {
            if (worker_id == 0) {
                worker_partitioned[i] = sinf((float)i * 0.01f);
            } else {
                worker_partitioned[i] = cosf((float)i * 0.01f);
            }
        }
    }
    
    /* =========================================== */
    /* CASE 3: Gang+Worker Partitioned Variable */
    /* Variable partitioned across both gangs and workers */
    /* =========================================== */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        private(gang_worker_shared)
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        
        gang_worker_shared = (double)gang_id * 100.0 + (double)worker_id;
        
        /* Vector loop inside worker */
        #pragma acc loop vector
        for (int v = 0; v < VECTOR_LEN; v++) {
            /* Use the variable to prevent optimization */
            if (gang_worker_shared < 0) gang_worker_shared = 0.0;
        }
    }
    
    /* =========================================== */
    /* CASE 4: Vector Partitioned Variable */
    /* Variable partitioned across vector lanes */
    /* =========================================== */
    #pragma acc kernels copyout(vector_only_var)
    {
        #pragma acc loop gang worker
        for (int g = 0; g < GANGS; g++) {
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                /* Vector-private variable */
                int vector_private;
                
                #pragma acc loop vector private(vector_private)
                for (int v = 0; v < VECTOR_LEN; v++) {
                    vector_private = v * 2 + g + w;
                    vector_only_var += vector_private;
                }
            }
        }
    }
    
    /* =========================================== */
    /* CASE 5: Gang+Vector Partitioned Variable */
    /* Variable partitioned across gangs and vectors */
    /* =========================================== */
    #pragma acc parallel num_gangs(GANGS) \
        copy(gang_vector_array) copyin(host_scalar)
    {
        int gang_id = __pgi_gangidx();
        
        /* Each gang has its own vector array */
        #pragma acc loop vector
        for (int v = 0; v < VECTOR_LEN; v++) {
            gang_vector_array[gang_id][v] = 
                (float)(gang_id * VECTOR_LEN + v) * host_scalar * 0.5f;
        }
        
        /* Worker loop to create three-level nesting */
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            /* Access the gang+vector partitioned array */
            float temp_sum = 0.0f;
            #pragma acc loop vector reduction(+:temp_sum)
            for (int v = 0; v < VECTOR_LEN; v++) {
                temp_sum += gang_vector_array[gang_id][v];
            }
            if (w == 0 && temp_sum < 0) {
                gang_vector_array[gang_id][0] = 0.0f;
            }
        }
    }
    
    /* =========================================== */
    /* CASE 6: Worker+Vector Partitioned Variable */
    /* Variable partitioned across workers and vectors */
    /* =========================================== */
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) \
        private(worker_vector_reduction)
    {
        int worker_id = __pgi_workeridx();
        worker_vector_reduction = 0;
        
        /* Vector loop with reduction */
        #pragma acc loop vector reduction(+:worker_vector_reduction)
        for (int v = 0; v < VECTOR_LEN; v++) {
            worker_vector_reduction += v * (worker_id + 1);
        }
        
        /* Conditional to affect partitioning analysis */
        if (worker_id % 2 == 0) {
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN/2; v++) {
                worker_vector_reduction -= v;
            }
        }
    }
    
    /* =========================================== */
    /* CASE 7: Fully Partitioned Variable */
    /* Variable partitioned across all three levels */
    /* =========================================== */
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copyout(fully_partitioned)
    {
        int gang_id = __pgi_gangidx();
        int worker_id = __pgi_workeridx();
        
        /* Fully partitioned 3D array */
        #pragma acc loop vector
        for (int v = 0; v < VECTOR_LEN; v++) {
            fully_partitioned[gang_id][worker_id][v] = 
                (double)(gang_id * WORKERS * VECTOR_LEN + 
                        worker_id * VECTOR_LEN + v) * 0.01;
        }
        
        /* Stencil computation to complicate dependency analysis */
        #pragma acc loop vector
        for (int v = 1; v < VECTOR_LEN - 1; v++) {
            /* Access neighboring elements */
            double left = fully_partitioned[gang_id][worker_id][v-1];
            double right = fully_partitioned[gang_id][worker_id][v+1];
            fully_partitioned[gang_id][worker_id][v] = 
                (left + right) * 0.5;
        }
    }
    
    /* =========================================== */
    /* Complex nested region mixing all patterns */
    /* =========================================== */
    {
        int mixed_partitioning[GANGS][WORKERS][VECTOR_LEN];
        float conditional_scalar;
        
        /* Runtime condition affecting partitioning */
        int use_complex_partitioning = 1;
        
        #pragma acc kernels copyout(mixed_partitioning, conditional_scalar) \
            copyin(use_complex_partitioning, global_matrix)
        {
            #pragma acc loop gang independent
            for (int g = 0; g < GANGS; g++) {
                float gang_local = (float)g * 10.0f;
                
                #pragma acc loop worker independent
                for (int w = 0; w < WORKERS; w++) {
                    int worker_local = w * 100;
                    
                    /* Conditional partitioning */
                    if (use_complex_partitioning) {
                        conditional_scalar = sinf(gang_local) * cosf((float)worker_local);
                    }
                    
                    #pragma acc loop vector independent
                    for (int v = 0; v < VECTOR_LEN; v++) {
                        /* Mix of different partitionings in one expression */
                        mixed_partitioning[g][w][v] = 
                            (int)(gang_local + (float)worker_local + 
                                 (float)v + conditional_scalar * 
                                 global_matrix[g % N][(w * VECTOR_LEN + v) % M]);
                    }
                }
            }
        }
        
        /* Verify computation */
        int checksum = 0;
        for (int g = 0; g < GANGS; g++) {
            for (int w = 0; w < WORKERS; w++) {
                for (int v = 0; v < VECTOR_LEN; v++) {
                    checksum += mixed_partitioning[g][w][v] & 0xFF;
                }
            }
        }
        printf("Final checksum: %d\n", checksum);
    }
    
    /* Final verification output */
    printf("Partitioning test completed successfully.\n");
    printf("Global gang redundant value: %d\n", global_gang_redundant);
    
    return 0;
}
