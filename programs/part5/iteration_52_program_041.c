/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifdef __OPTIMIZE__
#define USE_AGGRESSIVE_PATTERNS 1
#else
#define USE_AGGRESSIVE_PATTERNS 0
#endif

/* Complex arithmetic with long dependency chain */
static inline long complex_chain(long a, long b, long c, long d, long e, long f, long g) {
    /* Creates many temporary virtual registers */
    long t1 = a * b;
    long t2 = c + d;
    long t3 = e - f;
    long t4 = g * 2;
    long t5 = t1 / (t2 + 1);
    long t6 = t3 % (t4 + 1);
    long t7 = t5 ^ t6;
    long t8 = t7 << 3;
    long t9 = t8 >> 1;
    long t10 = t9 | (a & b);
    long t11 = t10 + (c ^ d);
    long t12 = t11 * (e | f);
    long t13 = t12 - (g & t1);
    long t14 = t13 / (t2 + 2);
    long t15 = t14 % (t3 + 3);
    return t15;
}

/* Volatile variables to prevent optimization */
static volatile int vol_a = 1;
static volatile int vol_b = 2;
static volatile int vol_c = 3;
static volatile int vol_d = 4;

/* Function with register pressure patterns */
int stress_computation(int seed, int n) {
    int result = seed;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base = (vol_a * vol_b + vol_c - vol_d) ^ seed;
    
    /* Inline assembly to clobber registers (when optimized) */
#if USE_AGGRESSIVE_PATTERNS
    asm volatile (
        "# Clobber multiple registers to increase pressure\n\t"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
#endif
    
    /* Complex control flow with spatially separated uses of 'base' */
    for (int i = 0; i < n; i++) {
        /* Volatile increment prevents loop optimizations */
        vol_a++;
        
        /* Opaque function call creates unanalyzable values */
        int rand_val = rand() % 256;
        
        if (i % 3 == 0) {
            /* Use base with different computations in branch 1 */
            int val1 = base + (rand_val * 7) / 3;
            result += complex_chain(val1, i, vol_b, vol_c, rand_val, base, seed);
        } 
        else if (i % 3 == 1) {
            /* Different use of base in branch 2 */
            int val2 = (base << 2) | (rand_val & 0xFF);
            result ^= complex_chain(val2, vol_a, i, vol_d, base, rand_val, seed);
        }
        else {
            /* Third use pattern for base */
            int val3 = base - (rand_val % 17);
            result *= complex_chain(val3, vol_c, vol_d, i, rand_val, base, seed) + 1;
        }
        
        /* Address computation with multiple offsets - base address recreation */
        int array[8];
        for (int j = 0; j < 4; j++) {
            /* Different offsets from computed base address */
            array[j * 2] = base + j * 4;
            array[j * 2 + 1] = base - j * 3;
            
            /* More complex arithmetic creating virtual registers */
            array[j] += (rand_val * j) / (vol_a + 1);
        }
        
        /* Use array values to prevent elimination */
        for (int j = 0; j < 4; j++) {
            result += array[j] - array[j + 4];
        }
    }
    
    /* Final complex expression with many temporaries */
    result = (result * base) / (vol_a + 1);
    result ^= (vol_b << 3) | (vol_c >> 2);
    result += complex_chain(result, vol_d, base, seed, n, vol_a, vol_b);
    
    return result;
}

/* Second stress function with different patterns */
int stress_computation2(int seed, int n) {
    volatile int vol_x = seed;
    volatile int vol_y = seed * 2;
    int acc = 0;
    
    /* Nested loops with volatile bounds */
    for (volatile int i = 0; i < (n % 8 + 2); i++) {
        for (int j = 0; j < (vol_x % 5 + 3); j++) {
            /* Complex floating point arithmetic - creates FP virtual registers */
            float f1 = vol_x * 1.5f;
            float f2 = vol_y * 0.75f;
            float f3 = f1 * f2 + f1 / (f2 + 1.0f);
            float f4 = f3 - f1 * f2;
            
            /* Mix integer and float computations */
            acc += (int)(f3 * 100) - (int)(f4 * 50);
            acc ^= (j << i) | (i << j);
            
            /* Multi-use temporary in different contexts */
            int temp = (vol_x * i + vol_y * j) % 256;
            
            if (j % 2 == 0) {
                acc += temp * 3;
            } else {
                acc -= temp * 2;
            }
        }
        
        /* Inline assembly between loop iterations */
#if USE_AGGRESSIVE_PATTERNS
        asm volatile (
            "# More register clobbering\n\t"
            : 
            : 
            : "memory", "cc", 
              "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23"
        );
#endif
        
        vol_x++;
        vol_y--;
    }
    
    return acc;
}

int main(int argc, char *argv[]) {
    srand(time(NULL));
    
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total_result = 0;
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < iterations; i++) {
        int seed = rand() % 1000;
        
        /* Varying call patterns */
        if (i % 2 == 0) {
            total_result += stress_computation(seed, i % 10 + 5);
        } else {
            total_result ^= stress_computation2(seed, i % 8 + 3);
        }
        
        /* Additional computation between calls */
        vol_a = (vol_a * 1103515245 + 12345) & 0x7fffffff;
        vol_b = (vol_b * 1664525 + 1013904223) & 0x7fffffff;
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", total_result % 1000000);
    
    return total_result != 0 ? 0 : 1;
}
