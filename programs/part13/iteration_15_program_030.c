/* Complex loop to trigger DDG edge creation in GCC scheduler */
#include <math.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Global arrays to prevent optimizations */
double global_src[2048];
double global_dest[2048];
double global_coeff[2048];

/* Function with complex loop to build detailed DDG */
void process_loop(int start, int end, int offset, double *restrict result) {
    /* Local arrays with potential aliasing */
    double src[1024];
    double dest[1024];
    double coeff[1024];
    double temp[1024];
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < 1024; i++) {
        src[i] = (i % 256) * 0.01;
        coeff[i] = ((i % 128) + 1) * 0.02;
        dest[i] = 0.0;
        temp[i] = 0.0;
    }
    
    /* Volatile to prevent dead code elimination */
    volatile double acc = 1.0;
    volatile double acc2 = 2.0;
    
    /* Complex loop with multiple dependency types */
    for (int i = start; i < end; i++) {
        /* 1. TRUE DEPENDENCY (RAW) with high-latency operation */
        double t1 = src[i] * coeff[i];          /* Read src, coeff */
        double t2 = external_func(t1);          /* Function call latency */
        double t3 = sqrt(t2 + acc);             /* FP sqrt latency */
        
        /* 2. ANTI-DEPENDENCY (WAR) with array access */
        double old_val = dest[i-1];             /* Read dest[i-1] */
        dest[i] = t3 + old_val;                 /* Write dest[i] - WAR with next iteration */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        if (i % 8 == 0) {
            acc = 1.0;                          /* WAW: overwrites acc */
        } else {
            acc = t3 * 0.5;                     /* WAW: different write to acc */
        }
        
        /* 4. LOOP-CARRIED DEPENDENCY with distance > 0 */
        temp[i] = temp[i-1] + src[i];           /* Distance=1 recurrence */
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + (int)(coeff[i % 4] * 10);
        if (idx < 1024) {
            dest[idx] = src[i] * 2.0;           /* May alias with dest[i] */
        }
        
        /* 6. CONTROL DEPENDENCY with branching */
        if (src[i] > 0.5) {
            /* Integer division with variable divisor (high latency) */
            int divisor = (i % 16) + 1;
            int int_result = (int)(t3 * 100) / divisor;
            dest[i] += int_result * 0.01;
        }
        
        /* 7. ANOTHER LOOP-CARRIED DEPENDENCY */
        acc2 = acc2 * 0.99 + src[i] * 0.01;     /* Distance=1, different recurrence */
        
        /* 8. MEMORY OPERATIONS with pointer aliasing */
        double *ptr1 = &dest[i];
        double *ptr2 = &dest[(i + offset) % 1024];
        *ptr1 = *ptr1 + *ptr2 * 0.1;            /* Potential WAR/WAW */
        
        /* 9. GLOBAL MEMORY ACCESS (more conservative analysis) */
        global_dest[i] = global_src[i] + global_coeff[i % 256];
    }
    
    /* Use results to prevent elimination */
    *result = acc + acc2 + dest[end-1] + temp[end-1];
}

/* Second loop with different pattern to ensure DDG construction */
void process_loop2(int n, double *arr1, double *arr2, double *arr3) {
    double sum1 = 0.0, sum2 = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple interleaved recurrences */
        sum1 = sum1 / ((i % 7) + 1) + arr1[i];  /* Integer division latency */
        sum2 = sum2 * 0.9 + arr2[i-1];          /* Loop-carried */
        
        /* Output dependency on arr3 */
        if (i % 3 == 0) {
            arr3[i] = sum1;
        } else {
            arr3[i] = sum2;
        }
        
        /* Anti-dependency through array */
        double tmp = arr3[i-1];                 /* Read */
        arr1[i] = tmp * arr2[i];                /* Write to arr1 - WAR with next iteration? */
        
        /* Complex conditional with control dependency */
        arr2[i] = (tmp > 0.5) ? sqrt(tmp) : log(fabs(tmp) + 1.0);
    }
    
    /* Volatile sink */
    volatile double sink = sum1 + sum2 + arr3[n-1];
}

int main() {
    double result1, result2;
    
    /* Initialize global arrays */
    for (int i = 0; i < 2048; i++) {
        global_src[i] = sin(i * 0.01);
        global_coeff[i] = cos(i * 0.005);
    }
    
    /* Process with different parameters to avoid constant propagation */
    for (int iter = 0; iter < 3; iter++) {
        double arr1[512], arr2[512], arr3[512];
        
        for (int i = 0; i < 512; i++) {
            arr1[i] = i * 0.02;
            arr2[i] = (i % 37) * 0.03;
        }
        
        /* Call both processing functions */
        process_loop(1, 500, iter * 10, &result1);
        process_loop2(500, arr1, arr2, arr3);
        
        /* Mix results to create external dependency */
        result2 = result1 + arr3[499];
    }
    
    /* Final volatile sink to ensure all computations are used */
    volatile double final_sink = result1 + result2 + global_dest[100];
    
    return (int)(final_sink * 0.0);  /* Return 0 but prevent dead code elimination */
}
