/* ddg_test.c - Test program for GCC DDG edge creation coverage */

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
                  double* shared,  /* Non-restrict pointer for aliasing */
                  int n) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double temp_reg; /* Register for anti-dependency */
    
    /* Loop with complex dependencies */
    for (int i = 1; i < n; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val = src1[i] * src2[i];      /* RAW: src1/src2 -> val */
        acc = external_func(acc + val);      /* RAW: acc,val -> acc (loop-carried) */
        
        /* 2. ANTI-DEPENDENCY (WAR) through shared memory */
        temp_reg = shared[i-1];              /* Read shared[i-1] */
        shared[i] = acc * 2.0;               /* Write shared[i] (WAR with next iter) */
        
        /* 3. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 16 == 0) {
            acc = 0.5;                       /* WAW: overwrites acc */
        }
        
        /* 4. Loop-carried dependency with distance > 1 */
        if (i >= 3) {
            dest[i] = dest[i-3] + temp_reg;  /* Distance=3 dependency */
        } else {
            dest[i] = acc;
        }
        
        /* 5. Complex addressing to confuse alias analysis */
        int idx = i + get_offset(i);
        if (idx >= 0 && idx < n) {
            shared[idx] = src1[i] * 0.25;    /* May alias with other accesses */
        }
        
        /* 6. Integer division (variable latency) */
        int divisor = (i & 31) + 1;
        int int_result = i / divisor;        /* Variable latency division */
        dest[i] += int_result * 0.01;
        
        /* 7. Control dependency affecting memory access */
        double* target = (i % 8 == 0) ? dest : shared;
        target[i % n] = acc * 3.0;
    }
    
    /* Volatile sink to prevent dead code elimination */
    sink = acc + dest[n/2] + shared[n/4];
}

/* Secondary loop with different dependency patterns */
void process_data2(float* a, float* b, float* c, int n) {
    float acc1 = 0.0f, acc2 = 0.0f;
    
    for (int i = 2; i < n; ++i) {
        /* Multiple interleaved recurrences */
        acc1 = a[i] + b[i-1] * acc1;         /* Loop-carried, distance=1 */
        acc2 = c[i] - b[i-2] * acc2;         /* Loop-carried, distance=2 */
        
        /* Cross-iteration output dependency */
        if ((i ^ (i >> 1)) & 1) {            /* Gray code pattern */
            a[i] = acc1;
            b[i] = acc2;
        } else {
            a[i] = acc2;
            b[i] = acc1;
        }
        
        /* Memory anti-dependency with pointer swapping */
        float* p1 = (i % 3 == 0) ? a : b;
        float* p2 = (i % 3 == 1) ? b : c;
        float tmp = p1[i-1];                  /* Read */
        p2[i] = tmp * 0.5f;                   /* Write (may alias) */
    }
}

int main() {
    const int SIZE = 1024;
    
    /* Source arrays with simple patterns */
    double src1[SIZE], src2[SIZE];
    double dest[SIZE], shared[SIZE];
    
    for (int i = 0; i < SIZE; ++i) {
        src1[i] = i * 0.1;
        src2[i] = i * 0.05;
        dest[i] = 0.0;
        shared[i] = i * 0.01;
    }
    
    /* Process with complex dependencies */
    process_data(dest, src1, src2, shared, SIZE);
    
    /* Second processing stage with different data types */
    float fa[SIZE], fb[SIZE], fc[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        fa[i] = i * 0.2f;
        fb[i] = i * 0.3f;
        fc[i] = i * 0.4f;
    }
    
    process_data2(fa, fb, fc, SIZE);
    
    /* Final volatile sink */
    volatile double final_sink = dest[SIZE-1] + fa[SIZE/2] + shared[SIZE/3];
    
    return (int)final_sink;
}
