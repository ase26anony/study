/* test_neuter_broadcast.c
 * Comprehensive test to cover all partitioning states in GCC's
 * omp-oacc-neuter-broadcast.cc switch statement (cases 0-7)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 128
#define M 64
#define P 32

/* Pattern A: Different scalar and array types */
static int global_scalar = 42;
static float global_array[N];

/* Pattern B: Multi-dimensional arrays */
static int multi_dim[M][N][P];

/* Pattern C: Dynamic memory */
int *dynamic_array;

/* Pattern D: Struct with multiple members */
struct DataPoint {
    double x;
    double y;
    int id;
    float values[4];
};

struct DataPoint data_points[N];

/* Function containing complex OpenACC region */
void test_openacc_partitioning(void) {
    int i, j, k;
    
    /* Local scalars with different storage classes */
    int local_scalar = 10;
    static int static_scalar = 20;
    register int reg_scalar = 30;
    
    /* Local arrays */
    float local_array[N];
    double local_multi[2][M][P];
    
    /* Initialize local arrays */
    for (i = 0; i < N; i++) {
        local_array[i] = i * 0.5f;
    }
    
    /* Complex OpenACC parallel region with multiple data clauses */
    #pragma acc parallel loop gang vector \
        copyin(global_scalar, global_array[0:N]) \
        copyout(local_array[0:N]) \
        create(local_multi[0:2][0:M][0:P]) \
        private(reg_scalar) \
        firstprivate(local_scalar) \
        reduction(+:static_scalar)
    for (i = 0; i < N; i++) {
        int worker_var = i % M;
        int vector_var = i % P;
        
        /* Access global scalar - likely gang redundant (case 0) */
        local_array[i] += global_scalar;
        
        /* Access global array - partitioning depends on access pattern */
        local_array[i] += global_array[i];
        
        /* Nested loops creating complex data flow */
        for (j = 0; j < M; j++) {
            /* Access multi-dimensional array with different indices
             * This may trigger different partitioning states */
            local_multi[0][j][vector_var] = multi_dim[j][i][vector_var];
            
            /* Conditional operations */
            if (j % 2 == 0) {
                local_multi[1][j][vector_var] = 
                    multi_dim[worker_var][j][vector_var] * 2.0;
            } else {
                local_multi[1][j][vector_var] = 
                    multi_dim[j][worker_var][vector_var] / 2.0;
            }
        }
        
        /* Access struct array members */
        data_points[i].x = i * 1.5;
        data_points[i].y = i * 2.5;
        data_points[i].id = i;
        
        /* Reduction variable */
        static_scalar += i;
        
        /* Vector-private variable */
        reg_scalar = vector_var;
        
        /* Access dynamic memory through pointer */
        if (dynamic_array && i < N/2) {
            local_array[i] += dynamic_array[i];
        }
    }
    
    /* Second parallel region with different partitioning */
    #pragma acc parallel loop gang worker vector \
        copy(multi_dim[0:M][0:N][0:P]) \
        copy(data_points[0:N]) \
        present(dynamic_array[0:N])
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            for (k = 0; k < P; k++) {
                /* Complex access pattern to trigger various partitioning */
                if (k % 3 == 0) {
                    multi_dim[i][j][k] = i + j + k;
                } else if (k % 3 == 1) {
                    multi_dim[i][j][k] = i * j * k;
                } else {
                    multi_dim[i][j][k] = (i << 2) | (j << 1) | k;
                }
                
                /* Cross-thread communication pattern */
                if (i > 0 && j > 0 && k > 0) {
                    multi_dim[i][j][k] += multi_dim[i-1][j][k] / 2;
                    multi_dim[i][j][k] += multi_dim[i][j-1][k] / 3;
                    multi_dim[i][j][k] += multi_dim[i][j][k-1] / 4;
                }
            }
        }
    }
}

/* Function with OpenMP target region */
void test_openmp_partitioning(void) {
    int i, j;
    int omp_local[N];
    int omp_shared = 0;
    
    /* Initialize */
    for (i = 0; i < N; i++) {
        omp_local[i] = i;
    }
    
    /* OpenMP target region with teams and distribute */
    #pragma omp target teams distribute parallel for \
        map(tofrom: omp_local[0:N]) \
        map(to: global_array[0:N]) \
        map(from: multi_dim[0:10][0:10][0:10]) \
        reduction(+:omp_shared)
    for (i = 0; i < N; i++) {
        int team_id = omp_get_team_num();
        int thread_id = omp_get_thread_num();
        
        /* Different access patterns to trigger partitioning analysis */
        omp_local[i] = omp_local[i] * 2 + global_array[i];
        
        /* Nested loop with conditional */
        for (j = 0; j < 10; j++) {
            if ((i + j) % 3 == 0) {
                multi_dim[team_id][j][thread_id % 10] = 
                    omp_local[i] + j + thread_id;
            }
        }
        
        /* Reduction */
        omp_shared += i;
    }
}

/* Helper function to initialize data */
void initialize_data(void) {
    int i, j, k;
    
    /* Initialize global arrays */
    for (i = 0; i < N; i++) {
        global_array[i] = i * 1.0f;
        data_points[i].x = 0.0;
        data_points[i].y = 0.0;
        data_points[i].id = -1;
        for (j = 0; j < 4; j++) {
            data_points[i].values[j] = 0.0f;
        }
    }
    
    /* Initialize multi-dimensional array */
    for (i = 0; i < M; i++) {
        for (j = 0; j < N; j++) {
            for (k = 0; k < P; k++) {
                multi_dim[i][j][k] = i * 10000 + j * 100 + k;
            }
        }
    }
    
    /* Allocate and initialize dynamic memory */
    dynamic_array = (int *)malloc(N * sizeof(int));
    if (dynamic_array) {
        for (i = 0; i < N; i++) {
            dynamic_array[i] = i * 3;
        }
        
        /* Use OpenACC data directive for dynamic array */
        #pragma acc enter data copyin(dynamic_array[0:N])
    }
}

/* Helper function to verify results */
int verify_results(void) {
    int i, errors = 0;
    
    /* Simple verification of data_points */
    for (i = 0; i < N; i++) {
        if (data_points[i].id != i && data_points[i].id != -1) {
            errors++;
        }
    }
    
    /* Check multi_dim array was modified */
    int sum = 0;
    for (i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                sum += multi_dim[i][j][k];
            }
        }
    }
    
    if (sum == 0) {
        errors++;
    }
    
    return errors;
}

/* Cleanup function */
void cleanup_data(void) {
    if (dynamic_array) {
        #pragma acc exit data delete(dynamic_array[0:N])
        free(dynamic_array);
    }
}

int main(void) {
    int errors = 0;
    
    printf("Initializing test data...\n");
    initialize_data();
    
    printf("Testing OpenACC partitioning...\n");
    test_openacc_partitioning();
    
    printf("Testing OpenMP partitioning...\n");
    #ifdef _OPENMP
    test_openmp_partitioning();
    #endif
    
    printf("Verifying results...\n");
    errors = verify_results();
    
    if (errors == 0) {
        printf("All tests passed successfully!\n");
    } else {
        printf("Found %d errors in verification\n", errors);
    }
    
    cleanup_data();
    
    return (errors > 0) ? 1 : 0;
}
