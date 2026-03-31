/* test_ddg.c - Complex loop to trigger DDG edge creation in GCC */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
int get_offset(int i, int* offsets) {
    return offsets[i % 4];
}

void process_loop(double* restrict result1, 
                  double* restrict result2,
                  const double* data1,
                  const double* data2,
                  int* offsets,
                  int n) {
    double acc = 1.0;
    double temp_acc = 0.0;
    
    /* Prevent loop invariant motion */
    volatile int barrier = 0;
    
    for (int i = 1; i < n; ++i) {
        /* 1. True Data Dependency (RAW) with high latency operation */
        double val1 = data1[i] * 2.5;
        double val2 = external_func(val1);  /* Function call latency */
        
        /* 2. Loop-carried dependency with division (high latency) */
        acc = acc / (val2 + 1.0);  /* Division has variable latency */
        
        /* 3. Anti-dependency (WAR) - read then write same location */
        double old_val = result1[i-1];      /* Read */
        result1[i] = acc + old_val + temp_acc; /* Write - creates WAR */
        
        /* 4. Output dependency (WAW) */
        temp_acc = val2 * 0.5;
        if (i % 16 == 0) {
            temp_acc = 1.0 / (data2[i] + 0.001);  /* Overwrites temp_acc - WAW */
        }
        
        /* 5. Complex addressing to confuse alias analysis */
        int idx = i + get_offset(i, offsets);
        if (idx < n && idx > 0) {
            /* Creates potential dependencies with other iterations */
            result2[idx] = result1[i] * 0.3;
        }
        
        /* 6. Control flow inside loop */
        if (data2[i] > 100.0) {
            /* Branch creates control dependencies */
            acc = sqrt(acc);  /* sqrt has variable latency */
        } else if (data2[i] < -100.0) {
            acc = external_func(acc);
        }
        
        /* 7. Another output dependency with memory */
        barrier = i;  /* volatile write creates memory barrier */
        result1[i] = result1[i] + barrier * 0.0001;  /* WAW with earlier write */
        
        /* 8. Integer division with variable divisor (high latency) */
        int int_val = (int)(data1[i] * 1000);
        if (int_val != 0) {
            offsets[i % 4] = 1000 / (int_val + 1);  /* Integer division latency */
        }
    }
    
    /* Ensure loop computations aren't optimized away */
    result1[0] = acc;
    result2[0] = temp_acc;
}

/* Main function to set up data and call the loop */
int main() {
    const int N = 1024;
    
    /* Allocate and initialize arrays with non-trivial patterns */
    double* data1 = __builtin_alloca(N * sizeof(double));
    double* data2 = __builtin_alloca(N * sizeof(double));
    double* result1 = __builtin_alloca(N * sizeof(double));
    double* result2 = __builtin_alloca(N * sizeof(double));
    int* offsets = __builtin_alloca(4 * sizeof(int));
    
    /* Initialize with values that create interesting dependencies */
    for (int i = 0; i < N; ++i) {
        data1[i] = (i % 37) * 1.5;
        data2[i] = (i % 23) * 2.1 - 25.0;
        result1[i] = 0.0;
        result2[i] = 0.0;
    }
    
    /* Initialize offsets for complex addressing */
    offsets[0] = 1;
    offsets[1] = -1;
    offsets[2] = 2;
    offsets[3] = -2;
    
    /* Call the processing loop multiple times to ensure execution */
    for (int iter = 0; iter < 3; ++iter) {
        process_loop(result1, result2, data1, data2, offsets, N);
        
        /* Modify inputs slightly for next iteration */
        for (int i = 0; i < N; ++i) {
            data1[i] += 0.1;
            data2[i] -= 0.05;
        }
    }
    
    /* Use volatile sink to prevent dead code elimination */
    volatile double sink1 = result1[N/2];
    volatile double sink2 = result2[N/4];
    volatile int sink3 = offsets[0];
    
    return (int)(sink1 + sink2 + sink3) % 256;
}
