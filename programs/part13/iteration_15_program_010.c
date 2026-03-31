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
double global_temp[2048];

/* Function with complex loop to build detailed DDG */
void process_loop(int start, int end, int offset1, int offset2) {
    volatile double sink; /* Prevent dead code elimination */
    double acc = 1.0;
    double local_acc = 0.0;
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < 2048; i++) {
        global_src[i] = (i % 100) * 0.01;
        global_coeff[i] = (i % 50) * 0.02;
        global_temp[i] = (i % 25) * 0.04;
    }
    
    /* Main target loop with complex dependencies */
    for (int i = start + 1; i < end; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high-latency operation */
        double val = global_src[i] * global_coeff[i];  /* Source 1 */
        val = sqrt(val);  /* High latency sqrt - depends on previous */
        
        /* 2. LOOP-CARRIED DEPENDENCY with distance > 0 */
        acc = acc + val * 0.5;  /* Recurrence: depends on acc from previous iteration */
        
        /* 3. ANTI-DEPENDENCY (WAR) through array access */
        double prev = global_dest[i-1];  /* Read dest[i-1] */
        global_dest[i] = acc + prev;     /* Write dest[i] - WAR with next iteration's read */
        
        /* 4. OUTPUT DEPENDENCY (WAW) */
        if (i % 16 == 0) {
            acc = external_func(acc);  /* Function call with latency */
            /* WAW: acc is written here and also in the loop-carried dependency above */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + offset1 + (int)global_temp[i % 4];
        if (idx < 2048) {
            global_dest[idx] = global_src[i] + offset2;  /* May alias with other dest accesses */
        }
        
        /* 6. CONTROL FLOW creating control dependencies */
        if (global_src[i] > 0.5) {
            local_acc += global_dest[i] * 2.0;  /* Control-dependent computation */
        } else {
            local_acc -= global_temp[i] * 1.5;  /* Alternative path */
        }
        
        /* 7. INTEGER DIVISION with variable divisor (high latency) */
        if (offset2 != 0) {
            int divisor = (i % 8) + 1;  /* Non-constant divisor */
            int quotient = i / divisor;  /* Integer division with latency */
            global_temp[i % 256] = quotient * 0.01;  /* Use result */
        }
    }
    
    /* Prevent elimination of computations */
    sink = acc + local_acc + global_dest[10] + global_temp[100];
    (void)sink;
}

/* Secondary loop with different pattern */
void process_loop2(int n, int* offsets) {
    double acc1 = 0.0, acc2 = 1.0;
    volatile double sink;
    
    for (int i = 2; i < n; i++) {
        /* Multiple interleaved recurrences */
        acc1 = acc1 + global_src[i] * 0.3;
        acc2 = acc2 * 0.95 + global_coeff[i];
        
        /* Cross-iteration anti-dependency */
        double temp = global_dest[i - offsets[i % 4]];
        global_dest[i] = temp + acc1 - acc2;
        
        /* Conditional with both paths having dependencies */
        if (i % 3 == 0) {
            acc1 = sqrt(acc1);  /* High latency in one path */
        } else {
            acc2 = acc2 / (global_src[i] + 1.0);  /* Division in other path */
        }
        
        /* Write to multiple potentially overlapping locations */
        global_temp[i] = acc1;
        global_temp[i + 1] = acc2;  /* Overlap creates dependencies */
    }
    
    sink = acc1 + acc2;
    (void)sink;
}

/* Main function to drive execution */
int main() {
    int offsets[4] = {1, 2, 1, 3};
    
    /* Process with different parameters to create varied DDG patterns */
    process_loop(0, 1024, 2, 3);
    process_loop(500, 1500, 1, 2);
    process_loop2(512, offsets);
    process_loop2(768, offsets);
    
    /* Final volatile store to ensure all computations complete */
    volatile double final_sink = 
        global_dest[100] + global_src[200] + global_temp[300];
    
    return (int)(final_sink * 0);
}
