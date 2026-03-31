#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define SIZE 1024

/* Function to prevent compiler from optimizing away the loop */
static void escape(void *p) {
    asm volatile("" : : "g"(p) : "memory");
}

int main(int argc, char *argv[]) {
    double array_a[SIZE];
    double array_b[SIZE];
    double array_c[SIZE] = {0};
    double sum = 0.0;
    
    /* Initialize arrays with pseudo-random values */
    int seed = argc > 1 ? atoi(argv[1]) : 42;
    for (int i = 0; i < SIZE; i++) {
        array_a[i] = (double)((seed + i * 13) % 100) / 10.0 + 1.0;
        array_b[i] = (double)((seed + i * 17) % 100) / 10.0 - 5.0;
    }
    
    /* Small offset to create non-constant array access patterns */
    int offset = argc > 2 ? atoi(argv[2]) % 4 : 1;
    if (offset == 0) offset = 1;  /* Ensure non-zero */
    
    /* Volatile variable to prevent constant propagation */
    volatile int vol_offset = offset;
    
    /* The target innermost loop for modulo scheduling */
    for (int i = 0; i < SIZE; i++) {
        /* 1. High-latency operation: sqrt with floating-point division */
        double base_val;
        if (i + vol_offset < SIZE) {
            base_val = array_a[i + vol_offset];
        } else {
            base_val = array_a[i];
        }
        double x = sqrt(base_val + 1.0) / 1.234567;  /* Non-constant divisor */
        
        /* 2. Loop-carried reduction with distance-1 dependency */
        sum = sum * 0.987654 + x;  /* True recurrence: sum depends on previous iteration's sum */
        
        /* 3. Conditional store based on previous iteration */
        if (i > 0 && array_b[i-1] > 0.0) {  /* Distance-1 memory dependency */
            array_c[i] = x * sum;  /* Complex expression to prevent simplification */
        }
        
        /* 4. Additional memory access with potential aliasing */
        /* Access array_b with different pattern to create register pressure */
        double temp = array_b[i] * 0.5;
        if (i % 3 == 0) {
            array_a[i] = temp;  /* Write back to create WAR/WAW dependencies */
        }
        
        /* 5. Another high-latency operation to increase critical path */
        if (i % 5 == 0) {
            /* Use integer division with non-constant divisor */
            int int_val = (int)(fabs(x) * 1000);
            if (int_val != 0) {
                sum += 1.0 / (double)int_val;  /* High-latency integer-to-float division */
            }
        }
    }
    
    /* Prevent dead code elimination */
    escape(array_a);
    escape(array_b);
    escape(array_c);
    escape(&sum);
    
    printf("Result: %f\n", sum);
    return 0;
}
