/* Complex loop to trigger DDG edge creation in GCC scheduler */
#include <math.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Global arrays to prevent optimizations */
double global_src[2048];
double global_dest[2048];
double global_coeff[2048];
volatile int global_offset = 3;

void process_loop(int n, int offset1, int offset2) {
    double acc = 1.0;
    double *src = global_src;
    double *dest = global_dest;
    double *coeff = global_coeff;
    
    /* Initialize with simple pattern */
    for (int i = 0; i < n; i++) {
        src[i] = i * 0.1;
        coeff[i] = 0.5 + (i % 7) * 0.01;
    }
    
    /* Main loop with complex dependencies */
    for (int i = 1; i < n - 10; i++) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        double temp = src[i] * coeff[i];
        acc = acc + temp;  /* RAW: acc depends on previous iteration's acc */
        
        /* 2. High-latency operation (division) */
        double div_result = acc / (coeff[i] + 0.001);
        
        /* 3. Anti-dependency (WAR) through array access */
        double prev_val = dest[i - 1];  /* Read dest[i-1] */
        dest[i] = div_result + prev_val;  /* Write dest[i] - WAR with next iteration */
        
        /* 4. Output dependency (WAW) with control flow */
        if (i % 16 == 0) {
            acc = external_func(div_result);  /* WAW: overwrites acc */
        }
        
        /* 5. Complex addressing with potential aliasing */
        int idx = i + offset1 + (i % offset2);
        if (idx < n) {
            dest[idx] = src[i] * 2.0;  /* May alias with other dest accesses */
        }
        
        /* 6. Another loop-carried dependency through memory */
        src[i + 1] = src[i] * 0.99;  /* RAW: src[i+1] depends on src[i] */
        
        /* 7. Integer division with variable divisor (high latency) */
        int int_div = (i * 100) / (offset1 + 1);
        
        /* 8. Control flow creating different dependency paths */
        if (int_div % 5 == 0) {
            coeff[i] = sqrt(fabs(acc));  /* sqrt is high latency */
        } else {
            coeff[i] = acc * 0.5;
        }
        
        /* 9. Output dependency through pointer */
        double *ptr = &dest[i];
        *ptr = coeff[i] + src[i];  /* WAW: overwrites dest[i] */
        
        /* 10. Cross-iteration dependency with distance > 1 */
        if (i > 2) {
            dest[i] += dest[i - 2] * 0.1;  /* Distance-2 dependency */
        }
    }
    
    /* Volatile sink to prevent dead code elimination */
    volatile double sink = acc + dest[n/2] + src[n/3];
}

/* Second loop with different pattern to ensure coverage */
void process_loop2(int n, int *offsets) {
    double acc1 = 0.0, acc2 = 1.0;
    double *buf1 = global_src;
    double *buf2 = global_dest;
    
    for (int i = 2; i < n; i++) {
        /* Multiple accumulators with dependencies */
        double t1 = buf1[i] * buf2[i - 1];  /* RAW on buf2[i-1] */
        double t2 = buf1[i - 1] / (buf2[i] + 0.001);  /* RAW on buf1[i-1], anti on buf2[i] */
        
        /* WAR: Read then write same location through different paths */
        double old_buf1 = buf1[i];
        buf1[i] = t1 + t2;  /* Anti-dependency */
        
        /* WAW with condition */
        if (offsets[i % 4] > 0) {
            buf2[i] = acc1 * old_buf1;
        } else {
            buf2[i] = acc2 / (old_buf1 + 1.0);
        }
        
        /* Loop-carried dependencies on accumulators */
        acc1 = acc1 + buf1[i] * 0.3;
        acc2 = acc2 - buf2[i] * 0.2;
        
        /* High-latency operation in control flow */
        if (i % 8 == 0) {
            acc1 = sqrt(fabs(acc1));  /* High latency sqrt */
        }
        
        /* Complex addressing with global offset */
        int idx = i + global_offset;
        if (idx < n) {
            buf1[idx] = buf1[i] + buf2[i - 1];  /* Potential aliasing */
        }
    }
    
    volatile double sink2 = acc1 + acc2 + buf1[n/4] + buf2[n/4];
}

int main() {
    int n = 1024;
    int offsets[] = {1, -1, 2, -2};
    
    /* Process with different parameters to explore different DDG edges */
    process_loop(n, 2, 3);
    process_loop2(n, offsets);
    
    /* Additional small loop with different characteristics */
    double small_acc = 0.0;
    for (int i = 1; i < 100; i++) {
        /* All dependency types in compact form */
        double a = global_src[i];
        double b = global_src[i - 1];  /* RAW */
        global_src[i - 1] = a * b;     /* WAR (if src[i-1] read later) and WAW */
        double c = global_dest[i];     /* Anti if dest[i] written earlier */
        global_dest[i] = c + a;        /* Output dependency */
        small_acc += global_src[i] * global_dest[i];  /* Loop-carried */
        
        /* Control flow creating different edge types */
        if (i % 3 == 0) {
            small_acc = small_acc / (i + 1);  /* High latency division */
        }
    }
    
    volatile double final_sink = small_acc;
    
    return 0;
}
