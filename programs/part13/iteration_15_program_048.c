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

void process_data(double* restrict dest, 
                  const double* src1, 
                  const double* src2,
                  double* shared,  /* Non-restrict for aliasing */
                  int n) {
    double acc = 1.0;
    double temp_reg = 0.0;
    
    /* Main loop with complex dependencies */
    for (int i = 1; i < n; ++i) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        double val = src1[i] * src2[i];
        acc = acc / (val + 1.0);  /* Division for latency */
        
        /* 2. Anti-dependency (WAR) via shared memory */
        double read_before_write = shared[i-1];  /* Read */
        shared[i] = acc + read_before_write;     /* Write to same location */
        
        /* 3. Output dependency (WAW) on temp_reg */
        temp_reg = external_func(acc);  /* First write */
        
        /* 4. Complex addressing with potential aliasing */
        int idx = i + get_offset(i);
        if (idx >= 0 && idx < n) {
            dest[idx] = temp_reg + src1[i];  /* May alias with shared[] */
        }
        
        /* 5. Control flow inside loop */
        if (i % 16 == 0) {
            /* Creates control dependency and another WAW */
            temp_reg = 0.0;  /* Second write to temp_reg */
            acc = external_func(shared[i]);  /* Function call latency */
        }
        
        /* 6. Memory operation with pointer arithmetic */
        double* ptr = shared + i;
        *ptr = *ptr + 1.0;  /* Read-modify-write */
        
        /* 7. Integer division with variable divisor (more latency) */
        int int_val = (int)(acc * 1000);
        if (int_val != 0) {
            int divisor = (i & 31) + 1;  /* Non-constant divisor */
            int result = int_val / divisor;  /* Integer division latency */
            dest[i] += result;  /* Accumulate */
        }
    }
    
    /* Volatile sink to prevent optimization */
    volatile double sink = acc + dest[n/2] + shared[n-1];
    (void)sink;
}

/* Another function with different dependency patterns */
void process_data2(float* a, float* b, float* c, int n) {
    float acc1 = a[0], acc2 = b[0];
    
    for (int i = 1; i < n; ++i) {
        /* Cross-iteration dependencies */
        float t1 = acc1 + a[i];
        float t2 = acc2 + b[i];
        
        /* Swap and mix */
        acc1 = t2 * 0.7f;
        acc2 = t1 * 0.3f;
        
        /* Conditional store with output dependency */
        if (i % 8 == 0) {
            c[i] = acc1;
        } else {
            c[i] = acc2;
        }
        
        /* Function call with side effect */
        c[i] = external_func(c[i]);
        
        /* Array access with stride */
        a[i * 2 % n] = acc1 + acc2;
    }
    
    volatile float vsink = acc1 + acc2 + c[n/4];
    (void)vsink;
}

int main() {
    const int N = 1024;
    
    /* Source arrays with simple patterns */
    double src1[N], src2[N];
    double dest[N], shared[N];
    
    for (int i = 0; i < N; ++i) {
        src1[i] = i * 0.1;
        src2[i] = i * 0.05;
        dest[i] = 0.0;
        shared[i] = i * 0.01;
    }
    
    /* Process with complex dependencies */
    process_data(dest, src1, src2, shared, N);
    
    /* Second processing function */
    float fa[N], fb[N], fc[N];
    for (int i = 0; i < N; ++i) {
        fa[i] = i * 0.2f;
        fb[i] = i * 0.15f;
        fc[i] = 0.0f;
    }
    
    process_data2(fa, fb, fc, N);
    
    /* Use results to prevent dead code elimination */
    volatile double final_check = dest[N/3] + shared[N/2] + fc[N/4];
    
    return (int)(final_check * 0.0);  /* Return 0 without revealing actual value */
}
