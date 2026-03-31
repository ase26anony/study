#include <omp.h>
#include <cmath>

// Function 1: Complex reduction patterns
void test_reductions(int n, float* arr, int* results) {
    int sum_int = 0;
    float sum_float = 0.0f;
    double prod_double = 1.0;
    int min_val = INT_MAX;
    int max_val = INT_MIN;
    
    // Combined parallel for simd with multiple reductions
    #pragma omp parallel for simd reduction(+:sum_int, sum_float) \
                             reduction(*:prod_double) \
                             reduction(min:min_val) \
                             reduction(max:max_val) \
                             schedule(static)
    for (int i = 0; i < n; i++) {
        sum_int += i;
        sum_float += arr[i];
        prod_double *= (1.0 + arr[i] / 1000.0);
        if (i < min_val) min_val = i;
        if (i > max_val) max_val = i;
    }
    
    // Nested reduction in taskloop
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskloop reduction(+:sum_int) nogroup
            for (int i = 0; i < n; i++) {
                sum_int += (int)arr[i];
            }
        }
    }
    
    results[0] = sum_int;
    results[1] = (int)sum_float;
    results[2] = (int)prod_double;
    results[3] = min_val;
    results[4] = max_val;
}

// Function 2: Scan operations with inscan reductions
void test_scans(int n, float* input, float* output) {
    float prefix_sum = 0.0f;
    
    // SIMD with inscan reduction
    #pragma omp simd reduction(inscan, +:prefix_sum) simdlen(8)
    for (int i = 0; i < n; i++) {
        prefix_sum += input[i];
        #pragma omp scan exclusive(prefix_sum)
        output[i] = prefix_sum;
    }
    
    // Parallel for with scan directive
    #pragma omp parallel for reduction(inscan, +:prefix_sum)
    for (int i = 0; i < n; i++) {
        float val = input[i] * 2.0f;
        #pragma omp scan exclusive(prefix_sum)
        output[i] += prefix_sum;
        prefix_sum += val;
    }
}

// Function 3: Conditional temporaries with volatile conditions
void test_conditionals(int n, volatile int cond1, volatile int cond2, int* data) {
    // Non-constant conditions forcing temporary clause generation
    #pragma omp parallel if(cond1 > 0) num_threads(4)
    {
        #pragma omp for if(cond2 < 100) schedule(dynamic)
        for (int i = 0; i < n; i++) {
            data[i] *= 2;
        }
        
        // Nested conditional
        #pragma omp sections if(cond1 + cond2 > 50)
        {
            #pragma omp section
            {
                for (int i = 0; i < n/2; i++) {
                    data[i] += 1;
                }
            }
            #pragma omp section
            {
                for (int i = n/2; i < n; i++) {
                    data[i] -= 1;
                }
            }
        }
    }
}

// Function 4: Enter data with 'to' mapper
void test_enter_data(int n, float* host_array, float* device_array) {
    // Create device array
    #pragma omp target enter data map(to: device_array[0:n])
    
    // Also test with structured block
    #pragma omp target data map(to: host_array[0:n])
    {
        #pragma omp target teams distribute parallel for
        for (int i = 0; i < n; i++) {
            device_array[i] = host_array[i] * 2.0f;
        }
    }
    
    // Multiple enter directives
    int* temp_buffer = new int[n];
    #pragma omp target enter data map(to: temp_buffer[0:n])
    
    #pragma omp target
    {
        for (int i = 0; i < n; i++) {
            temp_buffer[i] = i;
        }
    }
    
    #pragma omp target exit data map(from: temp_buffer[0:n])
    delete[] temp_buffer;
}

// Function 5: Combined complex patterns
void test_combined(int n, volatile int cond, float* arr, int* results) {
    float scan_temp = 0.0f;
    int reduction_temp = 0;
    
    // Combined parallel with if clause and reduction
    #pragma omp parallel if(cond > 10) reduction(+:reduction_temp)
    {
        #pragma omp for simd reduction(inscan, +:scan_temp)
        for (int i = 0; i < n; i++) {
            scan_temp += arr[i];
            #pragma omp scan exclusive(scan_temp)
            arr[i] = scan_temp;
            reduction_temp += (int)arr[i];
        }
    }
    
    results[0] = reduction_temp;
    results[1] = (int)scan_temp;
}
