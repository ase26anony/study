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

void process_loop(double* restrict result1, double* restrict result2, 
                  const double* data, int n, int* offsets) {
    double acc = 1.0;
    double temp_array[1024];
    
    /* Initialize with some values */
    for (int i = 0; i < n; i++) {
        temp_array[i] = (double)i * 0.5;
    }
    
    /* Main loop with complex dependencies */
    for (int i = 1; i < n - 1; i++) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val1 = data[i] * 2.0;          /* Read data[i] */
        double val2 = sqrt(val1 + acc);       /* RAW: uses val1 and acc, sqrt has variable latency */
        
        /* 2. ANTI-DEPENDENCY (WAR) with memory aliasing */
        double old_val = temp_array[i];       /* Read temp_array[i] */
        temp_array[i] = val2 * 3.0;           /* WAR: overwrites location just read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) on accumulator */
        acc = old_val + val2;                 /* WAW: overwrites acc used earlier */
        
        /* 4. LOOP-CARRIED DEPENDENCY with distance > 0 */
        result1[i] = result1[i-1] + acc;      /* RAW with distance=1: uses previous iteration's result */
        
        /* 5. Control flow inside loop */
        if (i % 16 == 0) {
            /* High latency integer division */
            int divisor = (i % 8) + 2;
            int quotient = i / divisor;       /* Variable latency integer division */
            
            /* Function call with side effect */
            acc = external_func(acc);         /* Function call latency */
            
            /* Another output dependency */
            result2[i] = (double)quotient;    /* WAW: overwrites result2[i] */
        } else {
            /* Complex addressing to confuse alias analysis */
            int idx = i + get_offset(i, offsets);
            if (idx < n) {
                result2[idx] = temp_array[i] + data[idx]; /* Multiple array accesses */
            }
        }
        
        /* 6. Mixed dependency types in single expression */
        double complex_val = (result1[i] * 0.3) / (acc + 0.001); /* Division has variable latency */
        
        /* 7. Another loop-carried dependency with anti-dependency */
        double prev_temp = temp_array[i-1];   /* Read previous iteration's temp */
        temp_array[i+1] = complex_val + prev_temp; /* Write to next iteration with RAW & WAR */
        
        /* 8. Periodic output dependency on accumulator */
        if (i % 32 == 0) {
            acc = 0.0;                        /* WAW: resets accumulator */
        }
    }
    
    /* Final computation with high latency */
    result1[n-1] = acc / (data[n-1] + 0.001); /* Division latency */
}

/* Main function to set up and run the loop */
int main() {
    const int N = 1024;
    double data[N];
    double result1[N];
    double result2[N];
    int offsets[4] = {0, 1, -1, 2};
    
    /* Initialize data */
    for (int i = 0; i < N; i++) {
        data[i] = sin(i * 0.01) + 1.0;  /* Non-trivial initialization */
    }
    
    /* Initialize results */
    result1[0] = 1.0;
    result2[0] = 2.0;
    
    /* Process the loop */
    process_loop(result1, result2, data, N, offsets);
    
    /* Use volatile to prevent optimization */
    volatile double sink1 = result1[N-1];
    volatile double sink2 = result2[N-2];
    
    return (int)(sink1 + sink2);
}
