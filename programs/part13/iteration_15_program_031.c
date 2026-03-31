/* Complex loop to trigger DDG edge creation in GCC scheduler */
#include <math.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Global arrays to prevent complete optimization */
double global_src[2048];
double global_dest[2048];
double global_coeff[2048];

/* Function with complex loop to build detailed DDG */
void process_loop(int start, int end, int offset, double *restrict result) {
    /* Local arrays with potential aliasing */
    double src[1024];
    double dest[1024];
    double coeff[1024];
    double temp[1024];
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        src[i] = (i % 256) * 0.01;
        coeff[i] = (i % 128) * 0.02 + 0.5;
        dest[i] = (i % 64) * 0.03;
        temp[i] = 0.0;
    }
    
    /* Volatile to prevent certain optimizations */
    volatile double acc = 1.0;
    volatile double mod_acc = 0.0;
    
    /* Pointer aliasing - same data accessed through different pointers */
    double *alias1 = dest;
    double *alias2 = dest + 512;
    
    /* Main loop with complex dependencies */
    for (int i = start; i < end; i++) {
        /* 1. TRUE DEPENDENCY (RAW) with high-latency operation */
        double val1 = src[i] * coeff[i];          /* Read src, coeff */
        double val2 = external_func(val1);        /* Function call latency */
        double val3 = sqrt(fabs(val2) + 1.0);     /* FP sqrt latency */
        
        /* 2. LOOP-CARRIED DEPENDENCY (distance > 0) */
        acc = acc + val3;                         /* Loop-carried RAW */
        
        /* 3. ANTI-DEPENDENCY (WAR) */
        double old_val = alias1[i - 1];           /* Read before write */
        alias1[i] = acc + old_val;                /* Write after read */
        
        /* 4. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 16 == 0) {
            mod_acc = 1.0;                        /* First write */
        } else if (i % 8 == 0) {
            mod_acc = external_func(mod_acc);     /* Second write to same var */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + (int)(coeff[i % 4] * 10) + offset;
        if (idx < 1024 && idx >= 0) {
            temp[idx] = src[i] + mod_acc;         /* May alias with other accesses */
        }
        
        /* 6. ANOTHER LOOP-CARRIED with memory */
        if (i > 1) {
            /* Read two iterations back, write current */
            alias2[i] = alias2[i - 2] * 0.9 + src[i];
        }
        
        /* 7. CONTROL DEPENDENCY affecting multiple operations */
        double conditional_result;
        if (src[i] > 0.5) {
            conditional_result = src[i] / coeff[i];  /* FP division latency */
        } else {
            conditional_result = src[i] * coeff[i];
        }
        
        /* 8. MIXED INTEGER/FLOATING POINT with dependencies */
        int int_val = (int)(conditional_result * 100);
        double final_val = (double)int_val / 50.0;   /* Integer to float conversion */
        
        /* 9. ANOTHER OUTPUT DEPENDENCY chain */
        double chain = final_val;
        chain = chain + src[i];                     /* WAW on chain */
        chain = external_func(chain);               /* Another WAW */
        
        /* Store to global to prevent elimination */
        if (i % 128 == 0) {
            global_src[i] = src[i];
            global_dest[i] = dest[i];
            global_coeff[i] = coeff[i];
        }
    }
    
    /* Use results to prevent dead code elimination */
    *result = acc + mod_acc + dest[end-1] + temp[end-2];
}

/* Secondary loop with different pattern */
void process_loop2(int n, double *A, double *B, double *C) {
    double acc1 = 0.0, acc2 = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Cross-iteration dependencies */
        double t1 = A[i-1] * B[i];      /* RAW on A[i-1] from previous iteration */
        double t2 = C[i] / (B[i] + 1.0); /* FP division latency */
        
        /* WAR dependency */
        double old_A = A[i];            /* Read A[i] */
        A[i] = t1 + t2;                 /* Write A[i] - anti-dependency */
        
        /* WAW dependency with condition */
        if (i % 3 == 0) {
            acc1 = old_A * 2.0;         /* Write acc1 */
        } else {
            acc1 = old_A * 3.0;         /* Another write to acc1 */
        }
        
        /* Loop-carried output dependency */
        acc2 = acc1 + acc2;             /* RAW on acc2 from prev iteration */
        
        /* Complex addressing */
        int idx = (i * 7) % n;
        B[idx] = acc2 + C[i];
        
        /* Memory aliasing - A and C might overlap */
        C[i] = A[i % (n/2)] * 0.5;
    }
}

int main() {
    double result1, result2, result3;
    
    /* Initialize global arrays */
    for (int i = 0; i < 2048; i++) {
        global_src[i] = sin(i * 0.01);
        global_coeff[i] = cos(i * 0.005);
    }
    
    /* Process with different parameters to create varied DDG edges */
    process_loop(1, 1000, 3, &result1);
    process_loop(2, 500, -2, &result2);
    
    /* Local arrays for second loop */
    double A[1024], B[1024], C[1024];
    for (int i = 0; i < 1024; i++) {
        A[i] = i * 0.01;
        B[i] = i * 0.02;
        C[i] = i * 0.03;
    }
    
    process_loop2(1024, A, B, C);
    
    /* Volatile sink to prevent optimization */
    volatile double sink = result1 + result2 + A[10] + B[20] + C[30];
    
    /* Use globals to prevent removal */
    sink += global_src[100] + global_dest[200] + global_coeff[300];
    
    return (int)sink % 256;
}
