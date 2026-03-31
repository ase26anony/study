/* test_ddg.c - Complex loop to trigger DDG edge creation */

#include <math.h>
#include <stdlib.h>

/* Non-inlineable function to force latency modeling */
static double external_computation(double x, double y) __attribute__((noinline));
static double external_computation(double x, double y) {
    return x * y + (x - y);
}

/* Complex loop with multiple dependency types */
void process_data(double* restrict dest, 
                  const double* src1, 
                  const double* src2,
                  double* shared_buf,  /* Non-restrict for aliasing */
                  int size, 
                  int offset) {
    double acc = 1.0;
    volatile double volatile_acc = 0.0; /* Prevent optimizations */
    
    /* Loop with multiple dependency patterns */
    for (int i = 1; i < size; ++i) {
        /* 1. TRUE DEPENDENCY (RAW) with high-latency operation */
        double temp = src1[i] * src2[i];          /* Read after previous iteration's acc */
        acc = external_computation(acc, temp);    /* Loop-carried, function call latency */
        
        /* 2. ANTI-DEPENDENCY (WAR) with memory aliasing */
        double prev_val = shared_buf[i-1];        /* Read from shared buffer */
        shared_buf[i] = acc + prev_val;           /* Write to same buffer - WAR */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        if (i % 16 == 0) {
            acc = sqrt(fabs(acc));                /* High-latency sqrt, overwrites acc */
        }
        
        /* 4. Loop-carried memory dependency with distance > 0 */
        dest[i] = dest[i-1] * 0.9 + acc;          /* Distance=1 recurrence */
        
        /* 5. Complex addressing to confuse alias analysis */
        int complex_idx = i + offset + (int)src1[i % 8];
        if (complex_idx < size && complex_idx >= 0) {
            shared_buf[complex_idx] = src2[i] * 0.5;  /* May alias with other accesses */
        }
        
        /* 6. Control flow creating control dependencies */
        if (acc > 100.0) {
            acc = acc / 2.0;                      /* Integer division by non-constant */
        } else if (acc < -50.0) {
            acc = external_computation(acc, -1.0);
        }
        
        volatile_acc = acc; /* Volatile write to prevent dead code elimination */
    }
    
    /* Final output dependency */
    dest[0] = acc;
}

/* Another function with different pattern to ensure DDG for multiple loops */
void process_data_2(float* arr_a, float* arr_b, float* arr_c, int n) {
    float sum = 0.0f;
    
    for (int i = 2; i < n; ++i) {
        /* Mixed float/int operations */
        float x = arr_a[i];
        float y = arr_b[i-1];  /* Loop-carried read */
        
        /* True dependency chain */
        float t1 = x * y;
        float t2 = t1 + sum;   /* Uses sum from previous iteration */
        
        /* Anti-dependency */
        float old_c = arr_c[i];
        arr_c[i-1] = t2;       /* WAR: Write to arr_c[i-1] after reading arr_c[i] */
        
        /* Output dependency on sum */
        if (i % 3 == 0) {
            sum = 0.0f;        /* WAW: Overwrites sum */
        } else {
            sum = t2;          /* WAW: Different write to sum */
        }
        
        /* High-latency operation */
        arr_a[i] = sqrtf(fabsf(arr_b[i]));  /* sqrt latency */
    }
}

/* Main function to drive everything */
int main() {
    const int SIZE = 512;
    
    /* Allocate and initialize arrays */
    double* dest = (double*)malloc(SIZE * sizeof(double));
    double* src1 = (double*)malloc(SIZE * sizeof(double));
    double* src2 = (double*)malloc(SIZE * sizeof(double));
    double* shared = (double*)malloc(SIZE * sizeof(double));
    
    float* arr_a = (float*)malloc(SIZE * sizeof(float));
    float* arr_b = (float*)malloc(SIZE * sizeof(float));
    float* arr_c = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; ++i) {
        src1[i] = (i % 7) * 0.5;
        src2[i] = (i % 5) * 0.3;
        shared[i] = (i % 11) * 0.7;
        dest[i] = 0.0;
        
        arr_a[i] = i * 0.1f;
        arr_b[i] = i * 0.2f;
        arr_c[i] = i * 0.3f;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 3; ++iter) {
        int offset = iter * 2;
        process_data(dest, src1, src2, shared, SIZE, offset);
        process_data_2(arr_a, arr_b, arr_c, SIZE);
    }
    
    /* Volatile sink to prevent dead code elimination */
    volatile double sink = dest[SIZE/2] + shared[SIZE/4];
    volatile float fsink = arr_a[SIZE/3] + arr_b[SIZE/3];
    
    /* Cleanup */
    free(dest);
    free(src1);
    free(src2);
    free(shared);
    free(arr_a);
    free(arr_b);
    free(arr_c);
    
    return (int)(sink + fsink) % 2;
}
