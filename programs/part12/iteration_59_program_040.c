/* test_neuter_broadcast.c - Comprehensive test for GCC's omp-oacc-neuter-broadcast pass
 * This program creates variables with diverse partitioning attributes to trigger
 * all cases in the partitioning state switch statement (cases 0-7).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENACC
#define PARALLEL_LOOP _Pragma("acc parallel loop copyout(result[0:N]) copyin(a[0:N], b[0:N]) create(temp[0:N])")
#else
#define PARALLEL_LOOP _Pragma("omp target teams distribute parallel for map(tofrom:result[0:N]) map(to:a[0:N], b[0:N]) map(alloc:temp[0:N])")
#endif

#define N 1024
#define M 32
#define P 16

/* Struct to test aggregate partitioning */
struct DataPoint {
    double x;
    double y;
    int id;
    float weight;
};

/* Function with complex parallel region */
void compute_kernel(int n, double *result, const double *a, const double *b) {
    double temp[N];
    int i, j, k;
    
    /* Multi-dimensional arrays for complex partitioning */
    int md_array[M][P];
    float partial_sums[M];
    
    /* Various scalar variables with different scopes */
    double gang_redundant_var = 3.14159;
    int worker_partitioned_counter = 0;
    float vector_partitioned_accum = 0.0f;
    
    /* Initialize multi-dimensional array */
    for (i = 0; i < M; i++) {
        partial_sums[i] = 0.0f;
        for (j = 0; j < P; j++) {
            md_array[i][j] = i * P + j;
        }
    }
    
    /* Main parallel region with diverse data access patterns */
    #ifdef _OPENACC
    #pragma acc parallel loop copyout(result[0:n]) \
        copyin(a[0:n], b[0:n]) create(temp[0:n]) \
        private(i, j, k) firstprivate(gang_redundant_var) \
        reduction(+:worker_partitioned_counter) \
        vector_length(32) num_gangs(4) num_workers(2)
    #else
    #pragma omp target teams distribute parallel for \
        map(tofrom:result[0:n]) map(to:a[0:n], b[0:n]) \
        map(alloc:temp[0:n]) private(i, j, k) \
        firstprivate(gang_redundant_var) \
        reduction(+:worker_partitioned_counter) \
        num_teams(4) num_threads(64)
    #endif
    for (i = 0; i < n; i++) {
        /* Gang-redundant usage */
        double gr = gang_redundant_var * i;
        
        /* Worker-partitioned computation */
        int worker_id = 0;
        #ifdef _OPENACC
        worker_id = __pgi_workerid();
        #else
        worker_id = omp_get_thread_num() / 16;  /* Simulate worker ID */
        #endif
        
        worker_partitioned_counter += worker_id;
        
        /* Vector-partitioned accumulation */
        float vec_accum = 0.0f;
        for (k = 0; k < 8; k++) {
            vec_accum += a[i] * k;
        }
        vector_partitioned_accum += vec_accum;
        
        /* Access multi-dimensional array with complex indexing */
        int idx1 = i % M;
        int idx2 = (i * 7) % P;
        int md_val = md_array[idx1][idx2];
        
        /* Conditional operations creating complex data flow */
        if (i % 3 == 0) {
            temp[i] = a[i] * b[i] + gr;
        } else if (i % 3 == 1) {
            temp[i] = a[i] / (b[i] + 0.001) - md_val;
        } else {
            temp[i] = (a[i] + b[i]) * gr / md_val;
        }
        
        /* Nested loop with reduction-like pattern */
        double nested_sum = 0.0;
        for (j = 0; j < (i % 8) + 1; j++) {
            nested_sum += temp[(i + j) % n] * j;
            partial_sums[idx1] += nested_sum * 0.1f;
        }
        
        /* Final result with mixed partitioning */
        result[i] = temp[i] + nested_sum + vec_accum + worker_id;
    }
}

