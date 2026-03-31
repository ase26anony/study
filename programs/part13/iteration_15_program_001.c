/* Complex loop to trigger DDG edge creation in GCC scheduler */
#include <math.h>

/* Non-inlineable function to create variable latency */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
int get_offset(int i, int* offsets) {
    return offsets[i % 4];
}

void process_loop(double* restrict result1, double* restrict result2) {
    /* Prevent optimizations */
    volatile int start = 1;
    int n = 1024;
    
    /* Source arrays with overlapping memory regions */
    double src[1028];      /* Larger than needed for aliasing */
    double coeff[1028];
    double dest[1028];
    int offsets[4] = {0, 1, -1, 2};
    
    /* Initialize arrays */
    for (int i = 0; i < 1028; i++) {
        src[i] = i * 0.1;
        coeff[i] = sin(i * 0.01);
        dest[i] = cos(i * 0.02);
    }
    
    /* Loop-carried accumulator with output dependency */
    double acc = src[0];
    double temp_acc = 0.0;
    
    /* Main loop with complex dependencies */
    for (int i = start; i < n; i++) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val1 = src[i] * coeff[i];      /* Read src[i], coeff[i] */
        double val2 = external_func(val1);    /* Function call - variable latency */
        acc = sqrt(acc + val2);               /* Loop-carried: acc depends on previous acc */
        
        /* 2. ANTI-DEPENDENCY (WAR) */
        double old_val = dest[i-1];           /* Read dest[i-1] */
        dest[i] = acc + old_val;              /* Write dest[i] - WAR with previous iteration */
        
        /* 3. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 8 == 0) {
            acc = 1.0;                        /* Overwrites acc - WAW dependency */
        }
        
        /* 4. COMPLEX ADDRESSING with potential aliasing */
        int offset = get_offset(i, offsets);
        /* This creates conservative dependencies because dest[i+offset] 
           might alias with other dest accesses */
        dest[i + offset] = src[i] * 0.5;
        
        /* 5. MORE DATA DEPENDENCIES with integer division (variable latency) */
        int divisor = (i % 16) + 1;
        int int_val = (int)src[i] / divisor;  /* Integer division - variable latency */
        
        /* 6. CONTROL DEPENDENCY affecting memory access */
        double conditional_val;
        if (int_val > 10) {
            conditional_val = dest[i] * 2.0;
        } else {
            conditional_val = dest[i-1] * 3.0;
        }
        
        /* 7. ANOTHER LOOP-CARRIED DEPENDENCY with different distance */
        temp_acc = temp_acc + conditional_val * 0.1;
        
        /* 8. MEMORY DEPENDENCY CHAIN */
        src[i+1] = src[i] * 0.9 + dest[i] * 0.1;
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile double sink1 = acc;
    volatile double sink2 = temp_acc;
    volatile double sink3 = dest[10];
    
    /* Store results */
    *result1 = acc;
    *result2 = temp_acc;
}

/* Second function with pointer aliasing to create conservative dependencies */
void aliasing_loop(double* a, double* b, int n) {
    /* a and b might alias - compiler must be conservative */
    double acc = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Read from b[i-1] then write to a[i] - potential WAR if a and b alias */
        double temp = b[i-1];
        a[i] = temp * 2.0 + acc;
        
        /* Loop-carried dependency through acc */
        acc = a[i] * 0.5;
        
        /* Write to b[i] after reading - potential WAW/WAR if aliased */
        b[i] = acc * 3.0;
        
        /* Complex control flow */
        if (i % 3 == 0) {
            a[i] = sqrt(a[i]);  /* High latency operation */
        }
    }
    
    volatile double sink = acc;
}

int main() {
    double result1, result2;
    
    /* Process main complex loop */
    process_loop(&result1, &result2);
    
    /* Process aliasing loop */
    double array1[256];
    double array2[256];
    
    for (int i = 0; i < 256; i++) {
        array1[i] = i * 0.01;
        array2[i] = i * 0.02;
    }
    
    aliasing_loop(array1, array2, 256);
    
    /* Use results to prevent elimination */
    volatile double final_sink = result1 + result2 + array1[100] + array2[100];
    
    return (int)(final_sink * 0);
}
