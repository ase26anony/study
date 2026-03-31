/* test_ddg.c - Complex loop to trigger DDG edge creation */
#include <math.h>

/* Non-inlineable function to force latency modeling */
static double external_func(double x) __attribute__((noinline));
static double external_func(double x) {
    return x * 0.75;
}

/* Global arrays to prevent complete optimization */
double global_src[1024];
double global_dest[1024];
double global_coeff[1024];

/* Function with complex loop to build detailed DDG */
void process_loop(double* src, double* dest, double* coeff, int n, int offset) {
    volatile double sink; /* Prevent dead code elimination */
    
    /* Loop-carried accumulator with high-latency operations */
    double acc = src[0] * coeff[0];
    
    /* Main loop with multiple dependency types */
    for (int i = 1; i < n; ++i) {
        /* 1. TRUE DATA DEPENDENCY (RAW) with high latency */
        double val = src[i] * coeff[i];      /* Read src[i], coeff[i] */
        acc = acc + val;                     /* Depends on previous acc and val */
        
        /* 2. ANTI-DEPENDENCY (WAR) with memory aliasing */
        double temp = dest[i-1];             /* Read dest[i-1] */
        dest[i] = acc + temp;                /* Write dest[i] - WAR with previous read */
        
        /* 3. LOOP-CARRIED DEPENDENCY with distance > 0 */
        if (i > 2) {
            /* Cross-iteration dependency: uses value from 2 iterations ago */
            dest[i] += dest[i-2] * 0.5;      /* Distance=2 dependency */
        }
        
        /* 4. OUTPUT DEPENDENCY (WAW) with control flow */
        if (i % 8 == 0) {
            acc = external_func(acc);        /* WAW: overwrites acc */
        }
        
        /* 5. HIGH-LATENCY OPERATION (division) */
        if (coeff[i] != 0.0) {
            val = src[i] / coeff[i];         /* Variable latency division */
            dest[i] += val;
        }
        
        /* 6. COMPLEX ADDRESSING with potential aliasing */
        int idx = i + offset;
        if (idx < n) {
            /* This creates ambiguous dependencies - compiler must be conservative */
            dest[idx] = src[i] + dest[i];    /* Could alias with other dest accesses */
        }
        
        /* 7. FUNCTION CALL with latency */
        acc = external_func(acc);            /* Non-inlineable call */
        
        /* 8. FLOATING POINT SQRT for high latency */
        if (i % 16 == 0) {
            acc = sqrt(fabs(acc));           /* High latency sqrt operation */
        }
    }
    
    /* Volatile sink to prevent optimization */
    sink = acc + dest[n-1];
}

/* Second loop with integer operations for different latency modeling */
int int_loop(int* arr1, int* arr2, int* arr3, int n, int step) {
    int sum = 0;
    volatile int vsink;
    
    for (int i = 1; i < n; ++i) {
        /* Integer division with variable divisor (high latency) */
        int divisor = arr2[i] + 1;
        if (divisor != 0) {
            sum = sum / divisor;             /* Integer division latency */
        }
        
        /* Anti-dependency chain */
        int old_val = arr1[i];               /* Read arr1[i] */
        arr1[i] = sum + i;                   /* Write arr1[i] - WAR */
        
        /* Output dependency */
        if (i % 4 == 0) {
            sum = arr3[i];                   /* WAW on sum */
        }
        
        /* Loop-carried with pointer arithmetic */
        int* ptr = &arr1[i] + step;
        if (ptr < &arr1[n]) {
            *ptr = old_val * 2;              /* Complex memory access */
        }
        
        /* Control flow creating control dependencies */
        if (arr2[i] > 100) {
            sum += arr3[i-1];                /* Another loop-carried dep */
        } else {
            sum -= arr2[i];                  /* Different path */
        }
    }
    
    vsink = sum;
    return sum;
}

/* Initialize arrays with pattern */
void init_arrays(void) {
    for (int i = 0; i < 1024; ++i) {
        global_src[i] = (i % 10) * 1.5;
        global_coeff[i] = (i % 7) * 0.3 + 0.1;
        global_dest[i] = 0.0;
    }
}

int main(void) {
    double local_src[512];
    double local_dest[512];
    double local_coeff[512];
    
    int int_arr1[256];
    int int_arr2[256];
    int int_arr3[256];
    
    /* Initialize data */
    init_arrays();
    
    for (int i = 0; i < 512; ++i) {
        local_src[i] = sin(i * 0.1);
        local_coeff[i] = cos(i * 0.05);
        local_dest[i] = 0.0;
    }
    
    for (int i = 0; i < 256; ++i) {
        int_arr1[i] = i * 2;
        int_arr2[i] = i % 20;
        int_arr3[i] = 100 - i;
    }
    
    /* Process with different parameters to create varied DDGs */
    process_loop(global_src, global_dest, global_coeff, 1024, 3);
    process_loop(local_src, local_dest, local_coeff, 512, -2);
    
    int result = int_loop(int_arr1, int_arr2, int_arr3, 256, 2);
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile double final_check = global_dest[100] + local_dest[50] + result;
    
    return (final_check > 0) ? 0 : 1;
}
