/* ddg_test.c - Complex loop to trigger DDG edge creation */

#include <math.h>
#include <stdio.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex loop with multiple dependency types */
void process_data(double* restrict dest, 
                  const double* src1, 
                  const double* src2,
                  double* shared,  /* Non-restrict pointer for aliasing */
                  int size,
                  int offset) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double temp_reg = 0.0;
    
    /* Loop with complex dependencies */
    for (int i = 1; i < size; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency */
        double coeff = external_func(src1[i]); /* Function call latency */
        double product = src2[i] * coeff;      /* Depends on coeff */
        
        /* 2. LOOP-CARRIED DEPENDENCY (distance > 0) */
        acc = acc / 2.5 + product;  /* Division latency + loop-carried */
        
        /* 3. ANTI-DEPENDENCY (WAR) via aliased memory */
        double prev_val = dest[i-1];          /* Read dest[i-1] */
        dest[i] = acc + prev_val;             /* Write dest[i] - WAR with next iteration */
        
        /* 4. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 16 == 0) {
            acc = sqrt(acc);  /* High latency sqrt + WAW on acc */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + offset + (int)src1[i % 8];
        if (idx < size && idx >= 0) {
            shared[idx] = dest[i] * 0.5;  /* May alias with dest */
        }
        
        /* 6. MULTIPLE WRITES to same location (WAW) */
        temp_reg = src1[i] + src2[i];
        if (i % 4 == 0) {
            temp_reg = external_func(temp_reg);  /* WAW on temp_reg */
        }
        
        /* 7. CONTROL DEPENDENCY */
        double conditional_result = (i % 3 == 0) ? 
                                   acc * 2.0 : acc * 0.5;
        
        /* 8. ANOTHER LOOP-CARRIED with memory */
        shared[i % 32] = shared[i % 32] + conditional_result;
    }
    
    /* Volatile sink to prevent dead code elimination */
    sink = acc + dest[size/2] + shared[0];
}

/* Helper to initialize arrays */
void init_arrays(double* arr1, double* arr2, int size) {
    for (int i = 0; i < size; ++i) {
        arr1[i] = (i * 1.5) / (i + 1.0);
        arr2[i] = sin(i * 0.1) + 1.0;
    }
}

int main() {
    const int SIZE = 1024;
    double src1[SIZE], src2[SIZE];
    double dest[SIZE] = {0};
    double shared[SIZE] = {0};
    
    /* Initialize with non-trivial patterns */
    init_arrays(src1, src2, SIZE);
    
    /* Initial value for loop-carried dependency */
    dest[0] = 1.0;
    shared[0] = 0.5;
    
    /* Process with complex dependencies */
    process_data(dest, src1, src2, shared, SIZE, 3);
    
    /* Use results to prevent optimization */
    volatile double result = dest[SIZE-1] + shared[SIZE-1];
    printf("Result: %f\n", result);
    
    return 0;
}
