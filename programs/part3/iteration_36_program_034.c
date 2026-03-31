#include <omp.h>
#include <cstdio>
#include <cmath>

// Function 1: Complex reductions with multiple variables
extern "C" void test_reduction_temporaries(int n, float* results) {
    int sum_int = 0;
    float sum_float = 0.0f;
    double prod_double = 1.0;
    int array_sum[4] = {0, 0, 0, 0};
    
    // Combined parallel for simd with multiple reductions
    #pragma omp parallel for simd reduction(+:sum_int, sum_float) \
                             reduction(*:prod_double) collapse(2)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 4; j++) {
            sum_int += i + j;
            sum_float += sqrtf(i * j + 1.0f);
            if (i * j > 0) {
                prod_double *= 1.0001;
            }
        }
    }
    
    // Nested reduction in taskloop
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp taskloop reduction(+:array_sum) nogroup
            for (int i = 0; i < n; i++) {
                for (int j = 0; j < 4; j++) {
                    array_sum[j] += (i % (j + 1));
                }
            }
        }
    }
    
    results[0] = sum_int;
    results[1] = sum_float;
    results[2] = prod_double;
    results[3] = array_sum[0] + array_sum[1] + array_sum[2] + array_sum[3];
}

// Function 2: SIMD with inscan reductions
extern "C" void test_scan_temporaries(int n, float* output) {
    int sum = 0;
    float prefix_sum = 0.0f;
    
    // SIMD with inscan reduction
    #pragma omp simd reduction(inscan, +:sum, prefix_sum)
    for (int i = 0; i < n; i++) {
        int val = i * i % 100;
        sum += val;
        
        #pragma omp scan inclusive(sum)
        prefix_sum += sum;
        
        output[i] = prefix_sum;
    }
    
    // Another scan pattern with explicit scan directive
    int scan_array[100];
    #pragma omp parallel for
    for (int i = 0; i < 100; i++) {
        scan_array[i] = i;
    }
    
    int running_sum = 0;
    #pragma omp parallel for reduction(inscan, +:running_sum)
    for (int i = 0; i < 100; i++) {
        running_sum += scan_array[i];
        
        #pragma omp scan exclusive(running_sum)
        scan_array[i] = running_sum;
    }
}

// Function 3: Conditional temporaries with volatile conditions
extern "C" int test_conditional_temporaries(volatile int cond1, volatile int cond2, int n) {
    int result = 0;
    
    // Parallel region with volatile condition
    #pragma omp parallel if(cond1 > 0) reduction(+:result)
    {
        #pragma omp for nowait
        for (int i = 0; i < n; i++) {
            result += i;
        }
        
        // Nested conditional
        #pragma omp sections if(cond2 != 0)
        {
            #pragma omp section
            {
                result += 1;
            }
            #pragma omp section
            {
                result += 2;
            }
        }
    }
    
    // Task with conditional
    #pragma omp parallel
    {
        #pragma omp single
        {
            #pragma omp task if(cond1 + cond2 > 5)
            {
                result *= 2;
            }
        }
    }
    
    return result;
}

// Function 4: Enter data with 'to' mapper
extern "C" void test_enter_data(int* data, int n) {
    // Allocate and map data with 'to' clause
    #pragma omp enter data map(to: data[0:n])
    
    // Use the data in parallel region
    #pragma omp target enter data map(to: data[0:n])
    
    #pragma omp target teams distribute parallel for
    for (int i = 0; i < n; i++) {
        data[i] = data[i] * 2 + 1;
    }
    
    #pragma omp target exit data map(from: data[0:n])
    #pragma omp exit data map(release: data)
}
