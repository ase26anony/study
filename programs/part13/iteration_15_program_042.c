/* test_ddg_edge.c - Complex loop to trigger DDG edge creation */

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

void process_data(double* restrict result, 
                  const double* src1, 
                  const double* src2,
                  double* shared_mem,  /* Non-restrict for aliasing */
                  int n) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double temp_reg;
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val = src1[i] * src2[i];      /* RAW: src1/src2 -> val */
        acc = external_func(acc + val);      /* RAW: acc,val -> acc (loop-carried) */
        
        /* 2. ANTI-DEPENDENCY (WAR) with memory aliasing */
        temp_reg = shared_mem[i-1];          /* Read before write */
        shared_mem[i] = acc + temp_reg;      /* WAR: shared_mem[i-1] read, then shared_mem[i] written */
        
        /* 3. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 16 == 0) {
            acc = 0.5;                       /* WAW: Overwrites acc */
        }
        
        /* 4. Loop-carried memory dependency with complex addressing */
        int idx = i + get_offset(i);
        if (idx > 0 && idx < n) {
            /* Creates both RAW and potential WAR dependencies */
            result[idx] = result[idx-1] + src1[i];  /* Loop-carried: result[idx-1] from previous iteration */
        }
        
        /* 5. Integer division with variable divisor (high latency) */
        int divisor = (i & 31) + 1;
        int int_result = (i * 100) / divisor;  /* Variable latency integer division */
        
        /* 6. Floating point division (high latency) */
        if (src2[i] != 0.0) {
            double ratio = src1[i] / src2[i];  /* High latency FP division */
            temp_reg = ratio * 0.1;            /* RAW: ratio -> temp_reg */
        }
        
        /* 7. Control flow creating different execution paths */
        double conditional_result;
        if (int_result > 500) {
            conditional_result = acc * 2.0;    /* RAW: acc -> conditional_result */
        } else {
            conditional_result = acc * 0.5;    /* RAW: acc -> conditional_result */
        }
        
        /* 8. Another output dependency in memory */
        shared_mem[i % 32] = conditional_result;  /* WAW: Multiple writes to same location */
        
        /* Use volatile to prevent dead code elimination */
        sink = acc + temp_reg + conditional_result;
    }
    
    /* Final volatile sink */
    sink = acc + result[n-1];
}

/* Main function to set up data and call the processing loop */
int main() {
    const int N = 1024;
    double src1[N], src2[N], result[N], shared_mem[N];
    
    /* Initialize with pattern data */
    for (int i = 0; i < N; ++i) {
        src1[i] = (i % 37) * 0.1;
        src2[i] = (i % 41) * 0.05 + 0.01;  /* Ensure non-zero for division */
        result[i] = 0.0;
        shared_mem[i] = (i % 17) * 0.01;
    }
    
    /* Call the function with complex loop */
    process_data(result, src1, src2, shared_mem, N);
    
    /* Use results to prevent elimination */
    volatile double final_sink = result[N/2] + shared_mem[N/4];
    
    return (int)(final_sink * 0.0);  /* Return 0, but compiler doesn't know */
}
