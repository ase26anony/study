/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

/* Non-inlineable function to create variable latency */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to inhibit alias analysis */
int get_offset(int i, int* offsets) {
    return offsets[i % 4];
}

void process_loop(double* restrict src, double* restrict coeff, 
                  double* dest1, double* dest2, int* offsets, int n) {
    double acc = 1.0;
    volatile double vol_var = 0.0;  /* Prevent optimizations */
    
    /* Main loop with complex dependencies */
    for (int i = 1; i < n; ++i) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        double temp = src[i] * coeff[i];
        acc = acc + temp;                    /* Loop-carried RAW */
        
        /* 2. High latency operation */
        acc = external_func(acc);            /* Function call latency */
        
        /* 3. Anti-dependency (WAR) */
        double prev_val = dest1[i-1];        /* Read before write */
        dest1[i] = acc + prev_val;           /* WAR: dest1[i-1] read, then dest1[i] written */
        
        /* 4. Output dependency (WAW) with control flow */
        if (i % 8 == 0) {
            acc = 1.0;                       /* WAW: overwrites acc */
        } else {
            acc = acc / 2.5;                 /* High latency division */
        }
        
        /* 5. Complex addressing with potential aliasing */
        int idx = i + get_offset(i, offsets);
        if (idx < n) {
            dest2[idx] = src[i];             /* Complex memory access */
        }
        
        /* 6. Another loop-carried dependency */
        vol_var = acc;                       /* Volatile prevents reordering */
        dest2[i] = dest2[i] + vol_var;       /* RAW on dest2[i] */
        
        /* 7. Create cross-iteration memory dependency */
        if (i > 2) {
            dest1[i] = dest1[i-2] * 0.5;     /* Distance=2 loop-carried */
        }
    }
    
    /* Use results to prevent elimination */
    volatile double sink = acc + dest1[10] + dest2[20];
}

/* Secondary loop with different pattern */
void process_loop2(float* a, float* b, float* c, int n) {
    float sum = 0.0f;
    
    for (int i = 1; i < n; ++i) {
        /* Mix of dependencies */
        float t1 = a[i] * b[i];              /* Independent */
        float t2 = c[i-1] + t1;              /* Loop-carried RAW on c */
        
        /* Conditional with dependencies */
        if (t2 > 0) {
            c[i] = sqrtf(t2);                /* High latency sqrt */
            sum += c[i];                     /* Loop-carried on sum */
        } else {
            c[i] = t1;
            sum -= t1;                       /* Loop-carried on sum */
        }
        
        /* Anti-dependency */
        a[i-1] = b[i] * 0.3f;                /* WAR: a[i-1] written after potential read */
    }
    
    volatile float sink = sum;
}

int main() {
    const int N = 1024;
    
    /* Allocate and initialize arrays */
    double* src = __builtin_alloca(N * sizeof(double));
    double* coeff = __builtin_alloca(N * sizeof(double));
    double* dest1 = __builtin_alloca(N * sizeof(double));
    double* dest2 = __builtin_alloca(N * sizeof(double));
    int* offsets = __builtin_alloca(4 * sizeof(int));
    
    float* fa = __builtin_alloca(N * sizeof(float));
    float* fb = __builtin_alloca(N * sizeof(float));
    float* fc = __builtin_alloca(N * sizeof(float));
    
    /* Initialize with simple patterns */
    for (int i = 0; i < N; ++i) {
        src[i] = i * 0.1;
        coeff[i] = 1.0 + (i % 3) * 0.1;
        dest1[i] = i * 0.01;
        dest2[i] = i * 0.02;
        
        fa[i] = i * 0.05f;
        fb[i] = i * 0.07f;
        fc[i] = i * 0.03f;
    }
    
    offsets[0] = 0;
    offsets[1] = 1;
    offsets[2] = -1;
    offsets[3] = 2;
    
    /* Call loops multiple times to ensure execution */
    for (int iter = 0; iter < 3; ++iter) {
        process_loop(src, coeff, dest1, dest2, offsets, N);
        process_loop2(fa, fb, fc, N);
    }
    
    return 0;
}
