/* test_ddg.c - Complex loop to trigger DDG edge creation */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
static int get_offset(int i) __attribute__((noinline));
static int get_offset(int i) {
    static const int offsets[] = {0, 1, -1, 2};
    return offsets[i & 3];
}

/* Main test function */
int main(void) {
    /* Source arrays with overlapping memory regions */
    double src[2048];
    double coeff[1024];
    double dest[2048];
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < 2048; i++) {
        src[i] = (i % 37) * 0.1;
        if (i < 1024) coeff[i] = (i % 19) * 0.05;
        dest[i] = (i % 23) * 0.15;
    }
    
    /* Loop-carried accumulator with volatile to prevent elimination */
    volatile double acc = 1.0;
    double local_acc = acc;
    
    /* Pointer aliasing to force conservative dependency analysis */
    double* ptr1 = &dest[512];
    double* ptr2 = &dest[0];
    
    /* Main target loop with complex dependencies */
    for (int i = 1; i < 1024; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high-latency operation */
        double temp = src[i] * coeff[i];
        temp = external_func(temp);  /* Function call with latency */
        
        /* 2. LOOP-CARRIED DEPENDENCY with sqrt (high latency) */
        local_acc = local_acc + temp;
        if (local_acc > 0) {
            local_acc = sqrt(local_acc);  /* High-latency FP operation */
        }
        
        /* 3. ANTI-DEPENDENCY (WAR) - read then write same location */
        double old_val = dest[i-1];          /* Read */
        dest[i] = local_acc + old_val;       /* Write - creates WAR */
        
        /* 4. OUTPUT DEPENDENCY (WAW) - multiple writes to same variable */
        if (i % 8 == 0) {
            local_acc = 1.0;                 /* Overwrites local_acc */
        }
        
        /* 5. COMPLEX ALIASING with pointer arithmetic */
        int offset = get_offset(i);          /* Non-constant offset */
        ptr1[i + offset] = src[i] * 0.5;     /* May alias with dest[] */
        
        /* 6. INTEGER DIVISION with variable divisor (high latency) */
        int divisor = (i % 7) + 1;
        int int_result = (i * 100) / divisor;  /* Variable integer division */
        
        /* 7. CONTROL DEPENDENCY inside loop */
        if (int_result > 500) {
            /* Additional memory operation with potential dependencies */
            ptr2[i] = local_acc * 0.8;
        } else {
            /* Alternative path with different dependencies */
            ptr2[i] = -local_acc;
        }
        
        /* 8. ANOTHER OUTPUT DEPENDENCY with conditional */
        double output_var;
        if (i % 3 == 0) {
            output_var = local_acc * 2.0;
        } else {
            output_var = local_acc * 3.0;
        }
        dest[i + 512] = output_var;  /* Write to potentially overlapping region */
        
        /* 9. MEMORY DEPENDENCY with array of structures */
        struct { double a; double b; } s;
        s.a = src[i];
        s.b = coeff[i];
        dest[i] += s.a + s.b;  /* Another write to dest[i] - WAW */
    }
    
    /* Volatile sink to prevent dead code elimination */
    volatile double sink = local_acc + dest[10] + dest[500];
    
    return (int)(sink * 1000);
}
