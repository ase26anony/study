/* test_partition_coverage.c
 * Compile with: gcc -O2 -fopenacc -fopenmp -o test_partition test_partition_coverage.c
 * For OpenACC offload: gcc -O2 -fopenacc -foffload=nvptx-none -o test_partition test_partition_coverage.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openacc.h>
#include <omp.h>

/* External function to prevent optimization */
extern void use_result(int);
extern void use_string(const char*);

/* Volatile variables to prevent optimization */
volatile int global_seed = 42;
volatile int force_runtime = 1;

/* Helper to generate runtime-dependent bounds */
int get_bound(int base, int offset) {
    return base + (global_seed % offset);
}

/* Test case 0: Gang redundant (single gang) */
void test_gang_redundant() {
    int n = get_bound(1000, 100);
    float *data = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) data[i] = i * 1.0f;
    
    #pragma acc parallel num_gangs(1) copy(data[0:n])
    {
        int idx = acc_gang_id * 100 + acc_worker_id * 10 + acc_vector_id;
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            data[i] += idx * 0.001f;
        }
    }
    
    /* Force compiler to consider partition mapping */
    if (force_runtime) {
        int check = (int)(data[0] * 1000);
        use_result(check);
    }
    
    free(data);
}

/* Test case 1: Gang partitioned (multiple gangs) */
void test_gang_partitioned() {
    int n = get_bound(1024, 512);
    int *arr = (int*)malloc(n * sizeof(int));
    
    for (int i = 0; i < n; i++) arr[i] = i;
    
    #pragma acc parallel num_gangs(8) copy(arr[0:n])
    {
        #pragma acc loop gang
        for (int i = 0; i < n; i++) {
            arr[i] += acc_gang_id;
        }
    }
    
    /* Conditional that could use partition description */
    int sum = 0;
    for (int i = 0; i < n; i++) sum += arr[i];
    if (sum % 7 == 0) {
        /* This might trigger debug output with partition string */
        #pragma acc parallel num_gangs(4)
        {
            /* Empty but forces partition generation */
        }
    }
    
    free(arr);
}

/* Test case 2: Worker partitioned */
void test_worker_partitioned() {
    int n = get_bound(512, 256);
    float *vec = (float*)malloc(n * sizeof(float));
    
    #pragma acc parallel num_gangs(1) num_workers(4) copyout(vec[0:n])
    {
        int worker_id = acc_worker_id;
        #pragma acc loop worker
        for (int i = 0; i < n; i++) {
            vec[i] = worker_id * 100.0f + i;
        }
    }
    
    /* Prevent dead code elimination */
    asm volatile("" : : "r"(vec[0]));
    
    free(vec);
}

/* Test case 3: Gang+Worker partitioned */
void test_gang_worker_partitioned() {
    int rows = get_bound(64, 32);
    int cols = get_bound(128, 64);
    int size = rows * cols;
    double *matrix = (double*)malloc(size * sizeof(double));
    
    #pragma acc parallel num_gangs(4) num_workers(2) copy(matrix[0:size])
    {
        int gid = acc_gang_id;
        int wid = acc_worker_id;
        
        #pragma acc loop gang worker
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                int idx = i * cols + j;
                matrix[idx] = gid * 1000.0 + wid * 100.0 + idx;
            }
        }
    }
    
    /* Runtime check that could trigger diagnostic */
    double max_val = 0.0;
    for (int i = 0; i < size; i++) {
        if (matrix[i] > max_val) max_val = matrix[i];
    }
    if (max_val > 10000.0 && force_runtime) {
        use_result((int)max_val);
    }
    
    free(matrix);
}

/* Test case 4: Vector partitioned */
void test_vector_partitioned() {
    int n = get_bound(1024, 512);
    int *data = (int*)malloc(n * sizeof(int));
    
    #pragma acc parallel num_gangs(1) vector_length(32) copy(data[0:n])
    {
        #pragma acc loop vector
        for (int i = 0; i < n; i++) {
            data[i] = acc_vector_id + i * 2;
        }
    }
    
    /* Use result to prevent optimization */
    int total = 0;
    for (int i = 0; i < n; i++) total ^= data[i];
    use_result(total);
    
    free(data);
}

/* Test case 5: Gang+Vector partitioned */
void test_gang_vector_partitioned() {
    int n = get_bound(2048, 1024);
    float *a = (float*)malloc(n * sizeof(float));
    float *b = (float*)malloc(n * sizeof(float));
    
    for (int i = 0; i < n; i++) {
        a[i] = i * 0.5f;
        b[i] = i * 0.25f;
    }
    
    #pragma acc parallel num_gangs(8) vector_length(64) copy(a[0:n], b[0:n])
    {
        #pragma acc loop gang vector
        for (int i = 0; i < n; i++) {
            a[i] = a[i] * 2.0f + b[i] + acc_gang_id * 0.1f;
        }
    }
    
    /* Complex dependency to force partitioning */
    float sum = 0.0f;
    #pragma acc parallel num_gangs(4) vector_length(32) reduction(+:sum) copyin(a[0:n])
    {
        #pragma acc loop gang vector reduction(+:sum)
        for (int i = 0; i < n; i++) {
            sum += a[i];
        }
    }
    
    if (sum > 0.0f) {
        asm volatile("" : : "r"(sum));
    }
    
    free(a); free(b);
}

