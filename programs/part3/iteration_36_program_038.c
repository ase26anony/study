#include <omp.h>
#include <cstdio>
#include <cmath>

// Function 1: Complex reduction patterns
void test_reductions(int N, float* data, int* results) {
    int sum_int = 0;
    float sum_float = 0.0f;
    double sum_double = 0.0;
    int prod_int = 1;
    
    // Combined parallel for simd with multiple reductions
    #pragma omp parallel for simd reduction(+:sum_int, sum_float, sum_double) \
                             reduction(*:prod_int) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 10; j++) {
            sum_int += i + j;
            sum_float += data[i] * j;
            sum_double += sqrt(i + j);
            if (i > 0 && j > 0) {
                prod_int *= (i % 10) + 1;
            }
        }
    }
    
    // Taskloop reduction
    #pragma omp parallel
    #pragma omp single
    {
        #pragma omp taskloop reduction(+:sum_int)
        for (int i = 0; i < N; i++) {
            sum_int += i * 2;
        }
    }
    
    results[0] = sum_int;
    results[1] = (int)sum_float;
    results[2] = (int)sum_double;
    results[3] = prod_int;
}

// Function 2: Scan operations with inscan reductions
void test_scans(int N, int* array) {
    int prefix_sum = 0;
    
    // SIMD with inscan reduction
    #pragma omp simd reduction(inscan, +:prefix_sum)
    for (int i = 0; i < N; i++) {
        int val = array[i];
        #pragma omp scan inclusive(prefix_sum)
        prefix_sum += val;
        array[i] = prefix_sum;
    }
    
    // Parallel for with scan directive
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < N; i++) {
        int temp = array[i] * 2;
        #pragma omp scan exclusive(prefix_sum)
        array[i] = prefix_sum;
        prefix_sum += temp;
    }
}

// Function 3: Conditional temporaries with volatile conditions
void test_conditionals(int N, volatile int cond_var, int* data) {
    // Use volatile variable to prevent constant folding
    #pragma omp parallel if(cond_var > 0) num_threads(4)
    {
        #pragma omp for
        for (int i = 0; i < N; i++) {
            data[i] += omp_get_thread_num();
        }
    }
    
    // Nested conditionals
    #pragma omp parallel if(cond_var < 100) if(cond_var > 10)
    {
        #pragma omp for
        for (int i = 0; i < N; i++) {
            data[i] *= 2;
        }
    }
}

// Function 4: Enter data with 'to' mapper
void test_enter_data(int N, float* host_array, float* device_array) {
    // Initialize host array
    for (int i = 0; i < N; i++) {
        host_array[i] = i * 1.5f;
    }
    
    // Use enter data with 'to' mapper
    #pragma omp enter data map(to: host_array[0:N])
    
    // Also test with structured reference
    #pragma omp enter data map(to: device_array[0:N/2])
    
    // Perform computation on device
    #pragma omp target teams distribute parallel for map(tofrom: host_array[0:N])
    for (int i = 0; i < N; i++) {
        host_array[i] = sin(host_array[i]);
    }
    
    #pragma omp exit data map(from: host_array[0:N])
}

// Function 5: Mixed complex patterns
void test_mixed_patterns(int N, volatile int cond, int* results) {
    int sum = 0;
    int scan_sum = 0;
    
    // Parallel region with if clause and reduction
    #pragma omp parallel if(cond != 0) reduction(+:sum)
    {
        #pragma omp for simd reduction(inscan, +:scan_sum)
        for (int i = 0; i < N; i++) {
            int val = i * (omp_get_thread_num() + 1);
            #pragma omp scan inclusive(scan_sum)
            scan_sum += val;
            sum += val;
        }
    }
    
    results[0] = sum;
    results[1] = scan_sum;
}
