/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Pseudo-random generator for runtime variability */
static unsigned int lcg_seed = 123456789;

static unsigned int lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return (lcg_seed >> 16) & 0x7FFF;
}

/* Function attribute to force optimization and tree dumping */
__attribute__((optimize("O2")))
void test_reduction_temporaries(int *arr1, int *arr2, int n, volatile int flag) {
    int sum = 0;
    long product = 1;
    int max_val = arr1[0];
    float float_sum = 0.0f;
    
    /* Complex reduction with multiple operators and data-dependent access */
    #pragma omp parallel for reduction(+:sum, float_sum) \
                             reduction(*:product) \
                             reduction(max:max_val) \
                             private(arr2) \
                             schedule(dynamic, 8)
    for (int i = 0; i < n; i++) {
        /* Data-dependent array access to prevent optimization */
        int idx = (i + flag) % n;
        sum += arr1[idx] * (i % 3);
        
        /* Avoid zero for product reduction */
        product *= (arr1[idx] % 10) + 1;
        
        /* Conditional max reduction */
        if (arr1[idx] > max_val || (i % 7 == 0)) {
            max_val = arr1[idx];
        }
        
        /* Float reduction with conversion */
        float_sum += (float)arr1[idx] * 0.5f;
        
        /* Modify arr2 to create side effects */
        arr2[(i * 17 + flag) % n] += i;
    }
    
    /* Prevent dead code elimination */
    __builtin_printf("Reduction results: %d %ld %d %.2f\n", 
                     sum, product, max_val, float_sum);
}

__attribute__((optimize("O2")))
void test_condition_temporaries(int *arr, int n, volatile int cond_flag) {
    volatile int runtime_cond = cond_flag;
    
    /* OMP parallel with if clause - may generate _condtemp_ */
    #pragma omp parallel if(runtime_cond > 0) num_threads(4)
    {
        int thread_id = omp_get_thread_num();
        
        /* Task with if clause inside parallel region */
        #pragma omp task if(thread_id % 2 == 0) firstprivate(runtime_cond)
        {
            int local_sum = 0;
            for (int i = thread_id; i < n; i += omp_get_num_threads()) {
                local_sum += arr[i];
            }
            arr[thread_id % n] = local_sum;
        }
        
        #pragma omp taskwait
        
        /* Nested parallel region with if clause */
        #pragma omp parallel if(runtime_cond < 100) num_threads(2)
        {
            #pragma omp for
            for (int i = 0; i < n/2; i++) {
                arr[i] *= 2;
            }
        }
    }
    
    /* Target teams with if clause */
    #pragma omp target teams if(runtime_cond != 0) \
                            num_teams(2) thread_limit(32) \
                            map(tofrom: arr[0:n/4])
    {
        #pragma omp distribute parallel for
        for (int i = 0; i < n/4; i++) {
            arr[i] += i;
        }
    }
    
    __builtin_printf("Condition test completed with flag=%d\n", runtime_cond);
}

__attribute__((optimize("O2")))
void test_scan_temporaries(int *arr, int n, volatile int scan_type) {
    int scan_sum = 0;
    int exclusive_prefix = 0;
    
    /* Exclusive scan */
    #pragma omp parallel for reduction(inscan, +:scan_sum) \
                             private(exclusive_prefix)
    for (int i = 0; i < n; i++) {
        /* Exclusive scan phase */
        exclusive_prefix = scan_sum;
        
        #pragma omp scan exclusive(scan_sum)
        scan_sum += arr[i];
        
        /* Store exclusive prefix */
        arr[i] = exclusive_prefix + (i * scan_type);
    }
    
    int inclusive_prefix = 0;
    
    /* Inclusive scan with inscan reduction */
    #pragma omp parallel for reduction(inscan, +:inclusive_prefix)
    for (int i = n/2; i < n; i++) {
        inclusive_prefix += arr[i];
        
        #pragma omp scan inclusive(inclusive_prefix)
        arr[i] = inclusive_prefix;
    }
    
    /* Combined parallel for scan */
    #pragma omp parallel for simd scan(+:scan_sum) \
                             schedule(static, 16)
    for (int i = 0; i < n/4; i++) {
        scan_sum += arr[i] * 2;
        arr[i] = scan_sum;
    }
    
    __builtin_printf("Scan results: sum=%d, prefix=%d\n", 
                     scan_sum, inclusive_prefix);
}

