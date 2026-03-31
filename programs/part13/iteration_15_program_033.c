/* test_ddg.c - Complex loop to trigger DDG edge creation */
#include <math.h>

/* Non-inlineable function to create latency */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex loop with multiple dependency types */
void process_data(double* restrict result, 
                  const double* data1, 
                  const double* data2,
                  double* shared_array,
                  int size,
                  int offset) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double temp_reg = 0.0;
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < size; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency */
        double val1 = data1[i] * 2.0;           /* Node A */
        double val2 = external_func(val1);      /* Node B depends on A (RAW) */
        acc = acc + val2;                       /* Node C depends on B (RAW) */
        
        /* 2. ANTI-DEPENDENCY (WAR) - read then write same location */
        double old_val = shared_array[i-1];     /* Node D: read */
        shared_array[i] = acc * old_val;        /* Node E: write (WAR with D) */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same variable */
        if (i % 16 == 0) {
            acc = sqrt(fabs(acc));              /* Node F: writes acc (WAW with C) */
        }
        
        /* 4. LOOP-CARRIED DEPENDENCY with distance > 0 */
        double prev = result[i-1];              /* Node G: read from prev iteration */
        result[i] = prev + data2[i] * acc;      /* Node H: loop-carried (distance=1) */
        
        /* 5. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + offset;
        if (idx < size) {
            shared_array[idx] = result[i] * 0.5; /* Complex memory op */
        }
        
        /* 6. INTEGER DIVISION with variable latency */
        if (i % 3 == 0) {
            int divisor = (i % 7) + 1;
            temp_reg = (double)i / divisor;     /* Variable latency division */
        }
        
        /* 7. CONTROL DEPENDENCY inside loop */
        double conditional_result;
        if (temp_reg > 10.0) {                  /* Control dependency */
            conditional_result = data1[i] * 3.0;
        } else {
            conditional_result = data2[i] * 2.0;
        }
        
        /* 8. MIXED TYPES creating complex DDG */
        result[i] += conditional_result * (i % 4);
    }
    
    /* Volatile sink to prevent dead code elimination */
    sink = acc + result[size-1] + shared_array[0];
}

/* Helper to prevent optimizations */
static void init_data(double* arr, int size) {
    for (int i = 0; i < size; ++i) {
        arr[i] = (double)(i % 100) * 0.01;
    }
}

int main() {
    const int SIZE = 512;
    double data1[SIZE], data2[SIZE];
    double result[SIZE] = {0};
    double shared[SIZE] = {0};
    
    /* Initialize with pattern */
    init_data(data1, SIZE);
    init_data(data2, SIZE);
    
    /* Multiple calls with different offsets to create varied patterns */
    for (int iter = 0; iter < 3; ++iter) {
        process_data(result, data1, data2, shared, SIZE, iter * 2);
    }
    
    /* Final volatile use */
    volatile double final_sink = result[SIZE/2] + shared[SIZE/4];
    
    return (int)(final_sink * 0.0); /* Prevent compiler from optimizing away */
}
