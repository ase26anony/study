/* ddg_test.c - Test program to trigger DDG edge creation in GCC */

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
int main(int argc, char* argv[]) {
    /* Large enough arrays to prevent complete unrolling */
    double src[2048];
    double coeff[2048];
    double dest[2048];
    double temp_arr[2048];
    
    /* Initialize with deterministic values */
    for (int i = 0; i < 2048; i++) {
        src[i] = (i % 37) * 0.1;
        coeff[i] = (i % 13) * 0.05 + 0.01;
        dest[i] = (i % 29) * 0.03;
        temp_arr[i] = 0.0;
    }
    
    /* Volatile to prevent dead code elimination */
    volatile double sink = 0.0;
    
    /* Loop-carried accumulator with mixed dependencies */
    double acc = src[0] * coeff[0];
    
    /* Pointer for aliasing complexity */
    double* ptr1 = dest;
    double* ptr2 = temp_arr;
    
    /* Offset array for complex addressing */
    int offsets[4] = {0, 1, -1, 2};
    
    /* Main target loop with complex dependencies */
    for (int i = 1; i < 2047; i++) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val1 = src[i] * coeff[i];      /* Read src[i], coeff[i] */
        double val2 = external_func(val1);    /* Function call - non-inlineable */
        
        /* 2. LOOP-CARRIED DEPENDENCY (distance > 0) */
        acc = acc / (val2 + 1.0);             /* Division - variable latency */
        
        /* 3. ANTI-DEPENDENCY (WAR) */
        double old_val = ptr1[i-1];           /* Read before write */
        ptr1[i] = acc + old_val;              /* Write to same location */
        
        /* 4. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 16 == 0) {
            acc = 1.0;                        /* Overwrites acc */
        } else if (i % 8 == 0) {
            acc = external_func(acc);         /* Another function call */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + get_offset(i, offsets); /* Complex index calculation */
        if (idx >= 0 && idx < 2048) {
            ptr2[idx] = src[i] + ptr1[i-1];   /* Multiple array accesses */
        }
        
        /* 6. MEMORY RECURRENCE with different distance */
        temp_arr[i] = temp_arr[i-1] * 0.9 + src[i];
        
        /* 7. INTEGER DIVISION with variable divisor */
        int int_div = (i * 100) / ((int)src[i] + 2);
        ptr1[i] += int_div * 0.001;
    }
    
    /* Use results to prevent optimization */
    sink = acc + dest[100] + temp_arr[500];
    
    /* Additional volatile store */
    volatile double final_result = sink;
    
    return (int)(final_result * 0.0);  /* Return 0 without revealing result */
}

/* Force separate compilation unit for external_func */
double dummy_call(void) {
    return external_func(1.0);
}
