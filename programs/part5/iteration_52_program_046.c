/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define AGGRESSIVE 1
#else
#define AGGRESSIVE 0
#endif

/* External function to create opaque values */
extern int get_opaque_value(int seed);

/* Stress function with complex register patterns */
int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 1;
    volatile int v4 = seed - 1;
    
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Opaque function call creates unanalyzable value */
        int opaque = get_opaque_value(seed + i);
        
        /* Long dependency chain with volatile accesses */
        int t1 = v1 * opaque + v2 / (opaque ? opaque : 1);
        int t2 = t1 - v3 % (opaque + 1);
        int t3 = t2 * v4 - opaque;
        int t4 = t3 + (v1 ^ v2) | (v3 & v4);
        
        /* Multi-use temporary value */
        int base = t4 * opaque - v1;
        
        /* Use base in multiple separated contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result += base / 2;
        } else {
            result += base + opaque;
        }
        
        /* Address computation with multiple offsets */
        int array[10];
        for (int j = 0; j < 5; j++) {
            /* Base address computation that might be rematerialized */
            int* ptr = &array[j];
            result += *(ptr + 0) + *(ptr + 1) + *(ptr + 2);
        }
        
#if AGGRESSIVE
        /* Inline assembly clobbering registers */
        asm volatile (
            "# Force register clobbering\n"
            : 
            : "r"(result), "r"(opaque)
            : "r0", "r1", "r2", "r3", "memory"
        );
#endif
        
        /* Volatile update prevents optimization */
        v1 = result % 100;
        v2 = (v2 + 1) % 200;
    }
    
    return result;
}

/* Another stress function with different pattern */
int stress_computation2(int seed, int n) {
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = seed * 0.75;
    double result = 0.0;
    
    /* Floating-point complex expressions */
    for (int i = 0; i < n; i++) {
        double opaque = get_opaque_value(seed + i) * 0.01;
        
        /* Complex FP chain */
        double t1 = vd1 * opaque + vd2 / (opaque + 0.001);
        double t2 = t1 - vd1 * vd2;
        double t3 = t2 * opaque - vd1 / vd2;
        double t4 = t3 + opaque * opaque - vd1 * vd2;
        
        /* Multi-use temporary in switch */
        double base = t4 * 2.0 - opaque;
        
        switch (i % 4) {
            case 0:
                result += base * 1.5;
                break;
            case 1:
                result += base / 1.5;
                break;
            case 2:
                result += base + opaque;
                break;
            case 3:
                result += base - opaque;
                break;
        }
        
        /* Prevent optimization with volatile */
        vd1 = result * 0.9;
        vd2 = result * 1.1;
    }
    
    return (int)result;
}

/* Function to provide opaque values */
int get_opaque_value(int seed) {
    /* Use system time to prevent compile-time evaluation */
    static int counter = 0;
    return (seed * 1103515245 + 12345 + counter++) % 1000;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 12345;
    
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;
    }
    
    srand(seed);
    
    int total = 0;
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < 5; i++) {
        total += stress_computation(seed + i * 100, iterations);
        total += stress_computation2(seed + i * 200, iterations / 2);
        
        /* Additional call site with different parameters */
        if (i % 2 == 0) {
            total += stress_computation(total, 10);
        } else {
            total += stress_computation2(total, 10);
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
