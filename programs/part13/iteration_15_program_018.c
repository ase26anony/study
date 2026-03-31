/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing helper */
static int get_offset(int i) __attribute__((noinline));
static int get_offset(int i) {
    return (i * 3) % 7;
}

/* Global arrays to force conservative alias analysis */
double global_src[2048];
double global_dest[2048];
double global_coeff[2048];
double global_temp[2048];

void process_loop(int start, int end, int offset) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double local_acc = 0.0;
    
    /* Initialize arrays with simple patterns */
    for (int i = 0; i < 2048; i++) {
        global_src[i] = i * 0.1;
        global_coeff[i] = 0.5 + (i % 3) * 0.1;
        global_temp[i] = 0.0;
    }
    
    /*
     * MAIN TARGET LOOP
     * Contains all required dependency types for DDG construction
     */
    for (int i = start; i < end; i++) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high-latency operation */
        double val = global_src[i] * global_coeff[i];  /* Source for next op */
        val = val / (global_coeff[i] + 1.0);           /* Division - high latency */
        
        /* 2. LOOP-CARRIED DEPENDENCY (distance > 0) */
        acc = acc + val;                               /* Recurrence: acc depends on previous iteration */
        
        /* 3. ANTI-DEPENDENCY (WAR) via array access */
        double old_val = global_dest[i - 1];           /* Read before write */
        global_dest[i] = acc + old_val;                /* Write to same location */
        
        /* 4. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 8 == 0) {
            local_acc = external_func(acc);            /* Call - non-inlineable */
        } else {
            local_acc = acc * 0.5;                     /* Different write to same variable */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + offset + get_offset(i);
        if (idx < 2048) {
            global_temp[idx] = global_src[i] + local_acc;
        }
        
        /* 6. ANOTHER OUTPUT DEPENDENCY with memory */
        global_dest[i] = local_acc;                    /* Overwrites previous write */
        
        /* 7. CONTROL DEPENDENCY affecting multiple operations */
        if (global_src[i] > 50.0) {
            /* Nested true dependency chain */
            double tmp = global_temp[i] * 2.0;
            global_temp[i] = tmp / 3.0;                /* Another division */
        }
        
        /* 8. MIXED INTEGER/FLOAT OPERATIONS for varied latency */
        int int_val = (int)global_src[i];
        if (int_val % 3 == 0) {
            /* Integer division - variable latency */
            int divisor = (int_val % 7) + 1;
            int_val = int_val / divisor;               /* Non-constant divisor */
            global_dest[i] += int_val;
        }
    }
    
    /* Use results to prevent dead code elimination */
    sink = acc + global_dest[10] + global_temp[100];
    (void)sink; /* Suppress unused variable warning */
}

/* Secondary loop with different pattern to ensure DDG for multiple loops */
void process_loop2(int n, double* restrict out, double* in1, double* in2) {
    /* Using restrict here to contrast with non-restrict in main loop */
    double sum = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Cross-iteration dependency */
        sum = sum + in1[i] * in2[i-1];
        
        /* Conditional with both taken and not-taken paths */
        if (sum > 100.0) {
            out[i] = sqrt(sum);        /* High latency sqrt */
            sum = out[i] * 0.9;        /* Output dependency on sum */
        } else {
            out[i] = sum;
        }
        
        /* Anti-dependency through array */
        double temp = out[i-1];
        out[i] = out[i] + temp;
    }
}

int main() {
    double array1[1024];
    double array2[1024];
    double result[1024];
    
    /* Initialize data */
    for (int i = 0; i < 1024; i++) {
        array1[i] = i * 0.25;
        array2[i] = i * 0.33;
    }
    
    /* Call first loop processor */
    process_loop(1, 1000, 3);
    
    /* Call second loop processor */
    process_loop2(1024, result, array1, array2);
    
    /* Use results */
    volatile double final_sink = result[500] + global_dest[500];
    (void)final_sink;
    
    return 0;
}
