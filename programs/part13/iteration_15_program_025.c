/* Complex loop to trigger DDG edge creation in GCC scheduler */
#include <math.h>

/* Non-inlineable function to create variable latency */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Function with complex addressing to inhibit alias analysis */
void process_data(double* restrict result, const double* data, 
                  const double* coeffs, int size, int offset) {
    volatile double sink; /* Prevent optimizations */
    double acc = 1.0;
    double temp_array[1024];
    
    /* Initialize with some values */
    for (int i = 0; i < size; i++) {
        temp_array[i] = (double)i * 0.5;
    }
    
    /* Main loop with complex dependencies */
    for (int i = 1; i < size - 1; i++) {
        /* 1. Loop-carried true dependency (RAW) with high latency */
        double val = data[i] * coeffs[i];      /* Read data & coeffs */
        acc = acc / (val + 1.0);               /* Division - high latency */
        
        /* 2. Anti-dependency (WAR) with memory */
        double prev = temp_array[i - 1];       /* Read from array */
        temp_array[i] = acc + prev;            /* Write to same array */
        
        /* 3. Output dependency (WAW) */
        if (i % 16 == 0) {
            acc = external_func(acc);          /* Function call - variable latency */
        }
        
        /* 4. Complex addressing with potential aliasing */
        int idx = i + (offset % 4);
        result[idx] = temp_array[i] * 2.0;     /* Write to result */
        
        /* 5. Control flow inside loop */
        if (data[i] > 0.5) {
            /* Another output dependency */
            acc = sqrt(fabs(acc));             /* sqrt - high latency */
            result[idx] = result[idx] + 1.0;   /* Read-modify-write */
        } else {
            /* Anti-dependency through different path */
            double tmp = result[idx - 1];      /* Read previous result */
            temp_array[i] = tmp * 0.8;         /* Write to temp_array */
        }
        
        /* 6. Integer division with variable divisor (high latency) */
        int int_val = (int)(acc * 100);
        if (int_val != 0) {
            int divisor = (i % 7) + 1;         /* Non-constant divisor */
            int ratio = int_val / divisor;     /* Integer division */
            result[idx] = result[idx] + ratio;
        }
    }
    
    /* Use volatile to prevent dead code elimination */
    sink = acc + result[10] + temp_array[20];
}

/* Another function with pointer aliasing to force conservative analysis */
void aliasing_loop(double* a, double* b, int n) {
    /* a and b may alias - no restrict keyword */
    double sum = 0.0;
    
    for (int i = 1; i < n; i++) {
        /* True dependency with pointer arithmetic */
        double x = a[i - 1];           /* Read from a */
        b[i] = x + b[i];               /* Read and write to b (may alias with a) */
        
        /* Loop-carried dependency through sum */
        sum = sum + a[i] * 0.3;        /* RAW dependency on sum */
        
        /* Complex condition with control dependency */
        if (sum > 100.0) {
            a[i] = sum / (i + 1);      /* Division - high latency */
            sum = sum * 0.9;           /* Output dependency on sum */
        }
    }
    
    volatile double sink = sum;
}

int main() {
    const int SIZE = 512;
    double data[SIZE];
    double coeffs[SIZE];
    double result[SIZE * 2] = {0};  /* Extra space for complex indexing */
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        data[i] = sin(i * 0.1);
        coeffs[i] = cos(i * 0.05);
    }
    
    /* Process with different offsets to vary addressing */
    for (int offset = 0; offset < 4; offset++) {
        process_data(result, data, coeffs, SIZE, offset);
    }
    
    /* Another loop with potential aliasing */
    aliasing_loop(data, result, SIZE);
    
    /* Final volatile sink to prevent optimization */
    volatile double final_sink = result[SIZE/2] + data[SIZE/4];
    
    return (int)(final_sink * 0);
}
