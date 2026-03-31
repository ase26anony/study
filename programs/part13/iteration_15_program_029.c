/* test_ddg.c - Complex loop to trigger DDG edge creation in GCC */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
int get_offset(int i) {
    static int offsets[4] = {0, 1, -1, 2};
    return offsets[i & 3];
}

void process_data(double* restrict dest, const double* src, 
                  const double* coeff, int n, int stride) {
    double acc = 1.0;
    double temp_acc = 0.0;
    
    /* Force loop-carried dependency with high-latency operations */
    for (int i = 1; i < n; ++i) {
        /* 1. True Data Dependency (RAW) with loop-carried recurrence */
        double val = src[i] * coeff[i];          /* RAW: src/coeff -> val */
        acc = acc + val;                         /* RAW: acc,val -> acc (loop-carried) */
        
        /* 2. High-latency operation (division) */
        double div_result = acc / (coeff[i] + 1.0);  /* RAW: acc,coeff -> div_result */
        
        /* 3. Anti-dependency (WAR) via array access */
        double prev = dest[i-1];                 /* Read dest[i-1] */
        dest[i] = div_result + prev;             /* Write dest[i] (WAR: dest[i-1] read then dest[i] written) */
        
        /* 4. Output dependency (WAW) with conditional */
        if (i % 8 == 0) {
            acc = external_func(div_result);     /* WAW: acc overwritten, function call latency */
        }
        
        /* 5. Complex aliasing with pointer arithmetic */
        int offset = get_offset(i);
        dest[i + offset] = src[i] * 0.5;         /* Potential WAW/WAR with previous dest writes */
        
        /* 6. Another output dependency in same iteration */
        temp_acc = div_result;                   /* WAW: temp_acc overwritten each iteration */
        
        /* 7. Control flow creating different dependency paths */
        if (src[i] > 0.5) {
            dest[i] = dest[i] * 2.0;             /* Additional RAW on dest[i] */
        } else {
            dest[i] = dest[i] * 0.5;             /* Alternative path */
        }
        
        /* 8. Integer division with variable divisor (high latency) */
        int int_val = (int)(src[i] * 100);
        if (int_val != 0) {
            int divisor = (int)(coeff[i] * 10) + 1;
            int ratio = int_val / divisor;       /* Integer division latency */
            dest[i] += ratio * 0.01;
        }
    }
    
    /* Volatile sink to prevent dead code elimination */
    volatile double sink = acc + dest[n/2] + temp_acc;
    (void)sink;
}

/* Main function with initialization and multiple loops */
int main() {
    const int N = 1024;
    double src[N];
    double coeff[N];
    double dest[N];
    
    /* Initialize with pattern */
    for (int i = 0; i < N; ++i) {
        src[i] = (i % 10) * 0.1;
        coeff[i] = 1.0 + (i % 7) * 0.05;
        dest[i] = 0.0;
    }
    
    /* First loop with complex dependencies */
    process_data(dest, src, coeff, N, 1);
    
    /* Second loop with different stride for additional DDG patterns */
    double dest2[N];
    for (int i = 0; i < N; ++i) dest2[i] = 0.0;
    
    volatile int stride = 2;
    double acc2 = 0.5;
    
    for (int i = 2; i < N; i += stride) {
        /* Loop-carried with memory dependencies */
        double x = src[i] * acc2;
        double y = sqrt(x);                     /* sqrt has variable latency */
        
        /* Anti-dependency chain */
        double t = dest2[i-2];
        dest2[i] = y + t;
        
        /* Output dependency with conditional */
        if (y > 1.0) {
            acc2 = y * 0.8;                     /* WAW on acc2 */
        } else {
            acc2 = y * 1.2;                     /* Alternative WAW */
        }
        
        /* Complex addressing inhibiting alias analysis */
        int idx = i + ((int)y & 3);
        if (idx < N) {
            dest2[idx] = src[i] + dest2[i];     /* Potential WAR/WAW */
        }
    }
    
    /* Use results */
    volatile double sink2 = acc2 + dest2[N/3];
    (void)sink2;
    
    return 0;
}
