/* test_omp_acc_partitioning.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 512
#define GANGS 2
#define WORKERS 4
#define VECTOR_LEN 32

/* Global variables to create complex scoping */
int global_gang_redundant = 0;  /* Should become case 0 */
float global_matrix[N][M];
static double static_partial;

/* Function to create nested parallelism scenarios */
void complex_partitioning_test() {
    int gang_private = 10;           /* Target case 1 */
    float worker_partitioned[M];     /* Target case 2 */
    double gang_worker_shared;       /* Target case 3 */
    int vector_only[N];              /* Target case 4 */
    long gang_vector_private;        /* Target case 5 */
    short worker_vector_array[64];   /* Target case 6 */
    char fully_partitioned[128];     /* Target case 7 */
    
    /* Initialize arrays */
    for (int i = 0; i < M; i++) worker_partitioned[i] = i * 0.5f;
    for (int i = 0; i < N; i++) vector_only[i] = i % 256;
    memset(worker_vector_array, 0, sizeof(worker_vector_array));
    memset(fully_partitioned, 1, sizeof(fully_partitioned));
    
    /* CASE 0: Gang Redundant - variable accessible by all gangs */
    #pragma acc parallel copy(global_gang_redundant) \
                         num_gangs(GANGS) num_workers(1) vector_length(1)
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            /* All gangs read/write same global */
            global_gang_redundant += g;
        }
    }
    
    /* CASE 1: Gang Partitioned - private per gang */
    #pragma acc parallel copyin(gang_private) \
                         num_gangs(GANGS) num_workers(1) vector_length(1)
    {
        int gang_local = gang_private;  /* Becomes gang partitioned */
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            gang_local += g * 100;
        }
        /* Conditional to affect partitioning analysis */
        if (gang_local > 500) {
            gang_private = gang_local;
        }
    }
    
    /* CASE 2 & 4: Worker and Vector partitioned in kernels region */
    #pragma acc kernels copy(worker_partitioned, vector_only)
    {
        /* Worker partitioned array */
        #pragma acc loop worker
        for (int w = 0; w < WORKERS; w++) {
            int start = w * (M / WORKERS);
            int end = (w + 1) * (M / WORKERS);
            for (int i = start; i < end; i++) {
                worker_partitioned[i] *= 2.0f;
            }
        }
        
        /* Vector partitioned array */
        #pragma acc loop vector
        for (int v = 0; v < N; v++) {
            vector_only[v] += v % 128;
        }
    }
    
    /* CASE 3: Gang+Worker Partitioned with complex nesting */
    #pragma acc parallel copy(gang_worker_shared) \
                         num_gangs(GANGS) num_workers(WORKERS) vector_length(1)
    {
        #pragma acc loop gang
        for (int g = 0; g < GANGS; g++) {
            double gang_sum = 0.0;
            #pragma acc loop worker reduction(+:gang_sum)
            for (int w = 0; w < WORKERS; w++) {
                gang_sum += (g * 1000.0) + (w * 100.0);
            }
            gang_worker_shared = gang_sum;
        }
    }
    
    /* CASE 5: Gang+Vector Partitioned with 2D access pattern */
    #pragma acc parallel copy(gang_vector_private) \
                         num_gangs(GANGS) num_workers(1) vector_length(VECTOR_LEN)
    {
        long temp = 0;
        #pragma acc loop gang vector
        for (int i = 0; i < N; i++) {
            temp += i * (i % 16);
        }
        gang_vector_private = temp;
    }
    
    /* CASE 6: Worker+Vector Partitioned with stencil computation */
    #pragma acc parallel copy(worker_vector_array) \
                         num_gangs(1) num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        #pragma acc loop worker vector
        for (int i = 1; i < 63; i++) {
            /* Stencil access pattern complicates analysis */
            worker_vector_array[i] = (worker_vector_array[i-1] + 
                                     worker_vector_array[i] + 
                                     worker_vector_array[i+1]) / 3;
        }
    }
    
    /* CASE 7: Fully Partitioned (gang+worker+vector) */
    #pragma acc parallel copy(fully_partitioned) \
                         num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        #pragma acc loop gang worker vector collapse(2)
        for (int g = 0; g < GANGS; g++) {
            for (int w = 0; w < WORKERS; w++) {
                for (int v = 0; v < VECTOR_LEN; v++) {
                    int idx = (g * WORKERS * VECTOR_LEN) + 
                              (w * VECTOR_LEN) + v;
                    if (idx < 128) {
                        fully_partitioned[idx] += g + w + v;
                    }
                }
            }
        }
    }
    
    /* Mixed partitioning in single complex region */
    #pragma acc parallel copy(global_matrix) \
                         copyin(static_partial) \
                         num_gangs(GANGS) num_workers(WORKERS) vector_length(VECTOR_LEN)
    {
        /* Multi-dimensional array with complex access */
        #pragma acc loop gang worker
        for (int i = 0; i < N; i++) {
            #pragma acc loop vector
            for (int j = 0; j < M; j++) {
                /* Conditional partitioning paths */
                if ((i + j) % 2 == 0) {
                    global_matrix[i][j] = i * j * 0.1f;
                } else {
                    global_matrix[i][j] = (i + j) * 0.05f + static_partial;
                }
                
                /* Cross-element dependency */
                if (j > 0) {
                    global_matrix[i][j] += global_matrix[i][j-1] * 0.01f;
                }
            }
        }
    }
}

/* Additional test with OpenMP offloading for cross-validation */
void omp_offloading_test() {
    int omp_shared_var = 42;
    int omp_private_arr[256];
    
    #pragma omp target teams distribute parallel for \
                map(tofrom: omp_shared_var) \
                map(tofrom: omp_private_arr[0:256]) \
                num_teams(GANGS) thread_limit(WORKERS*VECTOR_LEN)
    for (int i = 0; i < 256; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        omp_private_arr[i] = team_id * 1000 + thread_id;
        #pragma omp atomic
        omp_shared_var += 1;
    }
}

int main() {
    /* Initialize global data */
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            global_matrix[i][j] = (i + j) * 0.01f;
        }
    }
    static_partial = 3.14159;
    
    printf("Starting partitioning tests...\n");
    
    /* Run OpenACC tests */
    complex_partitioning_test();
    
    /* Run OpenMP offloading if supported */
    #ifdef _OPENMP
    omp_offloading_test();
    #endif
    
    /* Compute checksum to prevent dead code elimination */
    double checksum = 0.0;
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            checksum += global_matrix[i][j];
        }
    }
    
    printf("Final checksum: %f\n", checksum);
    printf("Global gang redundant value: %d\n", global_gang_redundant);
    
    return 0;
}
