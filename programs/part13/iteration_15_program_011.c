/* test_ddg.c - Complex loop with multiple dependency types for DDG coverage */

#include <math.h>
#include <stdlib.h>

/* Non-inlineable function to force latency modeling */
static double __attribute__((noinline)) 
external_compute(double a, double b) {
    return a * b + (a - b);
}

/* Complex loop with all required dependency types */
void process_loop(double* restrict dest, 
                  const double* src1, 
                  const double* src2,
                  double* aliased,  /* Non-restrict for aliasing */
                  int n, 
                  int offset) {
    double acc = 1.0;
    double tmp_array[8];
    
    /* Initialize tmp_array with values */
    for (int j = 0; j < 8; j++) {
        tmp_array[j] = (double)j * 0.5;
    }
    
    /* Main target loop with complex dependencies */
    for (int i = 1; i < n; i++) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency */
        double val1 = src1[i] * 3.14159;
        double val2 = external_compute(val1, src2[i]);  /* Function call latency */
        
        /* 2. LOOP-CARRIED DEPENDENCY with distance > 0 */
        acc = acc / (val2 + 0.001);  /* Floating division - variable latency */
        
        /* 3. ANTI-DEPENDENCY (WAR) via aliased array */
        double read_before_write = aliased[i-1];  /* Read old value */
        aliased[i] = acc + read_before_write;     /* Write new value */
        
        /* 4. OUTPUT DEPENDENCY (WAW) on accumulator */
        if (i % 16 == 0) {
            acc = sqrt(fabs(acc));  /* Overwrites acc - output dependency */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + offset + (int)(src1[i] * 0.1);
        idx = idx % (n - 1);
        if (idx < 0) idx = 0;
        
        /* 6. MEMORY DEPENDENCY with overlapping accesses */
        dest[idx] = dest[i] * 0.8 + aliased[idx];
        
        /* 7. CONTROL FLOW inside loop */
        double conditional_result;
        if (acc > 100.0) {
            conditional_result = acc * 0.5;
        } else if (acc < -100.0) {
            conditional_result = acc * 2.0;
        } else {
            /* Integer division with variable divisor */
            int divisor = (i % 7) + 1;
            conditional_result = (double)(i / divisor);  /* Variable latency */
        }
        
        /* 8. ANOTHER LOOP-CARRIED with different distance */
        tmp_array[i % 8] = tmp_array[(i-1) % 8] + conditional_result;
        
        /* 9. MIXED DEPENDENCIES in single expression */
        dest[i] = (aliased[i] * tmp_array[i % 8]) / (src1[i] + 1.0);
    }
    
    /* Prevent dead code elimination */
    volatile double sink = acc + dest[n/2] + aliased[0];
    (void)sink;
}

/* Additional complexity with pointer parameters */
void process_with_pointers(double* a, double* b, double* c, int n) {
    double local_acc = 0.0;
    
    for (int i = 2; i < n; i++) {
        /* Pointer-based dependencies */
        double* ptr1 = &a[i];
        double* ptr2 = &b[i-1];
        double* ptr3 = &c[i-2];
        
        /* Chain of dependencies */
        *ptr1 = *ptr2 * *ptr3 + local_acc;
        local_acc = *ptr1 / (double)(i % 5 + 1);  /* Integer division latency */
        
        /* Cross-iteration store */
        b[i] = a[i-1] + c[i-1];
        
        /* Conditional with side effects */
        if (local_acc > 1000.0) {
            c[i] = sqrt(local_acc);  /* High latency */
            local_acc = 0.0;  /* Output dependency */
        }
    }
}

/* Main driver with realistic data patterns */
int main() {
    const int N = 1024;
    
    /* Allocate and initialize arrays */
    double* src1 = (double*)malloc(N * sizeof(double));
    double* src2 = (double*)malloc(N * sizeof(double));
    double* dest = (double*)malloc(N * sizeof(double));
    double* aliased = (double*)malloc(N * sizeof(double));
    
    /* Initialize with pattern */
    for (int i = 0; i < N; i++) {
        src1[i] = sin(i * 0.1);
        src2[i] = cos(i * 0.05);
        dest[i] = (double)i * 0.01;
        aliased[i] = (double)(i % 100) * 0.1;
    }
    
    /* Call the complex loop multiple times with different offsets */
    for (int iter = 0; iter < 3; iter++) {
        int offset = iter * 10;
        process_loop(dest, src1, src2, aliased, N, offset);
        process_with_pointers(src1, src2, dest, N);
    }
    
    /* Use results to prevent optimization */
    volatile double result = 0.0;
    for (int i = 0; i < N; i += 64) {
        result += dest[i] + aliased[i];
    }
    
    /* Cleanup */
    free(src1);
    free(src2);
    free(dest);
    free(aliased);
    
    return (int)(result * 0.001);
}
