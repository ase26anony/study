/* Test program to trigger virtual register creation in GCC's early rematerialization pass */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define AGGRESSIVE_ASM 1
#else
#define AGGRESSIVE_ASM 0
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
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Use volatile variables to force loads/stores */
        int a = v1 * v2 + get_opaque_value(i);
        int b = v3 / (v4 + 1) | (v1 & 0xFF);
        int c = (a % (b + 1)) ^ (v2 >> 2);
        
        /* Multi-use temporary value */
        int base = a * b - c + get_opaque_value(i + seed);
        
        /* Use base in multiple, spatially separated contexts */
        if (i & 1) {
            result += base * 2;
        } else {
            result -= base / 3;
        }
        
        /* Another use of base with different computation */
        result ^= (base << 2) | (base >> 30);
        
        /* Complex floating-point chain (if supported) */
        float f1 = (float)v1 * 1.5f;
        float f2 = (float)v2 * 0.75f;
        float f3 = f1 * f2 - (float)v3 / f1 + (float)v4;
        
        /* Use floating result in integer computation */
        result += (int)f3;
        
        /* Address computation with multiple offsets */
        int array[8];
        for (int j = 0; j < 8; j++) {
            array[j] = get_opaque_value(i + j);
        }
        
        /* Multiple uses of computed address-like values */
        int idx1 = (i * 7) % 8;
        int idx2 = (i * 3) % 8;
        int val1 = array[idx1] + base;
        int val2 = array[idx2] - base;
        
        result += val1 * val2;
        
        /* Aggressive inline assembly to clobber registers */
        #if AGGRESSIVE_ASM
        asm volatile (
            "# Clobber many registers\n"
            "mov r0, %0\n"
            "mov r1, %1\n"
            "add r2, r0, r1\n"
            : 
            : "r" (val1), "r" (val2)
            : "r0", "r1", "r2", "memory"
        );
        #endif
        
        /* Loop-carried dependency with volatile */
        v1 = v1 + 1;
        v2 = v2 - (i % 3);
        v3 = v3 ^ result;
        v4 = v4 | (i << 2);
    }
    
    return result;
}

/* Another stress function with different pattern */
int stress_memory_access(int seed, int n) {
    volatile int counters[16];
    for (int i = 0; i < 16; i++) {
        counters[i] = seed + i;
    }
    
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        /* Complex expression with many intermediate values */
        int t1 = counters[0] * counters[1];
        int t2 = counters[2] / (counters[3] + 1);
        int t3 = t1 % (t2 + 1);
        int t4 = counters[4] ^ counters[5];
        int t5 = t3 * t4 - counters[6];
        
        /* Chain of computations */
        int chain = t5;
        for (int j = 0; j < 4; j++) {
            chain = (chain * 1103515245 + 12345) & 0x7FFFFFFF;
            chain = chain ^ (counters[j] << j);
        }
        
        result += chain;
        
        /* Update volatiles in complex pattern */
        for (int j = 0; j < 16; j += 2) {
            counters[j] = counters[j] + (result >> (j % 8));
            counters[j + 1] = counters[j + 1] ^ (i * j);
        }
        
        /* Switch with multi-use temporaries */
        switch (i % 4) {
            case 0:
                result += t1 * 3;
                break;
            case 1:
                result -= t2 / 2;
                break;
            case 2:
                result ^= t3 | t4;
                break;
            case 3:
                result = (result + t5) & 0xFF;
                break;
        }
    }
    
    return result;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use system rand() for unpredictability */
    static int initialized = 0;
    if (!initialized) {
        srand(time(NULL));
        initialized = 1;
    }
    return rand() ^ (seed * 1103515245 + 12345);
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
    
    printf("Starting stress test with seed=%d, iterations=%d\n", seed, iterations);
    
    /* Call stress functions multiple times from different contexts */
    int total = 0;
    
    for (int i = 0; i < 5; i++) {
        int r1 = stress_computation(seed + i, iterations);
        printf("Stress computation %d: %d\n", i, r1);
        total += r1;
        
        int r2 = stress_memory_access(seed + i * 7, iterations / 2);
        printf("Memory stress %d: %d\n", i, r2);
        total ^= r2;
    }
    
    /* Additional complex expression in main to increase pressure */
    volatile int main_volatile = seed;
    for (int i = 0; i < 10; i++) {
        int complex_expr = (main_volatile * 3 + i) / (main_volatile % 7 + 1);
        complex_expr = complex_expr ^ (complex_expr >> 16);
        complex_expr = complex_expr * 1103515245 + 12345;
        total += complex_expr;
        main_volatile = main_volatile + complex_expr;
    }
    
    printf("Final result: %d\n", total);
    return total & 0xFF;
}
