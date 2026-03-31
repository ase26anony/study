/* ddg_test.c - Test program for GCC DDG edge creation coverage */
#include <math.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
static int get_offset(int i, double* coeff) __attribute__((noinline));
static int get_offset(int i, double* coeff) {
    return (int)(coeff[i % 4] * 2.0) % 3;
}

/* Main test function with complex loop dependencies */
void process_data(double* src, double* coeff, double* dest, int n, int base_offset) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double tmp;
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        double val = src[i] * coeff[i];
        acc = acc + val;
        
        /* 2. High-latency operation (division) */
        if (acc != 0.0) {
            acc = 1.0 / acc;  /* Variable latency division */
        }
        
        /* 3. Anti-dependency (WAR) with memory */
        double prev = dest[i-1];  /* Read */
        dest[i] = acc + prev;     /* Write to same location */
        
        /* 4. Output dependency (WAW) with control flow */
        if (i % 16 == 0) {
            acc = external_func(acc);  /* Function call with latency */
            tmp = sqrt(acc);           /* Another high-latency op */
        } else {
            tmp = acc * 2.0;
        }
        
        /* 5. Complex addressing with potential aliasing */
        int offset = get_offset(i, coeff) + base_offset;
        dest[i + offset] = src[i] * tmp;  /* May alias with dest[i] */
        
        /* 6. Another output dependency */
        acc = tmp + coeff[i % 8];
        
        /* 7. Integer division with variable divisor (more latency) */
        int int_val = (int)acc;
        if (int_val != 0) {
            int divisor = (int)coeff[i % 4] + 1;
            if (divisor != 0) {
                int_val = 1000 / divisor;  /* Variable integer division */
            }
        }
        
        /* 8. Memory operation with pointer arithmetic */
        double* ptr = dest + i;
        *ptr = *ptr + int_val;
    }
    
    /* Use results to prevent dead code elimination */
    sink = acc + dest[n/2];
}

/* Secondary loop with different pattern to ensure DDG construction */
void process_data2(float* a, float* b, float* c, int n) {
    float sum = 0.0f;
    
    for (int i = 2; i < n; ++i) {
        /* Loop-carried recurrence */
        sum = sum + a[i] * b[i-1];
        
        /* Anti-dependency chain */
        float t1 = c[i-1];
        c[i] = sum + t1;
        
        /* Output dependency with condition */
        if (sum > 100.0f) {
            sum = 0.5f;
        }
        
        /* Complex array access */
        a[i + (int)b[i%3]] = c[i-2] * 0.3f;
        
        /* Function call for latency */
        sum = external_func(sum);
    }
    
    volatile float sink2 = sum + c[n/3];
}

int main() {
    const int SIZE = 1024;
    
    /* Source arrays with deterministic values */
    double src[SIZE];
    double coeff[SIZE];
    double dest[SIZE];
    
    float arr1[SIZE];
    float arr2[SIZE];
    float arr3[SIZE];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; ++i) {
        src[i] = sin(i * 0.01);
        coeff[i] = cos(i * 0.005) + 1.0;
        dest[i] = i * 0.1;
        
        arr1[i] = i * 0.25f;
        arr2[i] = i * 0.33f;
        arr3[i] = i * 0.5f;
    }
    
    /* Process with different offset patterns */
    process_data(src, coeff, dest, SIZE, 1);
    process_data(src, coeff, dest, SIZE, 2);
    
    process_data2(arr1, arr2, arr3, SIZE);
    process_data2(arr1, arr2, arr3, SIZE/2);
    
    /* Final volatile sink to prevent optimization */
    volatile double final_sink = dest[SIZE-1] + arr3[SIZE-2];
    
    return (int)final_sink;
}
