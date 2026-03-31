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

/* Function with complex loop to build detailed DDG */
double process_loop(int start, int end, int offset1, int offset2) {
    volatile double sink; /* Prevent dead code elimination */
    double acc = 1.0;
    double temp_acc = 0.0;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 2048; i++) {
        global_src[i] = (i % 3 == 0) ? i * 1.5 : i * 0.5;
        global_coeff[i] = sin(i * 0.01) + 1.0;
    }
    
    /* Main target loop with complex dependencies */
    for (int i = start + 1; i < end; i++) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        double val = global_src[i] * global_coeff[i];
        acc = acc + val;  /* Distance = 1, latency depends on FP add */
        
        /* 2. Anti-dependency (WAR) via array access */
        double prev = global_dest[i - 1];  /* Read */
        global_dest[i] = acc + prev;       /* Write to same location later */
        
        /* 3. Output dependency (WAW) with control flow */
        if (i % 16 == 0) {
            acc = external_func(acc);  /* Function call with latency */
            temp_acc = acc;            /* WAW: overwrites temp_acc */
        } else {
            temp_acc = sqrt(fabs(acc)); /* High latency sqrt */
        }
        
        /* 4. Complex addressing with potential aliasing */
        int idx1 = i + offset1;
        int idx2 = i + offset2;
        if (idx1 < 2048 && idx2 < 2048) {
            /* Potential WAR: read then write to overlapping locations */
            double tmp = global_src[idx2];
            global_src[idx1] = tmp * temp_acc;
        }
        
        /* 5. Integer division with variable divisor (high latency) */
        if (global_coeff[i] > 1.5) {
            int divisor = (int)global_coeff[i] % 7 + 2;
            int int_result = i / divisor;  /* Integer division latency */
            global_dest[i] += int_result;
        }
        
        /* 6. Nested control flow creating control dependencies */
        for (int j = 0; j < 3; j++) {
            if ((i + j) % 8 == 0) {
                /* Create output dependency inside nested loop */
                temp_acc = j * 0.1;
            }
        }
    }
    
    /* Use volatile to prevent optimization */
    sink = acc + global_dest[10] + global_dest[100];
    return sink;
}

/* Second loop with different dependency pattern */
double process_loop2(int n, double* restrict out, double* in1, double* in2) {
    double sum = 0.0;
    double local_acc = 0.0;
    
    /* Without restrict on out/in1/in2, compiler must assume aliasing */
    for (int i = 1; i < n; i++) {
        /* Loop-carried dependency chain */
        local_acc = local_acc / (in1[i] + 1.0);  /* FP division latency */
        
        /* Anti-dependency: read then write to potentially aliased arrays */
        double t = out[i-1];
        out[i] = local_acc + t + in2[i];
        
        /* Output dependency with conditional */
        if (local_acc > 100.0) {
            sum = local_acc;  /* WAW on sum */
        } else {
            sum = sqrt(local_acc);  /* Alternative WAW on sum */
        }
        
        /* Complex array indexing */
        int idx = (i * 3) % n;
        out[idx] = sum + in1[i];
    }
    
    return sum;
}

int main() {
    double result1, result2;
    
    /* Process with different offsets to create varied addressing */
    result1 = process_loop(0, 1024, 3, 5);
    
    /* Second processing with aliasing arrays */
    double array1[1024], array2[1024], array3[1024];
    for (int i = 0; i < 1024; i++) {
        array1[i] = i * 0.25;
        array2[i] = cos(i * 0.02);
    }
    
    /* No restrict keyword - compiler must assume aliasing */
    result2 = process_loop2(512, array3, array1, array2);
    
    /* Volatile sink to ensure computation isn't optimized away */
    volatile double final_sink = result1 + result2 + array3[50];
    
    return (int)final_sink;
}
