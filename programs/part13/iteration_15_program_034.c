/* test_ddg_coverage.c
 * Designed to trigger DDG edge creation in GCC's instruction scheduler
 * Compile with: gcc -O2 -fschedule-insns -fno-unroll-loops -fno-if-conversion -S test_ddg_coverage.c
 */

#include <math.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
static int get_offset(int i) __attribute__((noinline));
static int get_offset(int i) {
    return (i * 3) % 7;
}

/* Main test function with loop containing multiple dependency types */
void process_loop(double* restrict src, double* restrict coeff, 
                  double* dest1, double* dest2, int n) {
    double acc = 1.0;
    volatile double vol_sink;  /* Prevent optimizations */
    
    /* Loop with multiple dependency patterns */
    for (int i = 1; i < n; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double temp = src[i] * coeff[i];      /* RAW: src/coeff read, temp written */
        temp = sqrt(temp);                    /* RAW: temp read/written (sqrt has latency) */
        
        /* 2. LOOP-CARRIED DEPENDENCY with distance > 0 */
        acc = acc + temp;                     /* RAW: acc read/written, distance=1 */
        
        /* 3. ANTI-DEPENDENCY (WAR) through memory aliasing */
        double prev_val = dest1[i-1];         /* Read dest1[i-1] */
        dest1[i] = acc + prev_val;            /* Write dest1[i] (WAR with previous read) */
        
        /* 4. OUTPUT DEPENDENCY (WAW) */
        if (i % 8 == 0) {
            acc = external_func(acc);         /* WAW: acc overwritten, function call latency */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int offset = get_offset(i);           /* Function call inhibits analysis */
        dest2[i + offset] = src[i] * 2.0;     /* Complex addressing */
        
        /* 6. INTEGER DIVISION with variable latency */
        int divisor = (i % 5) + 1;
        int int_result = i / divisor;         /* Integer division with variable divisor */
        
        /* 7. CONTROL DEPENDENCY with side effects */
        if (int_result > 100) {
            dest1[i] = dest1[i] * 0.5;        /* Control-dependent write */
        }
        
        /* 8. ANOTHER OUTPUT DEPENDENCY through pointer */
        double* ptr = &dest1[i];
        *ptr = *ptr + 1.0;                    /* Pointer-based WAW */
        
        /* Use volatile to prevent dead code elimination */
        vol_sink = acc;
    }
    
    /* Final volatile sink */
    volatile double final_sink = acc + dest1[10] + dest2[20];
}

/* Wrapper to ensure the loop is actually called */
int main() {
    /* Large enough arrays to prevent complete unrolling */
    #define SIZE 1024
    static double src[SIZE];
    static double coeff[SIZE];
    static double dest1[SIZE + 10];  /* Extra space for complex addressing */
    static double dest2[SIZE + 10];
    
    /* Initialize with simple patterns */
    for (int i = 0; i < SIZE; ++i) {
        src[i] = (double)(i % 100) * 0.01;
        coeff[i] = (double)((i * 7) % 13) * 0.1;
        dest1[i] = (double)i * 0.001;
        dest2[i] = (double)i * 0.002;
    }
    
    /* Call the processing function multiple times */
    for (int iter = 0; iter < 10; ++iter) {
        process_loop(src, coeff, dest1, dest2, SIZE);
    }
    
    return 0;
}
