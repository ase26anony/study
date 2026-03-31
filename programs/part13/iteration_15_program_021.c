/* test_ddg.c - Complex loop to trigger DDG edge creation in GCC */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to confuse alias analysis */
static int get_offset(int i, int *offsets) {
    return offsets[i % 4];
}

void process_data(double *dest, double *src, double *coeff, 
                  double *alt, int *offsets, int n) {
    double acc = 1.0;
    double tmp;
    
    /* Main loop with complex dependencies */
    for (int i = 1; i < n; ++i) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        acc = acc / (src[i] + 0.5);  /* Division has variable latency */
        
        /* 2. Anti-dependency (WAR) through memory */
        tmp = dest[i-1];              /* Read from dest[i-1] */
        dest[i] = acc + tmp;          /* Write to dest[i] - anti-dep with previous iteration */
        
        /* 3. Output dependency (WAW) with control flow */
        if (i % 16 == 0) {
            acc = external_func(acc); /* Function call with latency */
        }
        
        /* 4. Complex aliasing with pointer arithmetic */
        int idx = i + get_offset(i, offsets);
        if (idx < n) {
            alt[idx] = src[i] * coeff[i];  /* May alias with dest[] */
        }
        
        /* 5. Another loop-carried dependency through coeff array */
        coeff[i] = coeff[i-1] * 0.99;
        
        /* 6. Control flow creating different execution paths */
        double val = (i % 8 == 0) ? src[i] * 2.0 : src[i] / 2.0;
        
        /* 7. Memory operation with potential output dependency */
        if (val > 1.0) {
            dest[i] = val;  /* WAW with line 2's dest[i] assignment */
        }
        
        /* 8. Integer division with variable divisor (high latency) */
        int int_div = (i * 100) / ((int)src[i] + 1);
        
        /* 9. Use result to prevent dead code elimination */
        alt[i] += (double)int_div * 0.01;
    }
    
    /* Volatile sink to prevent optimization */
    volatile double sink = acc + dest[n/2] + alt[n/2];
}

int main(void) {
    const int N = 1024;
    
    /* Allocate and initialize arrays with non-constant patterns */
    double src[N], coeff[N], dest[N], alt[N];
    int offsets[4] = {0, 1, -1, 2};
    
    for (int i = 0; i < N; ++i) {
        src[i] = (i % 37) * 0.1;      /* Non-linear pattern */
        coeff[i] = 1.0 - (i % 5) * 0.05;
        dest[i] = (double)i;
        alt[i] = (double)(i * 2);
    }
    
    /* Volatile to prevent invariant motion */
    volatile int iterations = N;
    
    /* Process multiple times to ensure loop isn't optimized away */
    for (int repeat = 0; repeat < 3; ++repeat) {
        process_data(dest, src, coeff, alt, offsets, iterations);
    }
    
    /* Final volatile sink */
    volatile double final_result = dest[N-1] + alt[N-1];
    
    return (int)final_result % 256;
}
