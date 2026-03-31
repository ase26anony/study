/* test_ddg.c - Complex loop to trigger DDG edge creation in GCC */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
int get_offset(int i) {
    static int offsets[8] = {0, 1, -1, 2, -2, 3, -3, 4};
    return offsets[i & 7];
}

void process_data(double* restrict result, 
                  const double* src1, 
                  const double* src2,
                  double* shared_buf1,  /* Non-restrict for aliasing */
                  double* shared_buf2,  /* Non-restrict for aliasing */
                  int n) {
    volatile double sink;  /* Prevent optimizations */
    
    /* Loop-carried accumulator with high-latency operations */
    double acc = 1.0;
    
    /* Multiple arrays with potential aliasing */
    double local_buf[16];
    
    /* Initialize local buffer with simple pattern */
    for (int j = 0; j < 16; ++j) {
        local_buf[j] = j * 0.1;
    }
    
    /* Main complex loop - target for DDG construction */
    for (int i = 1; i < n; ++i) {
        /* ========== TRUE DATA DEPENDENCIES (RAW) ========== */
        /* Chain of dependent floating-point operations */
        double t1 = src1[i] * 2.0;           /* Node 1 */
        double t2 = t1 + src2[i];            /* Node 2 - depends on t1 */
        double t3 = external_func(t2);       /* Node 3 - depends on t2, high latency */
        
        /* Loop-carried recurrence with high-latency sqrt */
        acc = sqrt(acc + t3);                /* Node 4 - depends on acc (loop-carried) and t3 */
        
        /* ========== ANTI-DEPENDENCIES (WAR) ========== */
        /* Read from shared buffer before writing to it */
        double old_val = shared_buf1[i-1];   /* Node 5 - read */
        shared_buf1[i] = acc + old_val;      /* Node 6 - write, anti-dep on Node 5 */
        
        /* ========== OUTPUT DEPENDENCIES (WAW) ========== */
        /* Multiple writes to same location with control flow */
        if (i % 16 == 0) {
            acc = 0.5;                       /* Node 7 - overwrites acc, output dep on Node 4 */
        }
        
        /* ========== COMPLEX ADDRESSING FOR ALIASING ========== */
        /* Address depends on computation, inhibits alias analysis */
        int idx = i + get_offset(i);
        if (idx >= 0 && idx < n) {
            shared_buf2[idx] = t2;           /* Node 8 - complex addressing */
        }
        
        /* ========== CONTROL DEPENDENCIES ========== */
        /* Conditional with data-dependent branch */
        double conditional_result;
        if (t1 > 100.0) {                    /* Control dep on t1 computation */
            conditional_result = t1 * 0.5;   /* Node 9a */
        } else {
            conditional_result = t2 * 0.3;   /* Node 9b */
        }
        
        /* ========== MEMORY ALIASING PATTERNS ========== */
        /* Overlapping array accesses */
        result[i] = conditional_result + local_buf[i & 15];
        
        /* Potential aliasing between different arrays */
        if (i % 8 == 0) {
            shared_buf1[i] = shared_buf2[i/2];  /* Could alias with previous accesses */
        }
        
        /* Integer division for additional latency */
        int divisor = (i % 10) + 1;
        int int_result = (i * 100) / divisor;   /* Node 10 - integer division latency */
        
        /* Use integer result to affect floating-point computation */
        local_buf[i & 15] += int_result * 0.01;
    }
    
    /* Volatile sink to prevent dead code elimination */
    sink = acc + result[n-1];
}

/* Secondary loop with different characteristics */
void process_data_2(float* __restrict out,
                    const float* __restrict in1,
                    const float* __restrict in2,
                    int n) {
    float acc1 = 0.0f, acc2 = 0.0f;
    
    for (int i = 0; i < n; ++i) {
        /* Multiple accumulators with cross-iteration dependencies */
        float val1 = in1[i] * 2.0f;
        float val2 = in2[i] * 3.0f;
        
        /* Loop-carried dependencies with different distances */
        acc1 = val1 + acc1 * 0.9f;      /* Distance 1 recurrence */
        if (i >= 2) {
            acc2 = val2 + acc2 * 0.8f + out[i-2];  /* Distance 2 recurrence */
        }
        
        /* Conditional output with anti-dependencies */
        float temp = out[i];            /* Read before write - anti-dep */
        out[i] = (i % 3 == 0) ? acc1 : acc2 + temp;
        
        /* High-latency operation periodically */
        if (i % 7 == 0) {
            acc1 = 1.0f / (acc1 + 0.001f);  /* Floating division */
        }
    }
}

int main() {
    const int N = 1024;
    
    /* Allocate and initialize data */
    double* src1 = __builtin_alloca(N * sizeof(double));
    double* src2 = __builtin_alloca(N * sizeof(double));
    double* result = __builtin_alloca(N * sizeof(double));
    double* shared1 = __builtin_alloca(N * sizeof(double));
    double* shared2 = __builtin_alloca(N * sizeof(double));
    
    float* f_out = __builtin_alloca(N * sizeof(float));
    float* f_in1 = __builtin_alloca(N * sizeof(float));
    float* f_in2 = __builtin_alloca(N * sizeof(float));
    
    /* Initialize with simple patterns */
    for (int i = 0; i < N; ++i) {
        src1[i] = i * 0.25;
        src2[i] = i * 0.33;
        shared1[i] = i * 0.1;
        shared2[i] = i * 0.2;
        
        f_in1[i] = i * 0.1f;
        f_in2[i] = i * 0.2f;
        f_out[i] = 0.0f;
    }
    
    /* Process with complex loop */
    process_data(result, src1, src2, shared1, shared2, N);
    
    /* Process with secondary loop */
    process_data_2(f_out, f_in1, f_in2, N);
    
    /* Use volatile to ensure computations aren't optimized away */
    volatile double final_check = result[N/2] + shared1[N/4] + f_out[N/3];
    
    return (int)(final_check * 0.0);  /* Return 0, but compiler doesn't know */
}
