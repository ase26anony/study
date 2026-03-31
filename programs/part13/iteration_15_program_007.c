/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing helper */
static int get_offset(int i) __attribute__((noinline));
static int get_offset(int i) {
    return (i * 3) % 7;
}

/* Main test function */
void process_data(double* restrict src, double* restrict coeff, 
                  double* dest, double* alt, int n) {
    double acc = 1.0;
    volatile double* volatile_ptr = dest; /* Prevent optimizations */
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* 1. True Data Dependency (RAW) with high latency */
        double val = src[i] * coeff[i];      /* RAW: src/coeff -> val */
        val = external_func(val);            /* Function call latency */
        
        /* 2. Loop-carried dependency with sqrt (high latency) */
        acc = sqrt(acc + val);               /* RAW: acc -> acc (loop-carried) */
        
        /* 3. Anti-dependency (WAR) - read then write same location */
        double old = dest[i-1];              /* Read dest[i-1] */
        dest[i] = acc + old;                 /* Write dest[i] (WAR with above) */
        
        /* 4. Output dependency (WAW) with control flow */
        if (i % 8 == 0) {
            acc = 1.0;                       /* WAW: acc overwritten */
        } else {
            acc = acc * 0.99;                /* RAW: acc -> acc */
        }
        
        /* 5. Complex aliasing with pointer arithmetic */
        int idx = i + get_offset(i);
        if (idx < n) {
            alt[idx] = src[i];               /* Potential aliasing with dest */
        }
        
        /* 6. Memory dependency through volatile */
        *volatile_ptr = acc;
        
        /* 7. Integer division with variable divisor (high latency) */
        int int_val = (int)acc;
        if (int_val != 0) {
            int divisor = (i % 5) + 1;
            int_val = 1000 / divisor;        /* Integer division latency */
            dest[i] += int_val;
        }
    }
}

/* Secondary function to create cross-iteration dependencies */
void process_with_overlap(double* data, int n) {
    for (int i = 2; i < n; ++i) {
        /* Loop-carried output dependency */
        data[i] = data[i-1] + data[i-2];     /* RAW with distance=1 and 2 */
        
        /* Anti-dependency within iteration */
        double temp = data[i];               /* Read */
        data[i] = temp * 0.5;                /* Write (WAR) */
        
        /* Control-dependent operation */
        if (data[i] > 100.0) {
            data[i] = 100.0;                 /* WAW potential */
        }
    }
}

int main() {
    const int N = 1024;
    
    /* Source arrays with simple patterns */
    double src[N], coeff[N], dest[N], alt[N];
    
    /* Initialize with deterministic values */
    for (int i = 0; i < N; ++i) {
        src[i] = (i % 10) * 1.5;
        coeff[i] = 1.0 + (i % 7) * 0.1;
        dest[i] = i * 0.5;
        alt[i] = i * 0.3;
    }
    
    /* Process data with complex dependencies */
    process_data(src, coeff, dest, alt, N);
    
    /* Second processing with overlapping dependencies */
    process_with_overlap(dest, N);
    
    /* Use volatile sink to prevent dead code elimination */
    volatile double sink = 0.0;
    for (int i = 0; i < 10; ++i) {
        sink += dest[i] + alt[i];
    }
    
    return (int)sink % 256;
}
