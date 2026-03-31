/* test_ddg.c - Complex loop to trigger DDG edge creation */

#include <math.h>
#include <stdlib.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex loop with multiple dependency types */
void process_data(double* restrict dest, 
                  const double* src1, 
                  const double* src2,
                  double* shared_buf,  /* Non-restrict for aliasing */
                  int size,
                  int offset) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double temp_reg = 0.0;
    
    /* Loop with complex dependencies */
    for (int i = 1; i < size; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val = src1[i] * src2[i];      /* RAW: src1/src2 -> val */
        val = external_func(val);            /* Function call latency */
        val = sqrt(fabs(val));               /* High latency sqrt */
        
        /* 2. LOOP-CARRIED DEPENDENCY (distance > 0) */
        acc = acc + val * 0.5;               /* Recurrence: acc_i depends on acc_{i-1} */
        
        /* 3. ANTI-DEPENDENCY (WAR) via aliased memory */
        double prev = shared_buf[i-1];       /* Read before write */
        shared_buf[i] = acc + prev;          /* Write after read - WAR */
        
        /* 4. OUTPUT DEPENDENCY (WAW) */
        temp_reg = acc * 2.0;                /* First write to temp_reg */
        if (i % 16 == 0) {
            temp_reg = 1.0 / (double)(i+1);  /* Second write to temp_reg - WAW */
        }
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + offset + (int)temp_reg;
        if (idx >= 0 && idx < size) {
            dest[idx] = src1[i] + temp_reg;  /* May alias with shared_buf */
        }
        
        /* 6. CONTROL DEPENDENCY */
        if (acc > 100.0) {
            acc = acc * 0.9;                 /* Control-dependent computation */
        } else if (acc < 0.0) {
            acc = 0.0;
        }
        
        /* 7. ANOTHER LOOP-CARRIED DEPENDENCY with different distance */
        if (i >= 3) {
            dest[i] = dest[i-3] * 0.8;       /* Distance 3 recurrence */
        }
    }
    
    /* Volatile sink to prevent dead code elimination */
    sink = acc + dest[size/2] + shared_buf[size/4];
}

/* Helper to initialize data */
void init_data(double* arr, int size, double seed) {
    for (int i = 0; i < size; ++i) {
        arr[i] = (double)(i * 1.5 + seed);
    }
}

int main() {
    const int SIZE = 512;
    double* src1 = (double*)malloc(SIZE * sizeof(double));
    double* src2 = (double*)malloc(SIZE * sizeof(double));
    double* dest = (double*)malloc(SIZE * sizeof(double));
    double* shared = (double*)malloc(SIZE * sizeof(double));
    
    /* Initialize with deterministic but non-trivial patterns */
    init_data(src1, SIZE, 1.0);
    init_data(src2, SIZE, 2.0);
    init_data(dest, SIZE, 3.0);
    init_data(shared, SIZE, 4.0);
    
    /* Run multiple times with different offsets to explore different paths */
    for (int offset = 0; offset < 4; ++offset) {
        process_data(dest, src1, src2, shared, SIZE, offset);
    }
    
    /* Final volatile sink */
    volatile double final_sink = dest[SIZE-1] + shared[SIZE-1];
    
    free(src1);
    free(src2);
    free(dest);
    free(shared);
    
    return (int)final_sink % 256;
}
