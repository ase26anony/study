#include <omp.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>

// Function 1: Complex reduction patterns
void test_reductions(int N, int* results) {
    int sum_int = 0;
    float sum_float = 0.0f;
    double prod_double = 1.0;
    int array_sum[10] = {0};
    
    // Combined construct with multiple reductions
    #pragma omp parallel for simd reduction(+:sum_int, sum_float) reduction(*:prod_double)
    for (int i = 0; i < N; i++) {
        sum_int += i;
        sum_float += sinf(i * 0.1f);
        prod_double *= (i % 10 + 1) * 0.1;
    }
    
    // Nested reduction with array elements
    #pragma omp parallel
    {
        int local_sums[10] = {0};
        #pragma omp for reduction(+:array_sum[:10])
        for (int i = 0; i < N; i++) {
            int idx = i % 10;
            array_sum[idx] += i;
            local_sums[idx] += i * 2;
        }
        
        // Taskloop reduction
        #pragma omp single
        {
            #pragma omp taskloop reduction(+:sum_int)
            for (int i = 0; i < 100; i++) {
                sum_int += i % 5;
            }
        }
    }
    
    results[0] = sum_int;
    results[1] = (int)sum_float;
    results[2] = (int)prod_double;
}

// Function 2: Scan operations
void test_scans(int N, int* output) {
    int sum = 0;
    
    // SIMD with inscan reduction
    #pragma omp simd reduction(inscan, +:sum)
    for (int i = 0; i < N; i++) {
        int val = i * 2;
        #pragma omp scan inclusive(sum)
        sum += val;
        output[i] = sum;
    }
    
    // Parallel for with scan directive
    #pragma omp parallel for reduction(inscan, +:sum)
    for (int i = 0; i < N; i++) {
        int partial = i * 3;
        #pragma omp scan exclusive(sum)
        int temp = sum;
        sum += partial;
        output[N + i] = temp;
    }
}

// Function 3: Conditional temporaries
void test_conditionals(volatile int cond1, volatile int cond2, int N, int* data) {
    // Non-constant conditions using volatile variables
    #pragma omp parallel if(cond1 > 0) num_threads(4)
    {
        #pragma omp for if(cond2 != 0)
        for (int i = 0; i < N; i++) {
            data[i] += i;
        }
        
        // Nested conditional
        #pragma omp sections if(cond1 + cond2 > 5)
        {
            #pragma omp section
            {
                for (int i = 0; i < N/2; i++) {
                    data[i] *= 2;
                }
            }
            #pragma omp section
            {
                for (int i = N/2; i < N; i++) {
                    data[i] /= 2;
                }
            }
        }
    }
}

// Function 4: Enter data with 'to' mapper
void test_enter_data(int N) {
    int* array1 = new int[N];
    int* array2 = new int[N];
    
    // Initialize arrays
    for (int i = 0; i < N; i++) {
        array1[i] = i;
        array2[i] = i * 2;
    }
    
    // Use enter data with 'to' mapper
    #pragma omp enter data map(to: array1[0:N], array2[0:N])
    
    // Perform computation on device if available
    #pragma omp target teams distribute parallel for map(tofrom: array1[0:N])
    for (int i = 0; i < N; i++) {
        array1[i] += array2[i];
    }
    
    // Exit data
    #pragma omp exit data map(from: array1[0:N])
    
    delete[] array1;
    delete[] array2;
}

// Complex function combining multiple patterns
void test_combined(volatile int cond, int N, int* results) {
    int sum = 0;
    int scan_sum = 0;
    
    // Combined parallel region with reduction, scan, and conditional
    #pragma omp parallel if(cond > 0) reduction(+:sum)
    {
        #pragma omp for simd reduction(inscan, +:scan_sum)
        for (int i = 0; i < N; i++) {
            int val = (i + 1) * (i + 1);
            #pragma omp scan inclusive(scan_sum)
            scan_sum += val;
            sum += val % 100;
        }
        
        // Task reduction inside parallel region
        #pragma omp single
        {
            #pragma omp task reduction(+:sum)
            {
                for (int i = 0; i < 50; i++) {
                    sum += i;
                }
            }
        }
    }
    
    results[0] = sum;
    results[1] = scan_sum;
}