/* Test case 6: Worker+Vector partitioned */
void test_worker_vector_partitioned() {
    int n = get_bound(1024, 512);
    int m = get_bound(32, 16);
    int *result = (int*)malloc(m * sizeof(int));
    
    #pragma acc parallel num_gangs(1) num_workers(4) vector_length(16) copyout(result[0:m])
    {
        int worker_vec_id = acc_worker_id * 100 + acc_vector_id;
        #pragma acc loop worker vector
        for (int i = 0; i < m; i++) {
            result[i] = worker_vec_id + i * 1000;
        }
    }
    
    /* Force materialization through external call */
    for (int i = 0; i < m; i++) {
        if (result[i] % 2 == 0) {
            use_result(result[i]);
        }
    }
    
    free(result);
}

/* Test case 7: Fully partitioned (Gang+Worker+Vector) */
void test_fully_partitioned() {
    int x = get_bound(16, 8);
    int y = get_bound(32, 16);
    int z = get_bound(64, 32);
    int size = x * y * z;
    float *grid = (float*)malloc(size * sizeof(float));
    
    /* Complex nested loops with full partitioning */
    #pragma acc parallel num_gangs(4) num_workers(2) vector_length(8) copy(grid[0:size])
    {
        #pragma acc loop gang
        for (int i = 0; i < x; i++) {
            #pragma acc loop worker
            for (int j = 0; j < y; j++) {
                #pragma acc loop vector
                for (int k = 0; k < z; k++) {
                    int idx = (i * y + j) * z + k;
                    grid[idx] = acc_gang_id * 1000.0f + 
                               acc_worker_id * 100.0f + 
                               acc_vector_id * 10.0f + 
                               idx * 0.1f;
                }
            }
        }
    }
    
    /* Verification that could trigger diagnostic output */
    int errors = 0;
    for (int i = 0; i < size; i++) {
        if (grid[i] < 0 || grid[i] > 100000.0f) errors++;
    }
    
    if (errors > 0 && force_runtime) {
        /* This condition might cause the compiler to generate 
           diagnostic output with partition strings */
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(4)
        {
            /* Force generation of fully partitioned code */
            int idx = acc_gang_id * 100 + acc_worker_id * 10 + acc_vector_id;
            asm volatile("" : : "r"(idx));
        }
    }
    
    free(grid);
}

/* Test default case (illegal partition codes) */
void test_illegal_partitions() {
    /* Try to generate invalid partition codes through 
       complex control flow and variable dimensions */
    int mode = global_seed % 10;
    
    if (mode > 7) {
        /* This might generate out-of-bounds partition codes */
        int gangs = (mode == 8) ? -1 : 0;
        int workers = (mode == 9) ? -1 : 0;
        
        /* Use macros to generate potentially invalid directives */
        #define EXEC_PARALLEL(g, w, v) \
            _Pragma("acc parallel num_gangs(g) num_workers(w) vector_length(v)") \
            { \
                int id = acc_gang_id; \
                asm volatile("" : : "r"(id)); \
            }
        
        /* These might generate illegal partition mappings */
        if (gangs < 0 || workers < 0) {
            /* Could trigger default case */
            volatile int x = 0;
            asm volatile("" : : "r"(x));
        }
    }
}

/* OpenMP versions to trigger similar logic */
#ifdef _OPENMP
void test_omp_partitioning() {
    int n = get_bound(1000, 500);
    int *array = (int*)malloc(n * sizeof(int));
    
    /* OpenMP target with teams and distribute */
    #pragma omp target teams distribute parallel for map(tofrom: array[0:n]) \
                    num_teams(4) thread_limit(32)
    for (int i = 0; i < n; i++) {
        array[i] = omp_get_team_num() * 1000 + omp_get_thread_num() + i;
    }
    
    /* Nested parallelism for combined partitioning */
    #pragma omp target teams distribute parallel for simd \
                    num_teams(2) thread_limit(64) simdlen(8)
    for (int i = 0; i < n; i++) {
        array[i] += i % 7;
    }
    
    free(array);
}
#endif

/* Main test driver */
int main(int argc, char **argv) {
    printf("Testing partition mapping coverage...\n");
    
    /* Force runtime execution paths */
    if (argc > 1) {
        global_seed = atoi(argv[1]);
    }
    
    /* Execute all test cases systematically */
    test_gang_redundant();          /* Case 0 */
    test_gang_partitioned();        /* Case 1 */
    test_worker_partitioned();      /* Case 2 */
    test_gang_worker_partitioned(); /* Case 3 */
    test_vector_partitioned();      /* Case 4 */
    test_gang_vector_partitioned(); /* Case 5 */
    test_worker_vector_partitioned(); /* Case 6 */
    test_fully_partitioned();       /* Case 7 */
    
    /* Test illegal/edge cases */
    test_illegal_partitions();      /* Default case */
    
    #ifdef _OPENMP
    test_omp_partitioning();        /* OpenMP variants */
    #endif
    
    printf("All partition tests completed.\n");
    
    /* Final check that could trigger diagnostic */
    if (force_runtime && global_seed % 13 == 0) {
        /* Force one more parallel region with variable partitioning */
        int dim = get_bound(100, 50);
        float *final = (float*)malloc(dim * sizeof(float));
        
        #pragma acc parallel num_gangs(2) num_workers(2) vector_length(4) copy(final[0:dim])
        {
            #pragma acc loop gang worker vector
            for (int i = 0; i < dim; i++) {
                final[i] = i * 0.01f;
            }
        }
        
        free(final);
    }
    
    return 0;
}

/* Dummy implementation of external functions */
void use_result(int val) {
    volatile int dummy = val;
    (void)dummy;
}

void use_string(const char *str) {
    volatile const char *dummy = str;
    (void)dummy;
}
