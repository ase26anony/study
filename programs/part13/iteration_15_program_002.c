/* Complex loop to trigger DDG edge creation in GCC scheduler */
#include <math.h>

/* Non-inlineable function to create variable latency */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Global arrays to prevent optimizations */
double global_src[2048];
double global_dest[2048];
double global_coeff[2048];

/* Function with complex loop to build detailed DDG */
void process_loop(int start, int end, int offset, double* restrict result) {
    double acc = 0.0;
    double temp_acc = 0.0;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 2048; i++) {
        global_src[i] = (i % 3 == 0) ? 1.5 : 2.0;
        global_coeff[i] = (i % 5 == 0) ? 0.5 : 1.0;
    }
    
    /* Main target loop with complex dependencies */
    for (int i = start; i < end; i++) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val = global_src[i] * global_coeff[i];
        acc = sqrt(acc + val);  /* sqrt() has variable latency */
        
        /* 2. ANTI-DEPENDENCY (WAR) - read then write same location */
        double prev = global_dest[i-1];  /* Read */
        if (i > 0) {
            /* Complex addressing to inhibit alias analysis */
            int idx = i + (offset % 4);
            if (idx < 2048) {
                global_dest[idx] = acc + prev;  /* Write after read */
            }
        }
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same variable */
        if (i % 8 == 0) {
            acc = external_func(acc);  /* Function call with latency */
        }
        
        /* 4. LOOP-CARRIED DEPENDENCY with distance > 0 */
        if (i >= 2) {
            /* Recurrence: current depends on value from 2 iterations ago */
            global_dest[i] = global_dest[i-2] * 0.9 + acc;
        }
        
        /* 5. CONTROL DEPENDENCY with conditional */
        double conditional_result;
        if (global_src[i] > 1.0) {
            /* Integer division with variable divisor (variable latency) */
            int divisor = (i % 7) + 1;
            conditional_result = (double)(i / divisor);
        } else {
            conditional_result = acc * 0.5;
        }
        
        /* 6. More anti-dependencies with pointer aliasing */
        double* ptr1 = &global_dest[i];
        double* ptr2 = &global_dest[i + (offset % 3)];
        *ptr1 = *ptr2 + conditional_result;  /* WAR: read ptr2, write ptr1 */
        
        /* 7. Another output dependency */
        temp_acc = acc;  /* WAW in next iteration */
        
        /* 8. Complex addressing that might alias */
        int complex_idx = (i * 13 + offset) % 2048;
        if (complex_idx != i) {
            global_src[complex_idx] = global_dest[i] * 0.8;  /* Potential WAR */
        }
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile double sink = acc + global_dest[10] + temp_acc;
    *result = sink;
}

/* Second function with different dependency pattern */
void process_loop2(int n, double* arr1, double* arr2, double* arr3) {
    double sum1 = 0.0, sum2 = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Loop-carried true dependency */
        sum1 = sum1 / (arr1[i] + 1.0);  /* Division has variable latency */
        
        /* Anti-dependency chain */
        double old_val = arr2[i-1];
        arr2[i] = sum1 + old_val;
        
        /* Output dependency */
        if (i % 16 == 0) {
            sum1 = 0.0;
        }
        
        /* Control dependency affecting memory accesses */
        if (sum1 > 100.0) {
            arr3[i] = arr1[i] * 2.0;
        } else {
            arr3[i] = arr2[i] / 3.0;  /* Another division */
        }
        
        /* Cross-iteration dependency with distance 3 */
        if (i >= 3) {
            arr1[i] = arr3[i-3] + sum1;
        }
    }
    
    volatile double sink2 = sum1 + arr2[n-1];
}

int main() {
    double result1, result2;
    double arr1[1024], arr2[1024], arr3[1024];
    
    /* Initialize arrays */
    for (int i = 0; i < 1024; i++) {
        arr1[i] = i * 0.1;
        arr2[i] = i * 0.2;
    }
    
    /* Call first loop processor */
    process_loop(1, 1000, 3, &result1);
    
    /* Call second loop processor */
    process_loop2(500, arr1, arr2, arr3);
    
    /* Use results to prevent optimization */
    volatile double final_sink = result1 + arr3[100];
    
    return (int)(final_sink * 0.0);  /* Return 0 but prevent optimization */
}
