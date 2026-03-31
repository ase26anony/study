/* test_ddg.c - Complex loop to trigger DDG edge creation in GCC */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to confuse alias analysis */
static int get_offset(int i) __attribute__((noinline));
static int get_offset(int i) {
    static int offsets[8] = {0, 1, -1, 2, -2, 3, -3, 4};
    return offsets[i & 7];
}

/* Global arrays to prevent SSA optimization */
double global_src[2048];
double global_dest[2048];
double global_coeff[2048];

void init_arrays(void) {
    for (int i = 0; i < 2048; i++) {
        global_src[i] = (i % 37) * 0.1;
        global_coeff[i] = (i % 19) * 0.05 + 0.01;
        global_dest[i] = (i % 23) * 0.2;
    }
}

/* Main computation with complex dependencies */
double compute_loop(int start, int end, double init_acc) {
    double acc = init_acc;
    volatile double* vsrc = global_src;
    volatile double* vdest = global_dest;
    volatile double* vcoeff = global_coeff;
    
    /* Force memory dependencies by using volatile pointers */
    double* src = (double*)vsrc;
    double* dest = (double*)vdest;
    double* coeff = (double*)vcoeff;
    
    /* Loop with multiple dependency types */
    for (int i = start + 1; i < end; i++) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val = src[i] * coeff[i];      /* RAW: src/coeff read */
        val = external_func(val);            /* Function call latency */
        
        /* 2. LOOP-CARRIED DEPENDENCY with sqrt (high latency) */
        acc = acc + val;                     /* Loop-carried RAW */
        if (acc > 0) {
            acc = sqrt(acc);                 /* High latency sqrt */
        }
        
        /* 3. ANTI-DEPENDENCY (WAR) */
        double old_val = dest[i-1];          /* Read dest[i-1] */
        dest[i] = acc + old_val;             /* Write dest[i] - WAR with next iteration */
        
        /* 4. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 16 == 0) {
            acc = 1.0 / (double)(i + 1);     /* High latency division + WAW on acc */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int offset = get_offset(i);
        int idx = i + offset;
        if (idx >= 0 && idx < 2048) {
            /* Potential aliasing between dest[i] and dest[idx] */
            dest[idx] = src[i] * 0.5;        /* More WAW/WAR possibilities */
        }
        
        /* 6. INTEGER DIVISION with variable divisor (high latency) */
        int int_val = i * 37;
        if (int_val != 0) {
            int divisor = (i % 7) + 2;       /* Non-constant divisor */
            int_val = int_val / divisor;     /* Integer division latency */
        }
        
        /* 7. CONTROL DEPENDENCY affecting memory access */
        double temp;
        if (int_val > 100) {
            temp = dest[i] * 1.1;            /* Control-dependent read */
        } else {
            temp = dest[i-1] * 0.9;          /* Different control-dependent read */
        }
        
        /* 8. ANOTHER LOOP-CARRIED DEPENDENCY with anti-dependency */
        src[i] = temp + coeff[i];            /* WAW on src[i], WAR on coeff[i] */
    }
    
    return acc;
}

/* Secondary loop with different pattern to ensure DDG construction */
void process_arrays(double* out, int n) {
    double local_acc = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple interleaved dependencies */
        double a = global_src[i];
        double b = global_dest[i-1];         /* Anti-dependency chain */
        
        /* Output dependency */
        global_dest[i] = a + b;
        
        /* Loop-carried with high latency */
        local_acc = local_acc / (a + 1.0);   /* Division latency + loop-carried */
        
        /* Conditional output dependency */
        if (local_acc < 0) {
            global_src[i] = -local_acc;      /* WAW on global_src[i] */
        }
        
        /* Complex addressing */
        int j = (i * 7) % n;
        if (j != i) {
            global_dest[j] = global_dest[j] + local_acc; /* More WAR/WAW */
        }
    }
    
    *out = local_acc;
}

int main(void) {
    init_arrays();
    
    /* First complex loop */
    double result1 = compute_loop(0, 1024, 1.0);
    
    /* Second processing loop */
    double result2;
    process_arrays(&result2, 512);
    
    /* Volatile sink to prevent dead code elimination */
    volatile double sink1 = result1;
    volatile double sink2 = result2;
    volatile double sink3 = global_dest[100] + global_src[200];
    
    /* Use results to prevent optimization */
    return (int)(sink1 + sink2 + sink3) % 256;
}
