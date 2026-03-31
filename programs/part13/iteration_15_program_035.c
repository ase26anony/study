/* test_ddg_edge_creation.c
 * Complex loop with multiple dependency types to trigger DDG edge creation
 */

#include <math.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Global arrays to prevent optimizations */
double global_src[1024];
double global_dest[1024];
double global_coeff[1024];

/* Complex addressing with potential aliasing */
void process_loop(int start, int end, int offset1, int offset2) {
    volatile double sink; /* Prevent dead code elimination */
    double acc = 1.0;
    double temp_reg;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 1024; i++) {
        global_src[i] = (i % 3 == 0) ? i * 1.5 : i * 0.5;
        global_coeff[i] = (i % 5 == 0) ? 2.0 : 1.0;
    }
    
    /* Main target loop with complex dependencies */
    for (int i = start; i < end; i++) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        acc = acc + global_src[i] * global_coeff[i];
        
        /* 2. High-latency operation (division) */
        double div_result = global_src[i] / (global_coeff[i] + 1.0);
        
        /* 3. Anti-dependency (WAR) - read then write same location */
        temp_reg = global_dest[i-1];  /* Read */
        global_dest[i] = acc + div_result + temp_reg;  /* Write */
        
        /* 4. Output dependency (WAW) with control flow */
        if (i % 8 == 0) {
            acc = external_func(acc);  /* Function call with latency */
        }
        
        /* 5. Complex addressing with potential aliasing */
        int idx = i + offset1 + (int)(global_coeff[i % 4]);
        if (idx < 1024) {
            global_dest[idx] = global_src[i] * 2.0;  /* May alias with previous writes */
        }
        
        /* 6. Another loop-carried dependency */
        global_coeff[i] = global_coeff[i-1] * 0.9;
        
        /* 7. Control flow creating control dependencies */
        if (global_src[i] > 100.0) {
            temp_reg = sqrt(global_src[i]);  /* High latency sqrt */
            global_dest[i] = temp_reg;
        }
    }
    
    /* Use volatile to prevent optimization */
    sink = acc + global_dest[10] + global_dest[100];
}

/* Second loop with different pattern to ensure coverage */
void process_loop2(double* arr1, double* arr2, double* arr3, int n) {
    double sum1 = 0.0, sum2 = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* Multiple interleaved dependencies */
        double t1 = arr1[i-1];      /* RAW from previous iteration */
        double t2 = arr2[i] + t1;   /* RAW within iteration */
        
        /* WAW on sum1 */
        if (i % 3 == 0) {
            sum1 = t1 + t2;
        } else {
            sum1 = t1 - t2;
        }
        
        /* WAR on arr3 */
        double old_val = arr3[i];   /* Read */
        arr3[i] = sum1 + old_val;   /* Write */
        
        /* Complex addressing */
        int idx = (i * 7) % n;
        arr2[idx] = arr1[i] * arr3[i-1];
        
        /* Integer division with variable divisor (high latency) */
        if (arr1[i] != 0.0) {
            int divisor = (int)fabs(arr1[i]) % 10 + 1;
            arr1[i] = (double)i / divisor;
        }
    }
    
    volatile double vsink = sum1 + arr2[5] + arr3[5];
}

int main() {
    double local_arr1[256];
    double local_arr2[256];
    double local_arr3[256];
    
    /* Initialize local arrays */
    for (int i = 0; i < 256; i++) {
        local_arr1[i] = i * 0.25;
        local_arr2[i] = i * 0.5;
        local_arr3[i] = i * 0.75;
    }
    
    /* Call first processing function */
    process_loop(1, 512, 2, 3);
    
    /* Call second processing function */
    process_loop2(local_arr1, local_arr2, local_arr3, 256);
    
    /* Additional volatile use */
    volatile double final_sink = global_dest[50] + local_arr1[50];
    
    return 0;
}
