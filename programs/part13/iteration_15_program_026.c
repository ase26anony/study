/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;  /* Simulate external computation */
}

/* Complex addressing helper */
static int get_offset(int i) __attribute__((noinline));
static int get_offset(int i) {
    return (i % 3) - 1;  /* Returns -1, 0, or 1 */
}

int main(void) {
    /* Declare arrays with overlapping memory regions */
    double src[1028];      /* Larger than needed for safe overflow */
    double coeff[1028];
    double dest[1028];
    double alt[1028];
    
    volatile double sink = 0.0;  /* Prevent dead code elimination */
    
    /* Initialize arrays with deterministic values */
    for (int i = 0; i < 1028; ++i) {
        src[i] = (i % 97) * 0.01;
        coeff[i] = (i % 53) * 0.02 + 0.5;
        dest[i] = (i % 71) * 0.03;
        alt[i] = (i % 29) * 0.04;
    }
    
    /* Loop-carried accumulator with volatile to prevent optimization */
    volatile double acc = 1.0;
    double local_acc = acc;  /* Non-volatile working copy */
    
    /* Pointer aliasing - dest and alt may alias */
    double *ptr1 = dest;
    double *ptr2 = alt + 4;  /* Offset creates potential overlap */
    
    /* Main target loop with complex dependencies */
    for (int i = 2; i < 1024; ++i) {
        /* 1. TRUE DEPENDENCY (RAW) with high-latency operation */
        double temp = src[i] * coeff[i];      /* Read src[i], coeff[i] */
        temp = external_func(temp);           /* Function call latency */
        
        /* 2. LOOP-CARRIED DEPENDENCY (distance=1) */
        local_acc = local_acc / (1.0 + temp); /* Integer division-like latency */
        
        /* 3. ANTI-DEPENDENCY (WAR) with memory */
        double prev_val = ptr1[i-1];          /* Read before write */
        ptr1[i] = local_acc + prev_val;       /* Write to same array */
        
        /* 4. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 16 == 0) {
            local_acc = 1.0;                  /* Overwrites local_acc */
        } else if (i % 8 == 0) {
            local_acc = sqrt(local_acc);      /* High-latency sqrt */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int offset = get_offset(i);           /* Non-constant offset */
        ptr2[i + offset] = src[i] * 0.5;      /* May alias with ptr1 */
        
        /* 6. ANOTHER OUTPUT DEPENDENCY on same location */
        if (coeff[i] > 1.0) {
            ptr1[i] = -local_acc;             /* Second write to ptr1[i] */
        }
        
        /* 7. CONTROL DEPENDENCY affecting memory access */
        double *target = (i % 3) ? ptr1 : ptr2;
        target[i-2] = target[i-2] + 0.01;     /* Conditional store */
        
        /* 8. ANOTHER LOOP-CARRIED DEPENDENCY with distance=2 */
        if (i >= 4) {
            src[i] = src[i-2] * 0.9;          /* Distance 2 recurrence */
        }
    }
    
    /* Force accumulator to be used */
    acc = local_acc;
    
    /* Consume results to prevent elimination */
    sink = acc + dest[10] + alt[20] + src[30];
    
    /* Additional computation to ensure loop isn't dead */
    double sum = 0.0;
    for (int i = 0; i < 1024; ++i) {
        sum += dest[i] + alt[i];
    }
    sink = sum;
    
    return (int)(sink * 0);
}