__attribute__((optimize("O2")))
void test_enter_clause(int **dyn_arr, int n, volatile int use_device) {
    /* Dynamically allocate array for enter data */
    *dyn_arr = (int *)malloc(n * sizeof(int));
    if (!*dyn_arr) return;
    
    /* Initialize array */
    for (int i = 0; i < n; i++) {
        (*dyn_arr)[i] = i * i + 1;
    }
    
    /* Use enter data with to clause - triggers OMP_CLAUSE_ENTER with to modifier */
    if (use_device) {
        #pragma omp target enter data map(to: (*dyn_arr)[0:n/2])
        
        /* Perform computation on device */
        #pragma omp target teams distribute parallel for \
                    map(always, tofrom: (*dyn_arr)[0:n/2])
        for (int i = 0; i < n/2; i++) {
            (*dyn_arr)[i] *= 2;
        }
        
        #pragma omp target exit data map(from: (*dyn_arr)[0:n/2])
    } else {
        /* Alternative: enter data with to clause but no device execution */
        #pragma omp target enter data map(to: (*dyn_arr)[0:64])
        
        /* Immediately exit */
        #pragma omp target exit data map(from: (*dyn_arr)[0:64])
    }
    
    __builtin_printf("Enter clause test: first=%d, last=%d\n", 
                     (*dyn_arr)[0], (*dyn_arr)[n-1]);
}

/* Main test orchestrator */
__attribute__((optimize("O2")))
void run_openmp_tests(int *arr1, int *arr2, int n, volatile int iter) {
    volatile int test_flags[4];
    
    /* Set volatile flags based on iteration and random seed */
    for (int i = 0; i < 4; i++) {
        test_flags[i] = (lcg_rand() + iter) % 100;
    }
    
    /* Execute all test functions in sequence */
    test_reduction_temporaries(arr1, arr2, n, test_flags[0]);
    test_condition_temporaries(arr1, n, test_flags[1]);
    test_scan_temporaries(arr2, n, test_flags[2]);
    
    int *dyn_arr = NULL;
    test_enter_clause(&dyn_arr, n, test_flags[3]);
    
    if (dyn_arr) {
        free(dyn_arr);
    }
    
    /* Compute checksum to prevent optimization */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] ^ arr2[i];
    }
    __builtin_printf("Iteration %d checksum: %d\n", iter, checksum);
}

int main(int argc, char **argv) {
    /* Initialize random seed from argv if provided */
    if (argc > 1) {
        lcg_seed = atoi(argv[1]);
    }
    
    const int N = 512;
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    
    if (!array1 || !array2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays with pseudo-random values */
    for (int i = 0; i < N; i++) {
        array1[i] = lcg_rand() % 1000;
        array2[i] = lcg_rand() % 1000;
    }
    
    volatile int num_iterations = 3;
    if (argc > 2) {
        num_iterations = atoi(argv[2]) % 5 + 1;
    }
    
    /* Run tests multiple times with different volatile conditions */
    for (volatile int iter = 0; iter < num_iterations; iter++) {
        run_openmp_tests(array1, array2, N, iter);
        
        /* Modify arrays between iterations for variability */
        for (int i = 0; i < N; i += 7) {
            array1[i] += lcg_rand() % 10;
            array2[i] -= lcg_rand() % 10;
        }
    }
    
    /* Final checksum output */
    long final_sum = 0;
    for (int i = 0; i < N; i++) {
        final_sum += array1[i] + array2[i];
    }
    printf("Final sum: %ld\n", final_sum);
    
    free(array1);
    free(array2);
    
    return 0;
}
