/* ddg_test.c - Complex loop to trigger DDG edge creation in GCC */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
int get_offset(int i, int* offsets) __attribute__((noinline));
int get_offset(int i, int* offsets) {
    return offsets[i % 4];
}

/* Main test function */
int main(void) {
    /* Source arrays with initialization */
    double src[1024];
    double coeff[1024];
    double dest[1024];
    int offsets[4] = {0, 1, -1, 2};
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < 1024; i++) {
        src[i] = (double)(i % 100) * 0.1;
        coeff[i] = (double)((i * 3) % 50) * 0.02;
        dest[i] = (double)(i % 10);
    }
    
    /* Loop-carried accumulator with volatile to prevent optimization */
    volatile double acc = 1.0;
    double local_acc = acc;
    
    /* Additional variables for complex dependencies */
    double temp1, temp2;
    int idx;
    
    /* 
     * MAIN TARGET LOOP - Complex enough to require detailed DDG construction
     * This loop contains all required dependency types:
     */
    for (int i = 1; i < 1023; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high-latency operation */
        temp1 = src[i] * coeff[i];           /* RAW: src/coeff -> temp1 */
        temp2 = local_acc + temp1;           /* RAW: local_acc, temp1 -> temp2 */
        
        /* High-latency operation (division) */
        if (temp2 != 0.0) {
            local_acc = 1.0 / temp2;         /* RAW: temp2 -> local_acc */
        }
        
        /* 2. ANTI-DEPENDENCY (WAR) with memory access */
        double read_before_write = dest[i-1]; /* Read dest[i-1] */
        dest[i] = local_acc + read_before_write; /* Write dest[i] - WAR with next iteration */
        
        /* 3. LOOP-CARRIED DEPENDENCY with recurrence */
        /* This creates distance > 0 edges in DDG */
        src[i] = src[i-1] * 0.9 + src[i] * 0.1; /* RAW with distance=1 */
        
        /* 4. OUTPUT DEPENDENCY (WAW) */
        if (i % 8 == 0) {
            local_acc = 1.0;                 /* WAW: overwrites local_acc */
        }
        
        /* 5. COMPLEX ADDRESSING with pointer arithmetic */
        idx = i + get_offset(i, offsets);    /* Function call inhibits analysis */
        if (idx >= 0 && idx < 1024) {
            dest[idx] = src[i] + coeff[i];   /* Complex aliasing possibilities */
        }
        
        /* 6. CONTROL DEPENDENCY inside loop */
        if (local_acc > 100.0) {             /* Control dependency */
            local_acc = external_func(local_acc); /* Non-inlineable function call */
        } else {
            local_acc = local_acc * 0.99;    /* Alternative path */
        }
        
        /* 7. ADDITIONAL OUTPUT DEPENDENCY with array */
        coeff[i] = coeff[i] * 0.95;          /* WAW on coeff array */
    }
    
    /* Force result usage to prevent dead code elimination */
    volatile double sink1 = local_acc;
    volatile double sink2 = dest[10];
    volatile double sink3 = src[100];
    
    return (int)(sink1 + sink2 + sink3) % 256;
}

/* Additional function in separate compilation unit style */
double another_external(double x) {
    /* Integer division with variable divisor - high latency */
    int divisor = (int)x % 10 + 1;
    return (double)((int)x / divisor);
}
