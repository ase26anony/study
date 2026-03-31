/* Complex loop with multiple dependency types to trigger DDG edge creation */
#include <math.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Function with complex addressing to inhibit alias analysis */
void process_loop(double* restrict result, const double* data, 
                  int size, int offset, double* shared) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double temp_reg = 0.0;
    
    /* Loop with multiple dependency types */
    for (int i = 1; i < size; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency operation */
        double val = data[i] * 2.0;
        val = sqrt(val);  /* High latency operation */
        
        /* 2. LOOP-CARRIED DEPENDENCY (distance > 0) */
        acc = acc + val * 0.5;  /* Recurrence: acc depends on previous iteration's acc */
        
        /* 3. ANTI-DEPENDENCY (WAR) - read then write same location */
        double old_val = shared[i];  /* Read */
        shared[i] = acc + old_val;   /* Write - creates anti-dependency */
        
        /* 4. OUTPUT DEPENDENCY (WAW) */
        temp_reg = acc * 3.0;        /* First write to temp_reg */
        if (i % 16 == 0) {
            temp_reg = external_func(acc);  /* Second write to temp_reg - WAW */
        }
        
        /* 5. Complex addressing to confuse alias analysis */
        int idx = i + offset;
        if (idx >= size) idx = i;
        
        /* Mixed array accesses that may alias */
        result[idx] = temp_reg + shared[i-1];  /* Uses previous iteration's shared */
        
        /* 6. Integer division with variable divisor (high latency) */
        int divisor = (i % 8) + 2;
        offset = (offset * 10) / divisor;  /* Variable latency integer division */
        
        /* 7. Control flow inside loop */
        if (acc > 100.0) {
            acc = acc * 0.9;
            shared[i] = shared[i] * 0.5;  /* Another write to shared[i] */
        }
    }
    
    /* Use volatile to prevent dead code elimination */
    sink = acc + result[10] + shared[size-1];
}

/* Another function with pointer aliasing */
void aliasing_loop(double* a, double* b, double* c, int n) {
    /* a, b, c may alias - compiler must be conservative */
    double sum = 0.0;
    
    for (int i = 1; i < n; ++i) {
        /* Multiple reads/writes to potentially overlapping locations */
        double x = a[i];
        double y = b[i-1];
        
        /* Loop-carried output dependency */
        sum = sum / (1.0 + x);  /* Division - high latency */
        
        /* Write to potentially aliased locations */
        a[i] = sum + x;
        b[i] = sum - y;
        
        /* Access with variable offset */
        int j = (i * 3) % n;
        c[j] = c[j] + sum;  /* Read-modify-write with possible aliasing */
    }
    
    volatile double sink = sum + a[0] + b[0];
}

int main() {
    const int SIZE = 1024;
    double data[SIZE];
    double result[SIZE];
    double shared[SIZE];
    double array1[SIZE];
    double array2[SIZE];
    double array3[SIZE];
    
    /* Initialize with simple pattern */
    for (int i = 0; i < SIZE; ++i) {
        data[i] = (i % 100) * 0.01;
        shared[i] = (i % 50) * 0.02;
        array1[i] = i * 0.001;
        array2[i] = i * 0.002;
        array3[i] = i * 0.003;
    }
    
    /* Call loops with complex dependencies */
    process_loop(result, data, SIZE, 3, shared);
    aliasing_loop(array1, array2, array3, SIZE);
    
    /* Use results to prevent elimination */
    volatile double final_sink = result[SIZE/2] + array1[SIZE/4] + array2[SIZE/4];
    
    return (int)(final_sink * 0.0);  /* Return 0 without revealing computation */
}
