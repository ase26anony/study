/* test_ddg.c - Complex loop to trigger DDG edge creation */

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
    double acc = 0.0;
    double temp_reg = 0.0;
    
    /* Main loop with complex dependencies */
    for (int i = 1; i < n; ++i) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        double val1 = data1[i] * 2.5;
        acc = acc / (val1 + 1.0);  /* Division creates latency */
        
        /* 2. Anti-dependency (WAR) with memory */
        double read_before_write = result2[i-1];
        result2[i] = acc + read_before_write * 0.5;
        
        /* 3. Output dependency (WAW) on temp_reg */
        temp_reg = data2[i] * external_func(acc);
        
        /* 4. Control flow inside loop */
        if (i % 16 == 0) {
            /* Creates control dependency */
            temp_reg = 1.0 / (data1[i] + 0.001);  /* Another division */
            acc = external_func(temp_reg);
        }
        
        /* 5. Complex addressing with potential aliasing */
        int idx = i + get_offset(i, offsets);
        if (idx < n) {
            result1[idx] = temp_reg + result2[i];
        }
        
        /* 6. Another loop-carried dependency with sqrt */
        double sqrt_input = acc + data1[i] + data2[i];
        if (sqrt_input > 0) {
            acc = __builtin_sqrt(sqrt_input);  /* High latency operation */
        }
        
        /* 7. Memory output dependency (WAW) */
        result2[i] = temp_reg;  /* Overwrites earlier write */
    }
    
    /* Volatile sink to prevent optimization */
    volatile double sink = acc + result1[10] + result2[10];
    (void)sink;
}

/* Helper to initialize arrays */
void init_arrays(double* arr1, double* arr2, int* offsets, int n) {
    for (int i = 0; i < n; ++i) {
        arr1[i] = i * 0.1;
        arr2[i] = i * 0.2;
    }
    offsets[0] = 0; offsets[1] = 1; offsets[2] = -1; offsets[3] = 2;
}

int main() {
    const int N = 1024;
    
    /* Allocate arrays without restrict to allow conservative alias analysis */
    double* data1 = __builtin_malloc(N * sizeof(double));
    double* data2 = __builtin_malloc(N * sizeof(double));
    double* result1 = __builtin_malloc(N * sizeof(double));
    double* result2 = __builtin_malloc(N * sizeof(double));
    int* offsets = __builtin_malloc(4 * sizeof(int));
    
    if (!data1 || !data2 || !result1 || !result2 || !offsets) {
        return 1;
    }
    
    init_arrays(data1, data2, offsets, N);
    
    /* Initialize results */
    for (int i = 0; i < N; ++i) {
        result1[i] = 0.0;
        result2[i] = 0.0;
    }
    
    /* Execute the complex loop multiple times */
    for (int iter = 0; iter < 10; ++iter) {
        process_loop(result1, result2, data1, data2, offsets, N);
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile double final_check = result1[N-1] + result2[N-1];
    
    __builtin_free(data1);
    __builtin_free(data2);
    __builtin_free(result1);
    __builtin_free(result2);
    __builtin_free(offsets);
    
    return (int)(final_check * 0.0);  /* Prevent dead code elimination */
}
