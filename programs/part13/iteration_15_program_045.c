/* ddg_test.c - Test program to trigger DDG edge creation in GCC */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
int get_offset(int i) __attribute__((noinline));
int get_offset(int i) {
    static int offsets[4] = {0, 1, -1, 2};
    return offsets[i & 3];
}

/* Main test function */
int main(void) {
    /* Source arrays with overlapping memory regions */
    double src[2048];
    double coeff[1024];
    double dest[2048];
    
    /* Initialize with deterministic values */
    for (int i = 0; i < 2048; i++) {
        src[i] = (i % 100) * 0.01;
        if (i < 1024) coeff[i] = (i % 50) * 0.02;
        dest[i] = 0.0;
    }
    
    /* Loop-carried accumulator with volatile to prevent optimization */
    volatile double acc = 1.0;
    double local_acc = acc;
    
    /* Pointer aliasing to force conservative dependency analysis */
    double* ptr1 = &src[512];
    double* ptr2 = &dest[512];
    
    /* Main loop with complex dependencies */
    for (int i = 1; i < 1024; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high-latency operation */
        double temp = src[i] * coeff[i];      /* Read src[i], coeff[i] */
        temp = external_func(temp);           /* Function call latency */
        
        /* 2. LOOP-CARRIED DEPENDENCY with sqrt (high latency) */
        local_acc = local_acc + temp;         /* RAW on local_acc */
        if (local_acc > 0) {
            local_acc = sqrt(local_acc);      /* High-latency sqrt */
        }
        
        /* 3. ANTI-DEPENDENCY (WAR) through array access */
        double prev_val = dest[i-1];          /* Read dest[i-1] */
        dest[i] = local_acc + prev_val;       /* Write dest[i] - WAR with next iteration */
        
        /* 4. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 16 == 0) {
            local_acc = 1.0;                  /* WAW on local_acc */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int offset = get_offset(i);           /* Function call prevents constant propagation */
        dest[i + offset] = src[i];            /* May alias with other dest accesses */
        
        /* 6. INTEGER DIVISION (variable latency) */
        int int_val = (int)(local_acc * 1000);
        if (int_val != 0) {
            int divisor = (i % 7) + 1;        /* Non-constant divisor */
            int_val = int_val / divisor;      /* Variable-latency integer division */
        }
        
        /* 7. ANOTHER TRUE DEPENDENCY CHAIN */
        double val2 = dest[i] * 0.5;          /* RAW on dest[i] */
        dest[i] = val2 + src[i-1];            /* WAW on dest[i] */
        
        /* 8. CONTROL FLOW creating control dependencies */
        if (src[i] > 0.5) {
            coeff[i] = coeff[i] * 2.0;        /* WAR on coeff[i] for next iteration? */
        } else {
            coeff[i] = coeff[i] / 2.0;        /* Alternative WAW */
        }
        
        /* 9. POINTER ALIASING operations */
        *ptr1 = *ptr1 + 0.1;                  /* May alias with src/dest */
        *ptr2 = *ptr2 - 0.1;                  /* May alias with dest */
        
        /* 10. FLOATING DIVISION (high latency) */
        if (coeff[i] != 0.0) {
            src[i] = src[i] / coeff[i];       /* High-latency FP division */
        }
    }
    
    /* Volatile sink to prevent dead code elimination */
    volatile double sink = local_acc + dest[10] + src[100] + coeff[50];
    
    return (int)sink;
}

/* Additional function in separate compilation unit to prevent inlining */
double helper_func(double a, double b) {
    return a > b ? a / (b + 1.0) : b / (a + 1.0);
}
