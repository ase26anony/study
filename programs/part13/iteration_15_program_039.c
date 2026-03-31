/* Prevent inlining of external functions */
static double external_func(double x) __attribute__((noinline));
static int get_offset(int i) __attribute__((noinline));

/* External function to force latency modeling */
static double external_func(double x) {
    /* Simulate high-latency operation */
    return x * 0.987654321;
}

/* Function to create complex addressing */
static int get_offset(int i) {
    static int offsets[4] = {0, 1, -1, 2};
    return offsets[i & 3];
}

/* Global arrays to prevent alias analysis */
double global_src[2048];
double global_dest[2048];
double global_coeff[2048];
double global_temp[2048];

int main(int argc, char *argv[]) {
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < 2048; i++) {
        global_src[i] = (i * 1.5) / (i + 1.0);
        global_coeff[i] = (i % 7) * 0.25;
        global_temp[i] = i * 0.1;
    }
    
    /* Volatile to prevent optimizations */
    volatile int start = argc > 1 ? 100 : 1;
    volatile int limit = 1024;
    
    /* Multiple accumulators for different dependency types */
    double acc1 = global_src[0];
    double acc2 = global_temp[0];
    double acc3 = 0.0;
    
    /* Pointer aliasing - same data accessed through different pointers */
    double *ptr1 = global_dest;
    double *ptr2 = global_dest + 512;
    
    /* Main loop with complex dependencies */
    for (int i = start; i < limit; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency */
        double val1 = global_src[i] * global_coeff[i];
        acc1 = acc1 + val1;  /* Loop-carried dependency */
        
        /* 2. ANTI-DEPENDENCY (WAR) */
        double old_val = ptr1[i-1];           /* Read before write */
        ptr1[i] = acc1 + old_val;             /* Write after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 16 == 0) {
            acc2 = external_func(acc1);       /* High-latency call */
        } else {
            acc2 = global_temp[i] * 2.0;      /* Different write to same var */
        }
        
        /* 4. Loop-carried dependency with distance > 1 */
        if (i >= 3) {
            acc3 = global_dest[i-3] * 0.5 + acc3;  /* Distance = 3 */
        }
        
        /* 5. Complex addressing with potential aliasing */
        int offset = get_offset(i);
        ptr2[i + offset] = acc2 + global_src[i];  /* May alias with ptr1 */
        
        /* 6. Integer division (variable latency) */
        int divisor = (i % 13) + 1;
        int int_result = (i * 100) / divisor;  /* Non-constant divisor */
        
        /* 7. Memory dependency through different arrays */
        global_temp[i] = global_temp[i-1] + int_result * 0.01;
        
        /* 8. Control flow creating different execution paths */
        double conditional_result;
        if (acc1 > acc2) {
            conditional_result = acc1 - acc2;
        } else {
            conditional_result = acc2 - acc1;
        }
        
        /* 9. Another output dependency */
        acc1 = conditional_result * 0.9;  /* Overwrites acc1 */
        
        /* 10. Floating-point division (high latency) */
        if (acc3 != 0.0) {
            acc2 = acc1 / (acc3 + 0.001);  /* Prevents division by zero */
        }
    }
    
    /* Use volatile sinks to prevent dead code elimination */
    volatile double sink1 = acc1;
    volatile double sink2 = acc2;
    volatile double sink3 = acc3;
    volatile double sink4 = global_dest[10];
    volatile double sink5 = global_temp[100];
    
    return (int)(sink1 + sink2 + sink3 + sink4 + sink5);
}
