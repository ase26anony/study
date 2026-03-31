/* Complex loop to trigger DDG edge creation in GCC scheduler */
#include <math.h>

/* Non-inlineable function to create variable latency */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing to confuse alias analysis */
int get_offset(int i) {
    static int offsets[8] = {0, 1, -1, 2, -2, 3, -3, 4};
    return offsets[i & 7];
}

void process_loop(double* restrict result1, 
                  double* restrict result2,
                  const double* data1,
                  const double* data2,
                  int n) {
    double acc1 = 0.0, acc2 = 0.0;
    double temp_array[1024];
    
    /* Initialize temp_array with data */
    for (int i = 0; i < n && i < 1024; i++) {
        temp_array[i] = data1[i] * 0.5;
    }
    
    /* Main complex loop with multiple dependency types */
    for (int i = 1; i < n; i++) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val1 = data1[i] / (data2[i] + 1.0);  /* Division has variable latency */
        
        /* 2. LOOP-CARRIED DEPENDENCY with recurrence */
        acc1 = acc1 + val1 * external_func(acc1);  /* distance > 0 */
        
        /* 3. ANTI-DEPENDENCY (WAR) - read then write same location */
        double old_temp = temp_array[i-1];  /* Read */
        
        /* 4. OUTPUT DEPENDENCY (WAW) - multiple writes to same variable */
        if (i % 16 == 0) {
            acc2 = 1.0;  /* First write to acc2 */
        }
        
        /* Complex addressing to inhibit alias analysis */
        int idx = i + get_offset(i);
        if (idx >= 0 && idx < n) {
            /* 5. MEMORY ANTI-DEPENDENCY with pointer aliasing */
            double* ptr1 = result1 + idx;
            double* ptr2 = result2 + i;
            
            /* Read from potentially aliased locations */
            double v1 = *ptr1;
            double v2 = *ptr2;
            
            /* Write causing WAR dependency */
            temp_array[i] = v1 + v2 + acc1;  /* Write - anti-dep with line 3 */
            
            /* 6. CONTROL DEPENDENCY inside loop */
            if (val1 > 0.0) {
                /* 7. Another OUTPUT DEPENDENCY (WAW) */
                acc2 = sqrt(fabs(acc1));  /* Second write to acc2 - WAW with line 4 */
                
                /* Integer division with variable divisor */
                int divisor = (i % 7) + 1;
                int int_val = i / divisor;  /* Variable latency integer division */
                
                /* Use result to prevent elimination */
                *ptr1 = acc2 * int_val;
            } else {
                *ptr1 = old_temp * 0.5;
            }
        }
        
        /* 8. Another LOOP-CARRIED DEPENDENCY with memory */
        if (i > 2) {
            result2[i] = result2[i-2] * 0.8 + acc1;  /* distance = 2 */
        }
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile double sink1 = acc1;
    volatile double sink2 = acc2;
    volatile double sink3 = temp_array[n-1];
    (void)sink1; (void)sink2; (void)sink3;
}

/* Main function to set up data and call the loop */
int main() {
    const int N = 512;
    double data1[N], data2[N];
    double result1[N], result2[N];
    
    /* Initialize with pattern to avoid constant propagation */
    for (int i = 0; i < N; i++) {
        data1[i] = (i % 37) * 0.1;
        data2[i] = (i % 41) * 0.2 + 0.1;
        result1[i] = 0.0;
        result2[i] = 0.0;
    }
    
    /* Call the complex loop multiple times to ensure it's not optimized away */
    for (int iter = 0; iter < 10; iter++) {
        process_loop(result1, result2, data1, data2, N);
    }
    
    /* Final volatile sink */
    volatile double final_sink = result1[N/2] + result2[N/2];
    (void)final_sink;
    
    return 0;
}
