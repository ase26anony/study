#include <omp.h>
#include <cmath>

// Function 1: Complex reduction patterns
void test_reductions(int N, float* data, int* results) {
    int sum_int = 0;
    float sum_float = 0.0f;
    double prod_double = 1.0;
    int array_reduce[4] = {0, 0, 0, 0};
    
    // Combined construct with multiple reductions
    #pragma omp parallel for simd reduction(+:sum_int, sum_float) \
                             reduction(*:prod_double) collapse(2)
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < 4; j++) {
            sum_int += i + j;
            sum_float += data[i] * j;
            if (i > 0) prod_double *= 1.0001;
        }
    }
    
    // Nested reduction with taskloop
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskloop reduction(+:array_reduce[:4]) nogroup
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < 4; j++) {
                    array_reduce[j] += (int)(data[i] * 100);
                }
            }
        }
    }
    
    results[0] = sum_int;
    results[1] = (int)sum_float;
    results[2] = (int)prod_double;
    for (int i = 0; i < 4; i++) {
        results[3 + i] = array_reduce[i];
    }
}

// Function 2: Scan operations with inscan reductions
void test_scans(int N, float* input, float* output) {
    float prefix_sum = 0.0f;
    float max_val = -1e9;
    
    // SIMD with inscan reduction
    #pragma omp simd reduction(inscan, +:prefix_sum) \
                       reduction(max:max_val)
    for (int i = 0; i < N; i++) {
        // Exclusive scan
        #pragma omp scan exclusive(prefix_sum)
        output[i] = prefix_sum;
        prefix_sum += input[i];
        max_val = fmaxf(max_val, input[i]);
    }
    
    // Another scan in parallel for
    float sum2 = 0.0f;
    #pragma omp parallel for reduction(inscan, +:sum2)
    for (int i = 0; i < N; i++) {
        float val = input[i] * 2.0f;
        #pragma omp scan inclusive(sum2)
        output[i] += sum2;
        sum2 += val;
    }
}

// Function 3: Conditional temporaries with volatile conditions
void test_conditionals(int N, volatile int cond_var, int* data, int* result) {
    int local_sum = 0;
    
    // Multiple if clauses with volatile conditions
    #pragma omp parallel if(cond_var > 0) reduction(+:local_sum) \
                        if(cond_var < N)
    {
        #pragma omp for
        for (int i = 0; i < N; i++) {
            local_sum += data[i];
        }
    }
    
    // Nested conditional
    #pragma omp parallel if(cond_var != 0)
    {
        #pragma omp for if(cond_var % 2 == 0)
        for (int i = 0; i < N; i++) {
            data[i] *= 2;
        }
    }
    
    *result = local_sum;
}

// Function 4: Enter data with to mapper
void test_enter_data(int N, float* host_array, float* device_array) {
    // Create data on host
    for (int i = 0; i < N; i++) {
        host_array[i] = i * 1.5f;
    }
    
    // Use enter data with to clause
    #pragma omp enter data map(to: host_array[0:N]) \
                           map(to: device_array[0:N/2])
    
    // Also test with array section
    #pragma omp enter data map(to: host_array[N/4:3*N/4])
    
    // Cleanup
    #pragma omp exit data map(delete: host_array[0:N])
}
