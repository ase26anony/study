/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define AGGRESSIVE_REGISTER_PRESSURE 1
#endif

/* External function to create opaque values */
extern int get_opaque_value(int seed);

/* Stress function with complex register pressure patterns */
int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 1;
    volatile int v4 = seed - 1;
    
    int result = 0;
    
    /* Complex arithmetic expression creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Prevent optimization with volatile loop bound */
        volatile int loop_bound = n;
        if (i >= loop_bound) break;
        
        /* Multi-use temporary value with complex computation */
        int base = v1 * v2 + v3 / (v4 + 1) - (v1 % (v2 + 1));
        base = base * base - base / 2 + base % 3;
        
        /* Use base in multiple, spatially separated contexts */
        int use1, use2, use3;
        
        /* First use - complex expression */
        use1 = base * v1 + v2 / (base + 1) - (v3 % (base + 2));
        use1 = use1 * use1 - use1 / 4 + use1 % 5;
        
        /* Opaque function call to prevent optimization */
        int opaque = get_opaque_value(seed + i);
        
        /* Second use - different expression */
        use2 = base + opaque * v4 - v1 / (base + 3);
        use2 = use2 * 3 - use2 / 6 + use2 % 7;
        
        /* Third use - in conditional context */
        if (opaque % 2 == 0) {
            use3 = base * 2 + v3 - v2 / (base + 4);
        } else {
            use3 = base / 2 + v4 - v1 / (base + 5);
        }
        use3 = use3 * 4 - use3 / 8 + use3 % 9;
        
        /* Combine all uses */
        result += use1 + use2 + use3;
        
        /* Address computation with multiple offsets */
        int array[10];
        for (int j = 0; j < 10; j++) {
            array[j] = j * j;
        }
        
        /* Multiple uses of computed address with different offsets */
        int *ptr = &array[i % 10];
        result += ptr[0] + ptr[1] - ptr[2] + ptr[3] - ptr[4];
        
        /* Inline assembly to clobber registers (when optimized) */
        #ifdef AGGRESSIVE_REGISTER_PRESSURE
        asm volatile (
            "# Clobber many registers to increase pressure\n\t"
            "mov r0, %0\n\t"
            "mov r1, %1\n\t"
            "mov r2, %2\n\t"
            : 
            : "r" (result), "r" (base), "r" (opaque)
            : "r0", "r1", "r2", "memory"
        );
        #endif
        
        /* More complex FP arithmetic to create FP virtual registers */
        volatile double d1 = result * 0.5;
        volatile double d2 = base * 0.25;
        double d3 = d1 * d2 + d1 / (d2 + 1.0) - fmod(d1, d2 + 2.0);
        result += (int)d3;
    }
    
    return result;
}

/* Another stress function with different pattern */
int stress_computation2(int seed, int n) {
    volatile long v1 = seed * 3L;
    volatile long v2 = seed * 5L;
    long result = 0;
    
    /* Switch statement with multi-use temporaries */
    for (int i = 0; i < n; i++) {
        volatile int mod = i % 4;
        
        /* Compute value once, use in multiple switch cases */
        long temp = v1 * v2 + (v1 % (v2 + 1)) - (v2 / (v1 + 1));
        temp = temp * temp - temp / 2 + temp % 3;
        
        switch (mod) {
            case 0: {
                /* Use temp in complex expression */
                long val = temp * 2 + v1 - v2 / (temp + 1);
                result += val % 1000;
                break;
            }
            case 1: {
                /* Different use of same temp */
                long val = temp / 2 + v2 - v1 / (temp + 2);
                result += val % 1000;
                break;
            }
            case 2: {
                /* Yet another use */
                long val = temp * 3 + (v1 + v2) / (temp + 3);
                result += val % 1000;
                break;
            }
            default: {
                /* Final use pattern */
                long val = temp / 3 + (v1 - v2) * (temp + 4);
                result += val % 1000;
                break;
            }
        }
        
        /* Additional register pressure */
        #ifdef AGGRESSIVE_REGISTER_PRESSURE
        asm volatile (
            "# More register clobbering\n\t"
            "mov r3, %0\n\t"
            "mov r4, %1\n\t"
            : 
            : "r" (result), "r" (temp)
            : "r3", "r4", "memory"
        );
        #endif
    }
    
    return (int)result;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use system rand() for true opacity */
    srand(seed);
    return rand();
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 42;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;
        if (iterations < 10) iterations = 10;
    }
    
    srand(seed);
    
    int total_result = 0;
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < 5; i++) {
        int n = 10 + (rand() % 20);
        
        /* Alternate between two different stress patterns */
        if (i % 2 == 0) {
            total_result += stress_computation(seed + i, n);
        } else {
            total_result += stress_computation2(seed + i, n);
        }
        
        /* Vary the input to prevent constant propagation */
        seed += total_result % 100;
    }
    
    /* Additional calls with different parameters */
    total_result += stress_computation(total_result, iterations % 50);
    total_result += stress_computation2(total_result, iterations % 30);
    
    printf("Result: %d\n", total_result % 1000000);
    
    return 0;
}
