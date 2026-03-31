/* test_ddg.c - Complex loop to trigger DDG edge creation in GCC */

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex addressing function to confuse alias analysis */
static int get_offset(int i, int *offsets) __attribute__((noinline));
static int get_offset(int i, int *offsets) {
    return offsets[i & 3];
}

void process_loop(double *restrict result, 
                  const double *src1, 
                  const double *src2,
                  int n, 
                  int *offsets) {
    /* Loop-carried accumulator with output dependency */
    double acc = 1.0;
    /* Anti-dependency variable */
    double prev = 0.0;
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double temp = src1[i] * src2[i];      /* RAW: src1/src2 -> temp */
        temp = external_func(temp);           /* Function call latency */
        
        /* 2. LOOP-CARRIED DEPENDENCY (distance > 0) */
        acc = acc + temp;                     /* RAW with distance=1 */
        
        /* 3. ANTI-DEPENDENCY (WAR) through memory */
        double read_prev = result[i-1];       /* Read before write */
        result[i] = acc + read_prev;          /* Write after read */
        
        /* 4. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 16 == 0) {
            acc = 0.5;                        /* WAW: overwrites acc */
        }
        
        /* 5. Complex addressing to force conservative alias analysis */
        int idx = i + get_offset(i, offsets);
        if (idx < n) {
            /* Potential aliasing with result[i] */
            result[idx] = src1[i] * 0.25;     /* May alias with result[i] */
        }
        
        /* 6. High latency floating point operation */
        prev = src2[i] / (acc + 1.0);         /* Division latency */
        
        /* 7. More loop-carried dependency with distance */
        if (i > 2) {
            result[i] += result[i-2] * 0.1;   /* Distance=2 dependency */
        }
        
        /* 8. Control flow creating different execution paths */
        double val = (i % 8 == 0) ? prev * 2.0 : prev * 0.5;
        result[i] += val;
    }
    
    /* Volatile sink to prevent dead code elimination */
    volatile double sink = acc + result[n-1];
}

/* Second loop with different patterns */
void process_loop2(double *a, double *b, double *c, int n) {
    /* Multiple overlapping dependencies */
    for (int i = 3; i < n; ++i) {
        /* Chain of true dependencies */
        double t1 = a[i] + b[i-1];
        double t2 = t1 * c[i];
        double t3 = t2 / (b[i] + 1.0);    /* Division latency */
        
        /* Anti-dependency chain */
        double old_a = a[i];              /* Read a[i] */
        a[i] = t3 + old_a;                /* Write a[i] after read */
        
        /* Output dependency with branching */
        if (t3 > 0) {
            b[i] = t1;
        } else {
            b[i] = t2;                    /* WAW on b[i] in different paths */
        }
        
        /* Loop-carried with variable distance */
        c[i] = c[i-1] + c[i-2] * 0.3;     /* Distance=1 and 2 dependencies */
    }
    
    volatile double sink2 = a[n-1] + b[n-1];
}

int main() {
    const int N = 1024;
    double src1[N], src2[N], result[N];
    double array1[N], array2[N], array3[N];
    int offsets[4] = {0, 1, -1, 2};
    
    /* Initialize with simple patterns */
    for (int i = 0; i < N; ++i) {
        src1[i] = i * 0.1;
        src2[i] = i * 0.05 + 1.0;
        array1[i] = i * 0.2;
        array2[i] = i * 0.15;
        array3[i] = i * 0.25;
    }
    
    /* Execute loops with complex dependencies */
    process_loop(result, src1, src2, N, offsets);
    process_loop2(array1, array2, array3, N);
    
    /* Use results to prevent optimization */
    volatile double final_result = result[N/2] + array1[N/2];
    
    return (int)final_result;
}
