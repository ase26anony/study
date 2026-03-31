/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables at different scopes */
int global_gang_redundant = 42;          /* Should become gang redundant (case 0) */
float global_matrix[N][M];               /* Base for various partitioning */
static int file_static_counter = 0;

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) * 0.001f;
        }
    }
}

/* Main function with complex partitioning scenarios */
int main() {
    /* Local variables with different scopes */
    int gang_partitioned_scalar = 0;     /* Target: case 1 */
    float worker_partitioned_array[WORKERS]; /* Target: case 2 */
    double gang_worker_partitioned[GANGS][WORKERS]; /* Target: case 3 */
    int vector_partitioned[N];           /* Target: case 4 */
    float gang_vector_partitioned[GANGS][VECTOR_LEN]; /* Target: case 5 */
    double worker_vector_partitioned[WORKERS][VECTOR_LEN]; /* Target: case 6 */
    int fully_partitioned[GANGS][WORKERS][VECTOR_LEN]; /* Target: case 7 */
    
    /* Initialize data */
    init_matrices();
    
    /* SCENARIO 1: Gang redundant variable (case 0) */
    /* This variable should be broadcast to all gangs */
    #pragma acc parallel copyin(global_gang_redundant) \
                         num_gangs(GANGS) num_workers(1) vector_length(1)
    {
        /* Access the gang-redundant variable */
        if (acc_on_device(acc_device_not_host)) {
            /* Use it in a computation */
            int local = global_gang_redundant + acc_gang();
        }
    }
    
    /* SCENARIO 2: Gang partitioned scalar (case 1) */
    #pragma acc parallel private(gang_partitioned_scalar) \
                         num_gangs(GANGS) num_workers(1) vector_length(1)
    {
        gang_partitioned_scalar = acc_gang() * 100;
        
        /* Nested loop to force partitioning analysis */
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            gang_partitioned_scalar += g;
        }
    }
    
    /* SCENARIO 3: Worker partitioned array (case 2) */
    #pragma acc parallel copyout(worker_partitioned_array) \
                         num_gangs(1) num_workers(WORKERS) vector_length(1)
    {
        int worker_id = acc_worker();
        worker_partitioned_array[worker_id] = worker_id * 2.0f;
        
        /* Conditional access to force analysis */
        if (worker_id % 2 == 0) {
            worker_partitioned_array[worker_id] += global_gang_redundant;
        }
    }
    
    /* SCENARIO 4: Gang+Worker partitioned (case 3) */
    #pragma acc parallel copyout(gang_worker_partitioned) \
                         num_gangs(GANGS) num_workers(WORKERS) vector_length(1)
    {
        int gang_id = acc_gang();
        int worker_id = acc_worker();
        
        gang_worker_partitioned[gang_id][worker_id] = 
            (double)gang_id * 100.0 + (double)worker_id;
            
        /* Access with stencil pattern */
        if (gang_id > 0 && worker_id > 0) {
            gang_worker_partitioned[gang_id][worker_id] += 
                gang_worker_partitioned[gang_id-1][worker_id-1] * 0.5;
        }
    }
    
    /* SCENARIO 5: Vector partitioned array (case 4) */
    #pragma acc kernels copyout(vector_partitioned)
    {
        #pragma acc loop independent vector
        for (int i = 0; i < N; i++) {
            vector_partitioned[i] = i * 3;
            
            /* Complex access pattern */
            if (i > 0 && i < N-1) {
                vector_partitioned[i] += vector_partitioned[i-1] / 2;
            }
        }
    }
    
    /* SCENARIO 6: Gang+Vector partitioned (case 5) */
    #pragma acc parallel copyout(gang_vector_partitioned) \
                         num_gangs(GANGS) vector_length(VECTOR_LEN)
    {
        int gang_id = acc_gang();
        int vector_id = acc_vector();
        
        gang_vector_partitioned[gang_id][vector_id] = 
            sinf((float)gang_id * 0.1f) * cosf((float)vector_id * 0.01f);
            
        /* Multi-dimensional stencil access */
        if (vector_id > 0) {
            gang_vector_partitioned[gang_id][vector_id] += 
                gang_vector_partitioned[gang_id][vector_id-1] * 0.3f;
        }
    }
    
    /* SCENARIO 7: Worker+Vector partitioned (case 6) */
    #pragma acc parallel copyout(worker_vector_partitioned) \
                         num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        int worker_id = acc_worker();
        int vector_id = acc_vector();
        
        worker_vector_partitioned[worker_id][vector_id] = 
            (double)worker_id * 50.0 + (double)vector_id;
            
        /* Conditional partitioning based on runtime value */
        if (worker_id % 2 == 0) {
            worker_vector_partitioned[worker_id][vector_id] *= 2.0;
        } else {
            worker_vector_partitioned[worker_id][vector_id] /= 2.0;
        }
    }
    
    /* SCENARIO 8: Fully partitioned 3D array (case 7) */
    /* Complex nested parallelism with all levels */
    #pragma acc parallel copyout(fully_partitioned) \
                         num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        int gang_id = acc_gang();
        int worker_id = acc_worker();
        int vector_id = acc_vector();
        
        /* Each element gets unique computation */
        fully_partitioned[gang_id][worker_id][vector_id] = 
            gang_id * 10000 + worker_id * 100 + vector_id;
            
        /* Complex data dependency across dimensions */
        if (gang_id > 0) {
            fully_partitioned[gang_id][worker_id][vector_id] += 
                fully_partitioned[gang_id-1][worker_id][vector_id] % 17;
        }
        if (worker_id > 0) {
            fully_partitioned[gang_id][worker_id][vector_id] += 
                fully_partitioned[gang_id][worker_id-1][vector_id] % 23;
        }
        if (vector_id > 0) {
            fully_partitioned[gang_id][worker_id][vector_id] += 
                fully_partitioned[gang_id][worker_id][vector_id-1] % 31;
        }
    }
    
    /* SCENARIO 9: Mixed partitioning in kernels region */
    /* This should trigger analysis of multiple variables simultaneously */
    #pragma acc kernels copy(global_matrix) \
                        copyin(global_gang_redundant) \
                        create(gang_partitioned_scalar)
    {
        /* Loop with gang partitioning */
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            int gang_private = i * 10;  /* Should be gang partitioned */
            
            /* Nested worker loop */
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                float worker_private = global_matrix[i][j];  /* Worker partitioned */
                
                /* Innermost vector loop */
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    /* Fully partitioned computation */
                    global_matrix[i][j] += 
                        sinf(worker_private * 0.1f + (float)k) * 
                        (float)gang_private * 
                        (float)global_gang_redundant;
                }
            }
        }
    }
    
    /* SCENARIO 10: Runtime-dependent partitioning */
    /* Use if clause to create conditional partitioning */
    int dynamic_size = N / 2;
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(16) \
                         if(dynamic_size > 256)
    {
        /* Variables that may be partitioned differently based on condition */
        int conditional_var;
        
        #pragma acc loop gang worker
        for (int i = 0; i < dynamic_size; i++) {
            conditional_var = i * 2;
            
            #pragma acc loop vector
            for (int j = 0; j < 16; j++) {
                /* Access pattern changes partitioning */
                if (j % 2 == 0) {
                    conditional_var += j;
                } else {
                    conditional_var -= j;
                }
            }
        }
    }
    
    /* Verify results by computing checksums */
    int checksum = 0;
    float fchecksum = 0.0f;
    
    /* Process fully_partitioned array */
    #pragma acc parallel copyin(fully_partitioned) reduction(+:checksum)
    {
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < GANGS; g++) {
            for (int w = 0; w < WORKERS; w++) {
                for (int v = 0; v < VECTOR_LEN; v++) {
                    checksum += fully_partitioned[g][w][v] % 100;
                }
            }
        }
    }
    
    /* Process global_matrix */
    #pragma acc parallel copy(global_matrix) reduction(+:fchecksum)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            #pragma acc loop worker vector
            for (int j = 0; j < M; j++) {
                fchecksum += global_matrix[i][j];
            }
        }
    }
    
    printf("Checksum results:\n");
    printf("Integer checksum: %d\n", checksum);
    printf("Float checksum: %f\n", fchecksum);
    
    /* Print some sample values to ensure computation happened */
    printf("Sample values from partitioned arrays:\n");
    printf("Gang+Worker[1][2] = %f\n", gang_worker_partitioned[1][2]);
    printf("Worker+Vector[3][10] = %f\n", worker_vector_partitioned[3][10]);
    printf("FullyPartitioned[1][2][3] = %d\n", fully_partitioned[1][2][3]);
    
    return 0;
}
