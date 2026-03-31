/* test_ddg.c - Complex loop to trigger DDG edge creation */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Global arrays to prevent over-optimization */
double global_src[2048];
double global_dest[2048];
double global_coeff[2048];
double global_aux[2048];

/* Complex addressing function to inhibit alias analysis */
int get_offset(int i, int base) {
    static int counter = 0;
    counter = (counter + 1) & 3;
    return base + counter;
}

void process_loop(int n, int offset1, int offset2) {
    volatile double sink; /* Prevent dead code elimination */
    double acc = 1.0;
    double temp_acc;
    int i;
    
    /* Initialize arrays with simple patterns */
    for (i = 0; i < n; i++) {
        global_src[i] = i * 0.5;
        global_coeff[i] = (i % 7) * 0.1;
        global_aux[i] = (i % 5) * 0.2;
    }
    
    /* Main loop with complex dependencies */
    for (i = 1; i < n - 1; i++) {
        /* 1. Loop-carried true dependency (RAW) with high-latency operation */
        double val = global_src[i] * global_coeff[i];
        acc = acc + val;
        
        /* High-latency floating-point division */
        if (acc != 0.0) {
            acc = acc / (global_aux[i] + 1.0);  /* Variable divisor */
        }
        
        /* 2. Anti-dependency (WAR) with memory aliasing */
        double read_before_write = global_dest[i - 1];  /* Read */
        global_dest[i] = acc + read_before_write;       /* Write to same array */
        
        /* 3. Output dependency (WAW) with control flow */
        if (i % 8 == 0) {
            acc = external_func(acc);  /* Function call with latency */
            temp_acc = acc;
        } else {
            temp_acc = acc * 0.9;
        }
        
        /* 4. Complex addressing to confuse alias analysis */
        int idx1 = get_offset(i, offset1);
        int idx2 = get_offset(i, offset2);
        
        /* Overlapping writes creating output dependencies */
        global_dest[idx1] = temp_acc + global_src[idx2];
        
        /* 5. Another loop-carried dependency with integer division */
        if (i % 3 == 0) {
            int divisor = (i % 10) + 1;  /* Non-constant divisor */
            int int_val = (int)acc / divisor;  /* Integer division with latency */
            global_dest[i + 1] = int_val * 0.5;
        }
        
        /* 6. Pointer aliasing to force conservative analysis */
        double *ptr1 = &global_dest[i];
        double *ptr2 = &global_dest[i + offset1 % 4];
        
        if (ptr1 != ptr2) {
            *ptr2 = *ptr1 + global_coeff[i];
        }
        
        /* 7. Control flow creating different dependency paths */
        double conditional_result;
        if (global_src[i] > 50.0) {
            conditional_result = sqrt(acc);  /* Another high-latency op */
        } else {
            conditional_result = acc * acc;
        }
        
        /* Write to multiple potentially overlapping locations */
        global_aux[i] = conditional_result;
        global_aux[i + offset2 % 3] = conditional_result * 0.8;
    }
    
    /* Use volatile sink to prevent elimination */
    sink = acc + global_dest[10] + global_aux[20];
}

/* Secondary loop with different pattern */
void process_loop2(int n) {
    double acc1 = 0.0, acc2 = 1.0;
    int i;
    
    for (i = 2; i < n; i++) {
        /* Cross-iteration dependencies with different distances */
        global_dest[i] = global_dest[i - 1] * 0.9 + global_dest[i - 2] * 0.1;
        
        /* Multiple accumulators with output dependencies */
        if (i % 5 == 0) {
            acc1 = global_src[i] / (i + 1);  /* Division */
        } else {
            acc2 = acc1 + global_coeff[i];
        }
        
        /* Complex addressing with potential aliasing */
        int idx = (i * 7) % n;
        global_aux[idx] = acc1 + acc2;
        
        /* Periodic reset creating output dependency */
        if (i % 12 == 0) {
            acc1 = 0.0;
            acc2 = 1.0;
        }
    }
}

int main() {
    int iterations = 1024;
    
    /* Process with different offset patterns */
    process_loop(iterations, 2, 3);
    process_loop(iterations, 1, 4);
    process_loop2(iterations);
    
    /* Final volatile store to ensure all computations are used */
    volatile double final_sink = 0.0;
    for (int i = 0; i < 10; i++) {
        final_sink += global_dest[i] + global_aux[i];
    }
    
    return (int)final_sink;
}
