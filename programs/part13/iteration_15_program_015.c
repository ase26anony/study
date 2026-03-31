/* test_ddg.c - Complex loop to trigger DDG edge creation */
#include <math.h>

/* Non-inlineable function to create variable latency */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Complex loop with multiple dependency types */
void process_data(double* restrict dest, 
                  const double* src1, 
                  const double* src2,
                  double* shared,  /* Non-restrict for aliasing */
                  int size, 
                  int offset) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double temp_reg;
    
    /* Loop with complex dependencies */
    for (int i = 1; i < size; ++i) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        double val = src1[i] * src2[i];
        acc = acc / (val + 0.1);  /* Floating division - variable latency */
        
        /* 2. Anti-dependency (WAR) through shared memory */
        temp_reg = shared[i-1];           /* Read */
        shared[i] = acc + temp_reg;       /* Write - creates WAR */
        
        /* 3. Output dependency (WAW) on acc */
        if (i % 16 == 0) {
            acc = external_func(acc);     /* Function call - variable latency */
        }
        
        /* 4. Complex addressing with potential aliasing */
        int idx = i + offset + (int)(src1[i] * 0.01);
        if (idx < size && idx >= 0) {
            dest[idx] = shared[i] * 2.0;  /* May alias with shared[] */
        }
        
        /* 5. Control flow inside loop */
        if (src1[i] > 0.5) {
            /* Another loop-carried dependency */
            acc = sqrt(fabs(acc)) + 0.1;  /* sqrt - high latency */
        } else {
            /* Output dependency on temp_reg */
            temp_reg = src2[i] * 0.5;
        }
        
        /* 6. Memory operation with complex index */
        dest[i] = temp_reg + (dest[i-1] * 0.8);  /* Loop-carried through dest */
    }
    
    /* Use results to prevent elimination */
    sink = acc + dest[size/2] + shared[size/4];
}

/* Helper to create data dependencies across function boundaries */
double process_with_recurrence(double* data, int n) {
    double sum = 0.0;
    for (int i = 1; i < n; ++i) {
        /* Strong loop-carried dependency chain */
        double x = data[i] + 0.1;
        sum = sum / (x + 1.0);      /* Division - variable latency */
        data[i] = sum + data[i-1];  /* Loop-carried through array */
        
        /* Conditional with anti-dependency */
        double old = data[i];       /* Read */
        if (sum > 100.0) {
            sum = 0.0;              /* Output dependency on sum */
        }
        data[i] = old * 0.9;        /* Write - WAR through data[i] */
    }
    return sum;
}

/* Main function with multiple loops */
int main() {
    const int SIZE = 512;
    double src1[SIZE], src2[SIZE], dest[SIZE], shared[SIZE];
    volatile double result;
    
    /* Initialize with pattern */
    for (int i = 0; i < SIZE; ++i) {
        src1[i] = (i % 37) * 0.1;
        src2[i] = (i % 41) * 0.07;
        dest[i] = 0.0;
        shared[i] = (i % 23) * 0.05;
    }
    
    /* Process with complex dependencies */
    process_data(dest, src1, src2, shared, SIZE, 3);
    
    /* Second loop with different dependency pattern */
    double sum = process_with_recurrence(dest, SIZE);
    
    /* Third loop: mixed integer/floating point */
    int int_data[SIZE];
    double fp_data[SIZE];
    for (int i = 0; i < SIZE; ++i) {
        int_data[i] = i * 2;
    }
    
    for (int i = 1; i < SIZE; ++i) {
        /* Integer division - variable latency */
        int divisor = (int_data[i-1] % 7) + 1;
        int_data[i] = int_data[i] / divisor;  /* Integer division */
        
        /* Floating with dependency on integer */
        fp_data[i] = sqrt(int_data[i] * 1.0);
        
        /* Cross-type anti-dependency */
        double temp = fp_data[i-1];
        int_data[i] = (int)(temp * 10.0);
        fp_data[i] = temp * 0.9;  /* WAR through fp_data */
    }
    
    /* Use all results */
    result = sum + dest[SIZE/3] + fp_data[SIZE/2] + int_data[SIZE/4];
    
    return (result > 0.0) ? 0 : 1;
}
