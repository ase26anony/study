/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_opaque_value(int seed);

/* Stress function with complex register patterns */
#ifdef __OPTIMIZE__
__attribute__((noinline))
#endif
int stress_computation(volatile int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 7;
    volatile int v4 = seed - 3;
    
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Opaque function call creates unanalyzable value */
        int opaque = get_opaque_value(v1 + i);
        
        /* Long dependency chain with volatile accesses */
        int t1 = v1 * opaque + v2 / (opaque ? opaque : 1);
        int t2 = t1 - v3 % (opaque + 1);
        int t3 = t2 * v4 + opaque / (v2 ? v2 : 1);
        int t4 = t3 - v1 % (v3 ? v3 : 1);
        int t5 = t4 * opaque + t2 / (v4 ? v4 : 1);
        
        /* Multi-use temporary value */
        int base = t5 * 31 - 17;
        
        /* Use base in multiple different contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result += base / 2;
        } else {
            result += base + opaque;
        }
        
        /* Inline assembly clobbering registers */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# Clobber multiple registers\n\t"
            : 
            : "r"(t5), "r"(base)
            : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
              "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
        #endif
        
        /* Address computation with multiple offsets */
        int array[8] = {0};
        int *ptr = &array[i % 8];
        
        /* Use base address with different offsets */
        *(ptr + 0) = base;
        *(ptr + 1) = base + 1;
        *(ptr + 2) = base * 2;
        *(ptr + 3) = base - 1;
        
        /* More complex arithmetic to increase register pressure */
        v1 = (v1 * 1103515245 + 12345) & 0x7fffffff;
        v2 = (v2 * 1664525 + 1013904223) & 0x7fffffff;
        v3 = (v3 * 134775813 + 1) & 0x7fffffff;
        v4 = (v4 * 214013 + 2531011) & 0x7fffffff;
    }
    
    return result;
}

/* Another stress function with different pattern */
#ifdef __OPTIMIZE__
__attribute__((noinline))
#endif
int stress_computation2(volatile int seed, int n) {
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = seed * 0.75;
    
    double result = 0.0;
    
    /* Floating-point complex expressions */
    for (int i = 0; i < n; i++) {
        double t1 = vd1 * vd2 + (double)seed / (vd1 + 0.001);
        double t2 = t1 - vd2 * (double)(i % 7);
        double t3 = t2 / (vd1 ? vd1 : 1.0) + vd2 * 3.14159;
        double t4 = t3 * t1 - t2 / (vd2 ? vd2 : 1.0);
        
        /* Multi-use temporary in switch */
        double base = t4 * 2.71828 - 1.41421;
        
        switch (i % 4) {
            case 0:
                result += base * 1.1;
                break;
            case 1:
                result += base / 1.1;
                break;
            case 2:
                result += base + t1;
                break;
            case 3:
                result += base - t2;
                break;
        }
        
        /* More register pressure */
        vd1 = vd1 * 1.1 - 0.5;
        vd2 = vd2 * 0.9 + 0.25;
        
        /* Additional arithmetic chain */
        for (int j = 0; j < 3; j++) {
            double temp = base * j + vd1 - vd2;
            result += temp / (j + 1);
        }
    }
    
    return (int)result;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use system time to prevent constant folding */
    static int counter = 0;
    return (seed * 1103515245 + 12345 + counter++) & 0x7fffffff;
}

int main(int argc, char *argv[]) {
    int n = 100;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0) n = 100;
        if (n > 1000) n = 1000; /* Limit to avoid excessive runtime */
    }
    
    srand(time(NULL));
    volatile int seed = rand();
    
    printf("Starting stress tests with n=%d, seed=%d\n", n, seed);
    
    /* Call stress functions multiple times from different contexts */
    int total = 0;
    
    /* First call site */
    total += stress_computation(seed, n);
    
    /* Modify seed */
    seed = seed * 3 + 7;
    
    /* Second call site with different parameters */
    total += stress_computation(seed, n / 2 + 1);
    
    /* Third call site with second stress function */
    total += stress_computation2(seed * 2, n / 3 + 1);
    
    /* Loop with varying parameters */
    for (int i = 0; i < 5; i++) {
        seed = get_opaque_value(seed + i);
        total += stress_computation(seed, n / 5 + 2);
    }
    
    printf("Result: %d\n", total);
    
    /* Prevent dead code elimination */
    if (total == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
