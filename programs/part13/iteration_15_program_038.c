/* ddg_test.c - Complex loop to trigger DDG edge creation */
#include <math.h>

/* Non-inlineable function to create function call latency */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Global arrays to prevent optimizations */
double global_src[1024];
double global_dest[1024];
double global_coeff[1024];
double global_temp[1024];

/* Function with complex loop to build DDG */
void process_loop(int start, int end, int offset, double* restrict result) {
    double acc = 0.0;
    double local_acc = 1.0;
    int idx_mod;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        global_src[i] = i * 0.1;
        global_coeff[i] = sin(i * 0.01) + 1.0;
        global_temp[i] = i * 0.05;
    }
    
    /* Main loop with complex dependencies */
    for (int i = start; i < end; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val = global_src[i] * global_coeff[i];  /* Read src, coeff */
        acc = sqrt(acc + val);  /* Loop-carried: sqrt has variable latency */
        
        /* 2. ANTI-DEPENDENCY (WAR) with memory aliasing */
        double prev = global_dest[i-1];  /* Read before write */
        global_dest[i] = acc + prev;     /* Write after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        if (i % 8 == 0) {
            acc = external_func(local_acc);  /* Function call with latency */
            local_acc = acc * 0.9;           /* Chain of dependencies */
        }
        
        /* 4. COMPLEX ADDRESSING with potential aliasing */
        idx_mod = i % 4;
        /* No restrict - compiler must assume aliasing */
        global_dest[i + (int)global_temp[idx_mod]] = global_src[i];
        
        /* 5. INTEGER DIVISION with variable latency */
        if (global_coeff[i] > 1.5) {
            int divisor = (int)global_src[i] % 16 + 1;
            int quotient = i / divisor;  /* Variable latency integer division */
            global_temp[i % 256] = quotient * 0.01;
        }
        
        /* 6. CONTROL FLOW creating control dependencies */
        double conditional_result;
        if (acc > 10.0) {
            conditional_result = acc * 0.5;
        } else {
            conditional_result = acc * 2.0;
        }
        
        /* 7. ANOTHER LOOP-CARRIED DEPENDENCY with different distance */
        if (i >= 2) {
            global_dest[i] += global_dest[i-2] * 0.3;  /* Distance 2 */
        }
        
        /* 8. MIXED TYPES to prevent vectorization */
        float float_acc = (float)acc;
        double double_acc = (double)float_acc;
        acc = double_acc * 0.99;
    }
    
    /* Volatile sink to prevent dead code elimination */
    volatile double sink = acc + global_dest[10] + global_dest[end-1];
    *result = sink;
}

/* Second loop with pointer aliasing */
void aliasing_loop(double* a, double* b, double* c, int n) {
    /* a, b, c may alias - no restrict keyword */
    for (int i = 1; i < n; ++i) {
        /* Complex pointer arithmetic */
        double* ptr_a = a + i;
        double* ptr_b = b + (i % 32);
        double* ptr_c = c + (i % 64);
        
        /* True dependency through pointers */
        double val_a = *ptr_a;
        double val_b = *ptr_b;
        
        /* Anti-dependency: read then write same location through ptr_c */
        double old_c = *ptr_c;
        *ptr_c = val_a * val_b + old_c;
        
        /* Output dependency: multiple writes to same location */
        if (val_a > val_b) {
            *ptr_a = val_b;
        } else {
            *ptr_a = val_a;
        }
        
        /* Loop-carried through array */
        b[i] = b[i-1] * 0.95 + a[i];
    }
}

int main() {
    double result1, result2;
    
    /* Process with different parameters to create varied DDGs */
    process_loop(1, 512, 3, &result1);
    
    /* Setup for second loop */
    double array1[256];
    double array2[256];
    double array3[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 0.25;
        array2[i] = i * 0.33;
        array3[i] = i * 0.5;
    }
    
    aliasing_loop(array1, array2, array3, 256);
    
    /* Use results to prevent optimization */
    volatile double final_sink = result1 + array1[100] + array2[100] + array3[100];
    
    return (int)(final_sink * 0.001);
}
