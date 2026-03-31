/* test_ddg.c - Complex loop to trigger DDG edge creation */

#include <math.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex loop with multiple dependency types */
void process_data(double* restrict dest, 
                  const double* src1, 
                  const double* src2,
                  double* shared,  /* Non-restrict for aliasing */
                  int size, 
                  int offset) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double temp_reg = 0.0;
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < size; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency */
        double val = src1[i] * src2[i];      /* RAW: src1/src2 -> val */
        acc = acc + val;                     /* RAW: acc -> acc (loop-carried) */
        
        /* 2. ANTI-DEPENDENCY (WAR) with memory aliasing */
        double old_val = shared[i-1];        /* Read shared[i-1] */
        shared[i] = acc * old_val;           /* Write shared[i] - WAR with next iteration */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        if (i % 16 == 0) {
            acc = external_func(acc);        /* WAW: acc overwritten, function call latency */
        }
        
        /* 4. Loop-carried dependency with distance > 1 */
        dest[i] = dest[i-2] + src1[i];       /* Distance=2 RAW */
        
        /* 5. Complex addressing for alias analysis */
        int idx = i + offset + (int)src1[i%8];
        if (idx < size && idx > 0) {
            shared[idx] = shared[idx] * 0.5; /* Self-dependency + possible WAR/WAW */
        }
        
        /* 6. Control flow creating control dependencies */
        if (src1[i] > 0.5) {
            temp_reg = sqrt(src1[i]);        /* High latency sqrt */
            dest[i] += temp_reg;
        } else {
            temp_reg = 1.0 / src1[i];        /* High latency division */
            dest[i] -= temp_reg;
        }
        
        /* 7. Another loop-carried dependency */
        acc = acc + dest[i] * 0.1;           /* RAW: acc -> acc, dest[i] -> acc */
    }
    
    /* Volatile sink to prevent dead code elimination */
    sink = acc + dest[size/2] + shared[size/4];
}

/* Helper to initialize data */
void init_arrays(double* arr1, double* arr2, int size) {
    for (int i = 0; i < size; ++i) {
        arr1[i] = (i % 7) * 0.3;
        arr2[i] = (i % 5) * 0.2;
    }
}

int main() {
    const int SIZE = 512;
    double src1[SIZE], src2[SIZE];
    double dest[SIZE];
    double shared[SIZE];  /* Aliased memory */
    
    /* Initialize with pattern */
    init_arrays(src1, src2, SIZE);
    
    /* Clear destination */
    for (int i = 0; i < SIZE; ++i) {
        dest[i] = 0.0;
        shared[i] = 1.0;
    }
    
    /* Set up initial values for loop-carried dependencies */
    dest[0] = 1.0;
    dest[1] = 2.0;
    shared[0] = 0.5;
    
    /* Process with complex dependencies */
    process_data(dest, src1, src2, shared, SIZE, 3);
    
    /* Use results to prevent optimization */
    volatile double result = 0.0;
    for (int i = 0; i < SIZE; i += 32) {
        result += dest[i] + shared[i];
    }
    
    return (int)result % 256;
}
