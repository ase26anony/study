/* test_ddg.c - Complex loop to trigger DDG edge creation in GCC scheduler */

#include <math.h>
#include <stdlib.h>

/* Non-inlineable function to force latency modeling */
static double __attribute__((noinline)) 
external_func(double x, double y) {
    return x * y + 1.0;
}

/* Complex loop with multiple dependency types */
void process_loop(double* restrict dest, 
                  const double* src1, 
                  const double* src2,
                  double* aliased_array,
                  int size,
                  int offset) {
    volatile double sink; /* Prevent optimizations */
    double acc = 0.0;
    double temp_reg = 0.0;
    
    /* Loop with multiple dependency patterns */
    for (int i = 1; i < size; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency */
        double val1 = src1[i] / src2[i];          /* Floating division */
        acc = acc + val1 * 2.5;                   /* Loop-carried RAW */
        
        /* 2. ANTI-DEPENDENCY (WAR) with memory aliasing */
        double read_before_write = aliased_array[i-1];  /* Read */
        aliased_array[i] = acc + read_before_write;     /* Write to same location */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        if (i % 16 == 0) {
            acc = external_func(acc, src1[i]);    /* WAW on acc */
        }
        
        /* 4. More loop-carried dependencies with variable addressing */
        int idx = i + (offset % 4);
        if (idx < size) {
            dest[idx] = dest[i-1] + src2[i];      /* Another loop-carried RAW */
        }
        
        /* 5. Control flow creating control dependencies */
        if (src1[i] > 0.5) {
            temp_reg = sqrt(src2[i]);             /* High latency sqrt */
            dest[i] = temp_reg + 1.0;
        } else {
            temp_reg = src2[i] * 0.5;             /* Different computation */
            dest[i] = temp_reg - 1.0;
        }
        
        /* 6. Complex addressing inhibiting alias analysis */
        double* ptr = aliased_array + (i % 8);
        *ptr = *ptr + acc;                        /* Pointer-based WAR */
        
        /* 7. Integer division (variable latency) */
        int int_val = (int)src1[i];
        if (int_val != 0) {
            int divisor = (int)src2[i] + 1;
            offset = offset / divisor;            /* Integer division */
        }
    }
    
    /* Prevent dead code elimination */
    sink = acc + dest[10] + aliased_array[5];
}

/* Helper to initialize arrays */
void init_arrays(double* arr1, double* arr2, int size) {
    for (int i = 0; i < size; ++i) {
        arr1[i] = (i % 100) * 0.01;
        arr2[i] = ((i + 7) % 50) * 0.02;
    }
}

int main() {
    const int SIZE = 1024;
    double* src1 = malloc(SIZE * sizeof(double));
    double* src2 = malloc(SIZE * sizeof(double));
    double* dest = malloc(SIZE * sizeof(double));
    double* aliased = malloc(SIZE * sizeof(double));
    
    /* Initialize with deterministic values */
    init_arrays(src1, src2, SIZE);
    init_arrays(aliased, dest, SIZE);  /* Partial overlap */
    
    /* Run the complex loop multiple times */
    for (int iter = 0; iter < 3; ++iter) {
        int offset = iter * 3;
        process_loop(dest, src1, src2, aliased, SIZE, offset);
    }
    
    /* Use results to prevent optimization */
    volatile double final_sink = dest[SIZE-1] + aliased[0];
    
    free(src1);
    free(src2);
    free(dest);
    free(aliased);
    
    return (int)final_sink % 256;
}
