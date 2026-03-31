/* Test program to trigger uncovered OpenMP clause printing in GCC */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

/* Function attribute to ensure optimization and tree dumping */
__attribute__((optimize("O2"), noinline))
void test_reduction_temporaries(int n, volatile int* data, volatile int* result) {
    int i;
    int sum = 0;
    int prod = 1;
    int max_val = data[0];
    int min_val = data[0];
    
    /* Complex reduction with multiple operators on volatile data
       Forces creation of _reductemp_ temporaries */
    #pragma omp parallel for reduction(+:sum) reduction(*:prod) \
            reduction(max:max_val) reduction(min:min_val) \
            private(i) shared(data)
    for (i = 0; i < n; i++) {
        /* Data-dependent computation to prevent optimization */
        int idx = (i * 17) % n;
        sum += data[idx] + (i % 3);
        prod *= (data[idx] % 10) + 1;
        if (data[idx] > max_val) max_val = data[idx];
        if (data[idx] < min_val) min_val = data[idx];
    }
    
    result[0] = sum;
    result[1] = prod % 1000;
    result[2] = max_val;
    result[3] = min_val;
    
    __builtin_printf("Reduction results: %d %d %d %d\n", 
                     sum, prod % 1000, max_val, min_val);
}

__attribute__((optimize("O2"), noinline))
void test_condition_temporaries(int n, volatile int flag1, volatile int flag2, 
                               volatile int* data) {
    volatile int runtime_flag = flag1 > flag2;
    
    /* OMP parallel with if clause - may generate _condtemp_ */
    #pragma omp parallel if(runtime_flag) num_threads(4)
    {
        int tid = omp_get_thread_num();
        
        /* Task with if clause - another context for _condtemp_ */
        #pragma omp task if(tid % 2 == 0) firstprivate(tid)
        {
            data[tid % n] += tid;
        }
        
        #pragma omp taskwait
        
        /* Target teams with if clause - different clause context */
        #ifdef _OPENMP
        #if _OPENMP >= 201511
        #pragma omp target teams if(runtime_flag && tid == 0) \
                map(tofrom: data[0:n]) num_teams(1) thread_limit(4)
        {
            #pragma omp distribute parallel for
            for (int i = 0; i < n; i++) {
                data[i] += i;
            }
        }
        #endif
        #endif
    }
    
    __builtin_printf("Condition test completed, flag=%d\n", runtime_flag);
}

__attribute__((optimize("O2"), noinline))
void test_scan_temporaries(int n, volatile int* arr) {
    int i;
    
    /* Exclusive scan - should generate _scantemp_ */
    #pragma omp parallel for simd scan(exclusive:+:arr[0:n])
    for (i = 0; i < n; i++) {
        if (i > 0) arr[i] += arr[i-1];
    }
    
    /* Inclusive scan with inscan directive */
    int scan_temp = 0;
    #pragma omp parallel for reduction(inscan,+:scan_temp)
    for (i = 0; i < n; i++) {
        // Exclusive scan phase
        int val = arr[i];
        #pragma omp scan exclusive(scan_temp)
        arr[i] = scan_temp;
        scan_temp += val;
    }
    
    __builtin_printf("Scan test completed, last element: %d\n", arr[n-1]);
}

__attribute__((optimize("O2"), noinline))
void test_enter_clause(int n) {
    /* Allocate and initialize data for enter clause */
    int* device_data = (int*)malloc(n * sizeof(int));
    for (int i = 0; i < n; i++) {
        device_data[i] = i * 2;
    }
    
    /* OMP enter data with to modifier - triggers OMP_CLAUSE_ENTER with to */
    #pragma omp target enter data map(to: device_data[0:n])
    
    /* Use the data in target region */
    #pragma omp target teams distribute parallel for \
            map(alloc: device_data[0:n])
    for (int i = 0; i < n; i++) {
        device_data[i] += 1;
    }
    
    /* Exit data */
    #pragma omp target exit data map(from: device_data[0:n])
    
    __builtin_printf("Enter clause test, sum: %d\n", 
                     device_data[n/2] + device_data[n-1]);
    
    free(device_data);
}

/* Main test driver with volatile controls */
__attribute__((optimize("O2")))
int main(int argc, char** argv) {
    const int N = 512;
    volatile int* data = (volatile int*)malloc(N * sizeof(int));
    volatile int* results = (volatile int*)malloc(4 * sizeof(int));
    
    /* Seed from command line for runtime variability */
    int seed = 42;
    if (argc > 1) seed = atoi(argv[1]);
    
    /* Initialize with pseudo-random values */
    for (int i = 0; i < N; i++) {
        data[i] = (i * seed + 12345) % 1000;
    }
    
    /* Volatile flags to control execution */
    volatile int flag1 = seed % 3;
    volatile int flag2 = seed % 5;
    volatile int iterations = 2 + (seed % 2);
    
    int total_checksum = 0;
    
    /* Multiple iterations to ensure code paths are taken */
    for (volatile int iter = 0; iter < iterations; iter++) {
        __builtin_printf("\n=== Iteration %d ===\n", iter);
        
        /* 1. Test reduction temporaries */
        test_reduction_temporaries(N, data, results);
        for (int i = 0; i < 4; i++) total_checksum += results[i];
        
        /* 2. Test condition temporaries */
        test_condition_temporaries(N, flag1 + iter, flag2 + iter, data);
        for (int i = 0; i < N; i += 32) total_checksum += data[i];
        
        /* 3. Test scan temporaries */
        test_scan_temporaries(N, data);
        for (int i = 0; i < N; i += 64) total_checksum += data[i];
        
        /* 4. Test enter clause with to modifier */
        test_enter_clause(N/2);
        total_checksum += iter * 1000;
    }
    
    /* Final checksum output to prevent optimization */
    __builtin_printf("\nFinal checksum: %d\n", total_checksum % 1000000);
    
    free((void*)data);
    free((void*)results);
    
    return total_checksum % 256;
}