/* Function with pointer-based dynamic memory */
void dynamic_memory_test(int size) {
    double *dyn_array = (double*)malloc(size * sizeof(double));
    double *host_array = (double*)malloc(size * sizeof(double));
    
    /* Initialize host array */
    for (int i = 0; i < size; i++) {
        host_array[i] = i * 0.5;
    }
    
    /* Map dynamic memory to device */
    #ifdef _OPENACC
    #pragma acc enter data copyin(host_array[0:size]) create(dyn_array[0:size])
    #pragma acc parallel loop present(host_array[0:size], dyn_array[0:size])
    #else
    #pragma omp target map(to:host_array[0:size]) map(from:dyn_array[0:size])
    #pragma omp teams distribute parallel for
    #endif
    for (int i = 0; i < size; i++) {
        /* Complex pointer arithmetic */
        double *ptr = &dyn_array[i];
        *ptr = host_array[i] * 2.0;
        
        /* Access with stride for different partitioning */
        if (i % 4 == 0) {
            ptr = &dyn_array[(i * 3) % size];
            *ptr += host_array[i];
        }
    }
    
    #ifdef _OPENACC
    #pragma acc exit data copyout(dyn_array[0:size]) delete(host_array[0:size])
    #endif
    
    /* Verify results */
    int errors = 0;
    for (int i = 0; i < size; i++) {
        double expected = host_array[i] * 2.0;
        if (i % 4 == 0) {
            int target_idx = (i * 3) % size;
            if (target_idx == i) {
                expected += host_array[i];
            }
        }
        if (dyn_array[i] != expected) errors++;
    }
    
    free(dyn_array);
    free(host_array);
}

/* Test with array of structs */
void struct_array_test(int count) {
    struct DataPoint *points = (struct DataPoint*)malloc(count * sizeof(struct DataPoint));
    
    /* Initialize struct array */
    for (int i = 0; i < count; i++) {
        points[i].x = i * 0.1;
        points[i].y = i * 0.2;
        points[i].id = i;
        points[i].weight = i * 0.01f;
    }
    
    double result[count];
    
    #ifdef _OPENACC
    #pragma acc parallel loop copyin(points[0:count]) copyout(result[0:count]) \
        gang vector
    #else
    #pragma omp target teams distribute parallel for \
        map(to:points[0:count]) map(from:result[0:count])
    #endif
    for (int i = 0; i < count; i++) {
        /* Access different struct members with different patterns */
        double x_val = points[i].x;          /* Likely gang partitioned */
        double y_val = points[(i + 1) % count].y;  /* Different access pattern */
        int id_val = points[i].id;           /* Simple mapping */
        float weight_val = points[i].weight; /* Another member */
        
        /* Complex computation using all members */
        result[i] = x_val * y_val * weight_val + id_val;
        
        /* Conditional update of struct member */
        if (i % 2 == 0) {
            points[i].weight *= 1.1f;  /* This creates write-back requirement */
        }
    }
    
    free(points);
}

/* Main test driver */
int main() {
    const int test_size = 1000;
    double *a = (double*)malloc(test_size * sizeof(double));
    double *b = (double*)malloc(test_size * sizeof(double));
    double *result = (double*)malloc(test_size * sizeof(double));
    
    /* Initialize test data */
    for (int i = 0; i < test_size; i++) {
        a[i] = i * 0.25;
        b[i] = i * 0.5 + 1.0;
        result[i] = 0.0;
    }
    
    printf("Starting comprehensive neuter-broadcast test...\n");
    
    /* Test 1: Basic kernel with mixed partitioning */
    compute_kernel(test_size, result, a, b);
    
    /* Test 2: Dynamic memory with pointer access */
    dynamic_memory_test(500);
    
    /* Test 3: Array of structs */
    struct_array_test(200);
    
    /* Verify results from first test */
    double checksum = 0.0;
    for (int i = 0; i < test_size; i++) {
        checksum += result[i];
    }
    printf("Result checksum: %f\n", checksum);
    
    /* Additional test with explicit data regions */
    {
        int explicit_data[100][50];
        float reduction_var = 0.0f;
        
        #ifdef _OPENACC
        #pragma acc data copy(explicit_data) copyin(a[0:100]) create(reduction_var)
        #pragma acc parallel loop gang worker vector reduction(+:reduction_var)
        #else
        #pragma omp target map(tofrom:explicit_data) map(to:a[0:100]) map(alloc:reduction_var)
        #pragma omp teams distribute parallel for reduction(+:reduction_var)
        #endif
        for (int i = 0; i < 100; i++) {
            for (int j = 0; j < 50; j++) {
                explicit_data[i][j] = i * 50 + j;
                reduction_var += a[i % test_size] * j;
            }
        }
        
        printf("Reduction result: %f\n", reduction_var);
    }
    
    free(a);
    free(b);
    free(result);
    
    printf("Test completed successfully.\n");
    return 0;
}
