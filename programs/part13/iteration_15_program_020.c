/* Complex loop with multiple dependency types to trigger DDG edge creation */
#include <math.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Function with complex addressing to inhibit alias analysis */
void process_data(double* restrict dest, double* src1, double* src2, 
                  double* coeff, int n, int offset) {
    volatile double sink; /* Prevent dead code elimination */
    
    /* Loop-carried accumulator with multiple dependency types */
    double acc = 1.0;
    double temp_acc = 0.0;
    
    /* Anti-dependency variable */
    double prev_val = 0.0;
    
    /* Create complex loop with mixed dependencies */
    for (int i = 1; i < n; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val1 = src1[i] * coeff[i];          /* Read src1 and coeff */
        double val2 = external_func(val1);         /* Function call latency */
        acc = sqrt(acc + val2);                    /* Loop-carried RAW + sqrt latency */
        
        /* 2. ANTI-DEPENDENCY (WAR) with memory aliasing */
        double read_before_write = dest[i-1];      /* Read dest[i-1] */
        dest[i] = acc + read_before_write;         /* Write dest[i] - WAR with previous iteration */
        
        /* 3. OUTPUT DEPENDENCY (WAW) */
        if (i % 16 == 0) {
            acc = 1.0;                             /* Overwrites acc - WAW with line 24 */
        }
        
        /* 4. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + (int)(coeff[i % 4] * offset);
        if (idx < n && idx > 0) {
            dest[idx] = src2[i] * 0.5;             /* May alias with other dest accesses */
        }
        
        /* 5. CONTROL DEPENDENCY with data flow */
        if (src1[i] > 0.5) {
            temp_acc += dest[i] * 2.0;             /* Control-dependent computation */
        } else {
            temp_acc -= dest[i] * 0.5;             /* Alternative path */
        }
        
        /* 6. ANOTHER LOOP-CARRIED DEPENDENCY with integer division */
        int divisor = (i % 7) + 2;                 /* Non-constant divisor */
        int int_result = (int)acc / divisor;       /* Integer division latency */
        prev_val = dest[i] * int_result;           /* Links iterations */
        
        /* 7. MEMORY DEPENDENCY CHAIN */
        src2[i] = src2[i-1] * 0.9 + src1[i];       /* Loop-carried memory dependency */
    }
    
    /* Use volatile to prevent optimization */
    sink = acc + temp_acc + prev_val + dest[10];
    (void)sink; /* Silence unused warning */
}

/* Helper to initialize arrays */
void init_arrays(double* arr1, double* arr2, double* coeff, int n) {
    for (int i = 0; i < n; ++i) {
        arr1[i] = sin(i * 0.1);
        arr2[i] = cos(i * 0.05);
        coeff[i] = 0.5 + (i % 10) * 0.1;
    }
}

int main() {
    const int N = 1024;
    
    /* Allocate arrays without restrict to allow aliasing */
    double* src1 = __builtin_alloca(N * sizeof(double));
    double* src2 = __builtin_alloca(N * sizeof(double));
    double* coeff = __builtin_alloca(N * sizeof(double));
    double* dest = __builtin_alloca(N * sizeof(double));
    
    /* Initialize with deterministic values */
    init_arrays(src1, src2, coeff, N);
    
    /* Initialize destination */
    for (int i = 0; i < N; ++i) {
        dest[i] = 0.0;
    }
    
    /* Process with complex dependencies */
    for (int iter = 0; iter < 10; ++iter) {
        /* Vary offset to affect addressing */
        int offset = (iter % 3) + 1;
        process_data(dest, src1, src2, coeff, N, offset);
    }
    
    /* Final volatile sink */
    volatile double final_sink = dest[N-1] + dest[0];
    (void)final_sink;
    
    return 0;
}
