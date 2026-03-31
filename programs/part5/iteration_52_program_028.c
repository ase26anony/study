/* Test program to stress early rematerialization with virtual register creation */
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
static int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 7;
    volatile int v4 = seed - 3;
    
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Opaque function call prevents constant propagation */
        int opaque = get_opaque_value(seed + i);
        
        /* Long dependency chain with volatile accesses */
        int t1 = v1 * opaque + v2 / (opaque ? opaque : 1);
        int t2 = t1 - v3 % (opaque + 1);
        int t3 = t2 * v4 + opaque;
        int t4 = t3 / (v1 + 1) - v2;
        int t5 = t4 % (v3 ? v3 : 1) + v4;
        
        /* Multi-use temporary value */
        int base = t5 * opaque - v1;
        
        /* Use base in multiple separated contexts */
        if (i % 3 == 0) {
            result += base * 2;
        } else if (i % 3 == 1) {
            result -= base / 2;
        } else {
            result ^= base;
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
            "# Clobber multiple registers\n"
            "mov r0, %0\n"
            "mov r1, %1\n"
            "add r0, r0, r1\n"
            : 
            : "r" (result), "r" (t5)
            : "r0", "r1", "memory"
        );
#endif
        
        /* Prevent loop unrolling with volatile */
        v1 += 1;
    }
    
    return result;
}

/* Another stress function with different patterns */
static int stress_computation2(int seed, int n) {
    volatile double vd1 = seed * 1.5;
    volatile double vd2 = seed * 0.75;
    int result = 0;
    
    /* Floating-point arithmetic creates different register pressure */
    for (int i = 0; i < n; i++) {
        double opaque = get_opaque_value(seed + i) * 1.0;
        
        /* Complex FP expression chain */
        double d1 = vd1 * opaque + vd2 / (opaque + 1.0);
        double d2 = d1 - vd1 * 0.5;
        double d3 = d2 * vd2 + opaque / 3.0;
        double d4 = d3 / (vd1 + 1.0) - vd2;
        
        /* Multi-use FP temporary */
        double fp_base = d4 * opaque - vd1;
        
        /* Use in different contexts */
        switch (i % 4) {
            case 0:
                result += (int)(fp_base * 100);
                break;
            case 1:
                result -= (int)(fp_base * 50);
                break;
            case 2:
                result |= (int)fp_base;
                break;
            case 3:
                result &= (int)fp_base;
                break;
        }
        
        /* More register pressure with nested loops */
        for (int j = 0; j < 3; j++) {
            int temp = result * j;
            result += temp / (j + 1);
            result ^= temp % 256;
        }
    }
    
    return result;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use system rand for unpredictability */
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    return rand() % 1000 + seed;
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
        if (iterations < 10) iterations = 10;
    }
    
    printf("Starting stress test with seed=%d, iterations=%d\n", seed, iterations);
    
    /* Call stress functions multiple times from different contexts */
    int total = 0;
    
    /* First call site */
    total += stress_computation(seed, iterations);
    
    /* Different input to prevent CSE */
    total += stress_computation(seed * 2, iterations / 2);
    
    /* Second stress function */
    total += stress_computation2(seed + 1, iterations);
    
    /* Another call with different parameters */
    total += stress_computation2(seed * 3, iterations / 3);
    
    /* Loop with varying parameters to create different register pressure */
    for (int i = 0; i < 5; i++) {
        total += stress_computation(seed + i * 100, iterations / (i + 2));
    }
    
    printf("Result: %d\n", total);
    
    /* Prevent dead code elimination */
    if (total == 0) {
        printf("Zero result detected\n");
    }
    
    return total != 0 ? 0 : 1;
}
