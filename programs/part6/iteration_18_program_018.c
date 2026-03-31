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
float global_matrix[N][M];               /* Multi-dimensional array */
static int module_private_var = 100;

/* Function to initialize matrices */
void init_matrices() {
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (float)(i * M + j) * 0.1f;
        }
    }
}

/* Main function with complex nested parallelism */
int main() {
    /* Local variables with different scopes */
    int host_scalar = 10;
    float host_array[N];
    double nested_private_var;
    
    /* Initialize data */
    init_matrices();
    
    /* Initialize host array */
    for (int i = 0; i < N; i++) {
        host_array[i] = (float)i * 0.5f;
    }
    
    /* ============================================
       CASE 0: Gang Redundant
       ============================================ */
    printf("Testing gang redundant partitioning...\n");
    #pragma acc parallel num_gangs(GANGS) copy(host_scalar)
    {
        /* global_gang_redundant should be marked as gang redundant */
        int local = global_gang_redundant + host_scalar;
        #pragma acc loop gang
        for (int i = 0; i < GANGS; i++) {
            /* Use the variable in gang context */
            if (i == 0) {
                local += 1;
            }
        }
        host_scalar = local;
    }
    
    /* ============================================
       CASE 1: Gang Partitioned  
       ============================================ */
    printf("Testing gang partitioned...\n");
    int gang_partitioned_var = 0;
    #pragma acc parallel num_gangs(GANGS) copy(gang_partitioned_var)
    {
        /* Each gang gets its own copy */
        int gang_private = acc_gang_id();
        
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            if (acc_gang_id() == g) {
                gang_private = g * 100;
            }
        }
        
        gang_partitioned_var = gang_private;
    }
    
    /* ============================================
       CASE 2: Worker Partitioned
       ============================================ */
    printf("Testing worker partitioned...\n");
    float worker_results[WORKERS] = {0};
    
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) copyout(worker_results[0:WORKERS])
    {
        /* Variable partitioned across workers */
        float worker_private;
        
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            worker_private = (float)acc_worker_id() * 10.0f;
            worker_results[acc_worker_id()] = worker_private;
        }
    }
    
    /* ============================================
       CASE 3: Gang+Worker Partitioned
       ============================================ */
    printf("Testing gang+worker partitioned...\n");
    int gw_partitioned[GANGS][WORKERS] = {{0}};
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) \
        copyout(gw_partitioned[0:GANGS][0:WORKERS])
    {
        /* Variable partitioned across both gangs and workers */
        int gw_private;
        
        #pragma acc loop gang worker
        for (int g = 0; g < GANGS; g++) {
            for (int w = 0; w < WORKERS; w++) {
                gw_private = g * 1000 + w * 100;
                gw_partitioned[g][w] = gw_private;
            }
        }
    }
    
    /* ============================================
       CASE 4: Vector Partitioned
       ============================================ */
    printf("Testing vector partitioned...\n");
    float vector_data[N];
    
    #pragma acc kernels copyout(vector_data[0:N])
    {
        /* Vector-only partitioning */
        #pragma acc loop vector
        for (int i = 0; i < N; i++) {
            float vector_private = (float)i * 2.0f;
            vector_data[i] = vector_private;
        }
    }
    
    /* ============================================
       CASE 5: Gang+Vector Partitioned
       ============================================ */
    printf("Testing gang+vector partitioned...\n");
    float gv_result[GANGS][VECTOR_LEN];
    
    #pragma acc parallel num_gangs(GANGS) vector_length(VECTOR_LEN) \
        copyout(gv_result[0:GANGS][0:VECTOR_LEN])
    {
        /* Complex partitioning across gangs and vectors */
        float gv_private;
        
        #pragma acc loop gang vector
        for (int g = 0; g < GANGS; g++) {
            for (int v = 0; v < VECTOR_LEN; v++) {
                gv_private = (float)g * 100.0f + (float)v;
                gv_result[g][v] = gv_private;
            }
        }
    }
    
    /* ============================================
       CASE 6: Worker+Vector Partitioned
       ============================================ */
    printf("Testing worker+vector partitioned...\n");
    float wv_result[WORKERS][VECTOR_LEN];
    
    #pragma acc parallel num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copyout(wv_result[0:WORKERS][0:VECTOR_LEN])
    {
        /* Worker+vector partitioning */
        float wv_private;
        
        #pragma acc loop worker vector
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTOR_LEN; v++) {
                wv_private = (float)w * 50.0f + (float)v * 0.5f;
                wv_result[w][v] = wv_private;
            }
        }
    }
    
    /* ============================================
       CASE 7: Fully Partitioned (Gang+Worker+Vector)
       ============================================ */
    printf("Testing fully partitioned...\n");
    float full_partitioned[GANGS][WORKERS][VECTOR_LEN];
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copyout(full_partitioned[0:GANGS][0:WORKERS][0:VECTOR_LEN])
    {
        /* Fully partitioned variable */
        float fully_private;
        
        /* Multi-dimensional stencil computation to complicate analysis */
        #pragma acc loop gang worker vector collapse(3)
        for (int g = 0; g < GANGS; g++) {
            for (int w = 0; w < WORKERS; w++) {
                for (int v = 0; v < VECTOR_LEN; v++) {
                    /* Complex computation with conditionals */
                    if (v % 2 == 0) {
                        fully_private = (float)g * 1000.0f + 
                                       (float)w * 100.0f + 
                                       (float)v;
                    } else {
                        fully_private = (float)g * 500.0f + 
                                       (float)w * 50.0f + 
                                       (float)v * 2.0f;
                    }
                    
                    /* Access neighboring elements (stencil) */
                    if (v > 0 && v < VECTOR_LEN - 1) {
                        fully_private += (float)(v-1) * 0.1f - (float)(v+1) * 0.1f;
                    }
                    
                    full_partitioned[g][w][v] = fully_private;
                }
            }
        }
    }
    
    /* ============================================
       Mixed complex scenario with conditional partitioning
       ============================================ */
    printf("Testing mixed conditional partitioning...\n");
    int conditional_result = 0;
    int runtime_condition = host_scalar % 3;
    
    #pragma acc parallel num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN) \
        copy(conditional_result)
    {
        /* Variables with different sharing attributes */
        int gang_shared;
        int worker_private;
        float vector_local;
        
        /* Conditional partitioning based on runtime value */
        if (runtime_condition == 0) {
            gang_shared = acc_gang_id();
            #pragma acc loop gang
            for (int g = 0; g < GANGS; g++) {
                if (acc_gang_id() == g) {
                    gang_shared *= 2;
                }
            }
        } else if (runtime_condition == 1) {
            worker_private = acc_worker_id();
            #pragma acc loop worker
            for (int w = 0; w < WORKERS; w++) {
                if (acc_worker_id() == w) {
                    worker_private += w * 10;
                }
            }
        } else {
            vector_local = (float)acc_vector_id();
            #pragma acc loop vector
            for (int v = 0; v < VECTOR_LEN; v++) {
                if (acc_vector_id() == v) {
                    vector_local *= 1.5f;
                }
            }
        }
        
        /* Reduction to prevent dead code elimination */
        #pragma acc atomic update
        conditional_result += gang_shared + worker_private + (int)vector_local;
    }
    
    /* ============================================
       Complex nested kernels with data dependencies
       ============================================ */
    printf("Testing nested kernels with data flow...\n");
    float final_matrix[N][M];
    
    /* First kernel: gang-partitioned computation */
    #pragma acc kernels copyin(global_matrix) copyout(final_matrix)
    {
        #pragma acc loop gang
        for (int i = 0; i < N; i++) {
            float row_private = 0.0f;
            
            #pragma acc loop worker
            for (int j = 0; j < M; j++) {
                float element_private = global_matrix[i][j];
                
                #pragma acc loop vector
                for (int k = 0; k < 4; k++) {
                    element_private += sinf((float)k * 0.1f);
                }
                
                final_matrix[i][j] = element_private;
                row_private += element_private;
            }
            
            /* Use row_private for some computation */
            if (row_private > 1000.0f) {
                #pragma acc loop vector
                for (int j = 0; j < M; j++) {
                    final_matrix[i][j] *= 0.9f;
                }
            }
        }
    }
    
    /* Compute checksum to verify execution */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += final_matrix[i][j];
        }
    }
    
    /* Also sum up results from all partitioning tests */
    double partition_sum = 0.0;
    partition_sum += host_scalar;
    partition_sum += gang_partitioned_var;
    
    for (int i = 0; i < WORKERS; i++) {
        partition_sum += worker_results[i];
    }
    
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            partition_sum += gw_partitioned[g][w];
        }
    }
    
    for (int i = 0; i < N; i++) {
        partition_sum += vector_data[i];
    }
    
    for (int g = 0; g < GANGS; g++) {
        for (int v = 0; v < VECTOR_LEN; v++) {
            partition_sum += gv_result[g][v];
        }
    }
    
    for (int w = 0; w < WORKERS; w++) {
        for (int v = 0; v < VECTOR_LEN; v++) {
            partition_sum += wv_result[w][v];
        }
    }
    
    for (int g = 0; g < GANGS; g++) {
        for (int w = 0; w < WORKERS; w++) {
            for (int v = 0; v < VECTOR_LEN; v++) {
                partition_sum += full_partitioned[g][w][v];
            }
        }
    }
    
    partition_sum += conditional_result;
    
    printf("Final checksum: %f\n", checksum);
    printf("Partition test sum: %f\n", partition_sum);
    printf("All partitioning tests completed.\n");
    
    return 0;
}
