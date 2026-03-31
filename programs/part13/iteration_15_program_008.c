/* test_ddg.c - Complex loop to trigger DDG edge creation */

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
                  double* shared_mem,  /* Non-restrict for aliasing */
                  int n) {
    double acc = 1.0;
    double temp_reg;
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* 1. TRUE DEPENDENCY (RAW) with high latency operation */
        double val = src1[i] * src2[i];      /* Read src1, src2 */
        acc = acc + val;                     /* Loop-carried: acc depends on previous acc */
        
        /* 2. ANTI-DEPENDENCY (WAR) with memory */
        temp_reg = shared_mem[i-1];          /* Read from shared_mem */
        shared_mem[i] = acc * temp_reg;      /* Write to shared_mem - anti-dep on line above */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        if (i % 16 == 0) {
            acc = external_func(acc);        /* Function call with latency */
            /* This creates WAW on acc with the acc update above */
        }
        
        /* 4. Loop-carried memory dependency with distance > 0 */
        result[i] = result[i-1] + acc;       /* Distance=1 carried dependency */
        
        /* 5. Complex addressing to confuse alias analysis */
        int idx = i + get_offset(i);
        if (idx > 0 && idx < n) {
            /* Creates potential dependencies with unknown aliasing */
            shared_mem[idx] = shared_mem[i] * 0.5;
        }
        
        /* 6. Integer division with variable divisor (high latency) */
        int divisor = (i & 3) + 2;
        int int_result = (i * 100) / divisor;  /* Integer division latency */
        
        /* 7. Control flow creating control dependencies */
        if (int_result > 1000) {
            acc = acc * 0.9;                 /* Control-dependent update */
        } else {
            acc = acc * 1.1;                 /* Alternative path */
        }
        
        /* 8. Floating-point division (high latency) */
        if (src2[i] != 0.0) {
            temp_reg = src1[i] / src2[i];    /* FP division latency */
            result[i] += temp_reg;
        }
    }
    
    /* Volatile sink to prevent dead code elimination */
    volatile double sink = acc + result[n-1] + shared_mem[0];
    (void)sink;
}

/* Main function with initialization */
int main() {
    const int N = 1024;
    double src1[N], src2[N], result[N], shared[N];
    
    /* Initialize with deterministic but non-constant values */
    for (int i = 0; i < N; ++i) {
        src1[i] = (i % 100) * 0.01;
        src2[i] = ((i + 1) % 50) * 0.02;
        result[i] = i * 0.001;
        shared[i] = (i % 10) * 0.1;
    }
    
    /* Process with complex dependencies */
    process_data(result, src1, src2, shared, N);
    
    /* Additional volatile use */
    volatile double final_check = result[N/2] + shared[N/4];
    (void)final_check;
    
    return 0;
}
