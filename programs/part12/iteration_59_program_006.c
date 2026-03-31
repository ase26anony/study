/* test_neuter_broadcast.c - Comprehensive test for omp-oacc-neuter-broadcast.cc coverage */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define M 64
#define P 32

/* Pattern A: Various scalar and array variables */
static int global_scalar = 42;
static float global_array[N];

/* Pattern B: Multi-dimensional arrays */
static int multi_dim[M][P];
static double cube[8][16][32];

/* Pattern D: Struct with multiple members */
struct DataPoint {
    int id;
    float value;
    double precision;
    char tag;
};

static struct DataPoint data_points[N];

/* Helper function to initialize data */
void init_data(void) {
    for (int i = 0; i < N; i++) {
        global_array[i] = i * 1.5f;
        data_points[i].id = i;
        data_points[i].value = i * 2.0f;
        data_points[i].precision = i * 3.14159;
        data_points[i].tag = 'A' + (i % 26);
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            multi_dim[i][j] = i * P + j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 16; j++) {
            for (int k = 0; k < 32; k++) {
                cube[i][j][k] = i * 100.0 + j * 10.0 + k;
            }
        }
    }
}

/* Pattern C: Variable-length data via pointers */
void process_dynamic_data(int size) {
    int *dynamic_array = (int *)malloc(size * sizeof(int));
    float *dynamic_float = (float *)malloc(size * sizeof(float));
    
    if (!dynamic_array || !dynamic_float) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }
    
    /* Initialize dynamic arrays */
    for (int i = 0; i < size; i++) {
        dynamic_array[i] = i * 3;
        dynamic_float[i] = i * 0.5f;
    }
    
    /* OpenACC parallel region with complex data mapping */
    #pragma acc parallel loop copyin(dynamic_array[0:size], dynamic_float[0:size]) \
        copyout(global_array[0:N]) copy(multi_dim[0:M][0:P]) \
        copy(cube[0:8][0:16][0:32]) copy(data_points[0:N]) \
        firstprivate(global_scalar) private(i) reduction(+:global_scalar)
    for (int i = 0; i < size; i++) {
        /* Nested loops to create complex data flow */
        for (int j = 0; j < M; j++) {
            /* Conditional operations */
            if (dynamic_array[i] % 2 == 0) {
                for (int k = 0; k < P; k++) {
                    /* Access multi-dimensional array with varying indices */
                    multi_dim[j][k] += dynamic_array[i];
                    
                    /* Vector-level partitioning opportunities */
                    #pragma acc loop vector
                    for (int v = 0; v < 8; v++) {
                        cube[v][j % 16][k % 32] += dynamic_float[i];
                    }
                }
            } else {
                /* Worker-level partitioning opportunities */
                #pragma acc loop worker
                for (int w = 0; w < 4; w++) {
                    int idx = (i + w) % N;
                    global_array[idx] *= dynamic_float[i];
                    data_points[idx].value += dynamic_float[i];
                }
            }
        }
        
        /* Gang-level operations */
        if (i % 16 == 0) {
            global_scalar += dynamic_array[i];
        }
    }
    
    free(dynamic_array);
    free(dynamic_float);
}

/* OpenMP target version for comparison */
void process_omp_target(void) {
    int local_array[N];
    float temp_results[M][P];
    
    /* Initialize local data */
    for (int i = 0; i < N; i++) {
        local_array[i] = i * 2;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < P; j++) {
            temp_results[i][j] = 0.0f;
        }
    }
    
    /* Complex OpenMP target region with nested parallelism */
    #pragma omp target teams distribute parallel for \
        map(to: local_array[0:N], multi_dim[0:M][0:P]) \
        map(tofrom: temp_results[0:M][0:P], cube[0:8][0:16][0:32]) \
        map(tofrom: data_points[0:N]) \
        firstprivate(global_scalar) reduction(+:global_scalar)
    for (int i = 0; i < N; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        
        /* Complex nested loops with conditionals */
        for (int dim1 = 0; dim1 < M; dim1++) {
            /* Worker partitioning pattern */
            #pragma omp parallel for
            for (int dim2 = 0; dim2 < P; dim2++) {
                temp_results[dim1][dim2] += local_array[i] * 0.01f;
                
                /* Vector partitioning opportunities */
                #pragma omp simd
                for (int v = 0; v < 8; v++) {
                    cube[v][dim1 % 16][dim2 % 32] += 
                        (team_id * 1000 + thread_id * 100 + v) * 0.001;
                }
            }
        }
        
        /* Struct member access with different partitioning */
        data_points[i].value += local_array[i] * 0.5f;
        data_points[i].precision *= 1.0001;
        
        /* Conditional gang-level operation */
        if (i % 32 == 0) {
            global_scalar += team_id;
        }
    }
}

/* Mixed OpenACC kernels region */
void process_acc_kernels(void) {
    double local_cube[4][8][16];
    int counters[4][8] = {0};
    
    /* Initialize */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 16; k++) {
                local_cube[i][j][k] = (i + j + k) * 1.5;
            }
        }
    }
    
    /* OpenACC kernels with complex data clauses */
    #pragma acc kernels copyin(local_cube) copyout(counters) \
        create(global_array[0:N/2]) present(multi_dim)
    {
        /* Multiple nested loops to trigger different partitioning */
        #pragma acc loop gang
        for (int g = 0; g < 4; g++) {
            #pragma acc loop worker
            for (int w = 0; w < 8; w++) {
                #pragma acc loop vector
                for (int v = 0; v < 16; v++) {
                    local_cube[g][w][v] *= 2.0;
                    
                    /* Conditional access patterns */
                    if (local_cube[g][w][v] > 50.0) {
                        counters[g][w]++;
                        int idx = (g * 8 + w) % (N/2);
                        global_array[idx] = local_cube[g][w][v];
                        
                        /* Access multi-dim array with transformed indices */
                        multi_dim[g % M][w % P] += idx;
                    }
                }
            }
        }
        
        /* Additional independent loop with different partitioning */
        #pragma acc loop independent
        for (int i = 0; i < N/2; i++) {
            global_array[i] += i * 0.25f;
        }
    }
}

/* Validation function */
int validate_results(void) {
    int errors = 0;
    
    /* Simple validation checks */
    for (int i = 0; i < N; i++) {
        if (data_points[i].id != i) {
            errors++;
        }
    }
    
    printf("Validation: %d errors found\n", errors);
    return errors == 0;
}

int main(void) {
    printf("Starting neuter-broadcast coverage test...\n");
    
    /* Initialize all data structures */
    init_data();
    
    /* Execute OpenACC version with dynamic data */
    printf("Running OpenACC dynamic data processing...\n");
    process_dynamic_data(256);
    
    /* Execute OpenMP target version */
    printf("Running OpenMP target processing...\n");
    process_omp_target();
    
    /* Execute OpenACC kernels version */
    printf("Running OpenACC kernels processing...\n");
    process_acc_kernels();
    
    /* Validate results */
    if (validate_results()) {
        printf("All tests completed successfully!\n");
    } else {
        printf("Some validation errors detected\n");
    }
    
    return 0;
}
