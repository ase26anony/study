/* Test program to trigger virtual register creation in early-remat.cc */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* External function to create opaque values */
extern int get_opaque_value(int seed);

/* Stress function with complex arithmetic and register pressure */
int stress_computation(int seed, int iterations) {
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < iterations; i++) {
        /* Multi-use temporary value - candidate for rematerialization */
        int base = (vol_a * seed) + (vol_b % (seed + 1)) - (vol_c ^ (seed * 2));
        
        /* Complex floating-point expression with many intermediates */
        float f_temp = vol_f1 * vol_f2 + vol_f3 / (vol_f1 + 1.0f) - 
                      (vol_f2 * 2.0f) / (vol_f3 - 0.5f) + 
                      (float)base * 0.1f;
        
        /* Address computation with multiple offsets */
        int array[100];
        int *ptr = &array[seed % 50];
        
        /* Use base with different offsets - may trigger register recreation */
        int val1 = *(ptr + base % 10);
        int val2 = *(ptr + (base * 2) % 10);
        int val3 = *(ptr + (base / 2) % 10);
        
        /* Inline assembly that clobbers registers */
        asm volatile (
            "# Clobber hard registers to increase pressure\n"
            "mov r0, %0\n"
            "mov r1, %1\n"
            "mov r2, %2\n"
            : 
            : "r" (val1), "r" (val2), "r" (val3)
            : "r0", "r1", "r2", "memory"
        );
        
        /* More complex arithmetic using opaque function results */
        int opaque = get_opaque_value(seed + i);
        int complex_expr = (opaque * base) / (val1 + 1) + 
                          (opaque % (val2 + 2)) * (val3 - 3) -
                          (base ^ opaque) + (val1 * val2) / (val3 + 1);
        
        /* Control flow that obstructs optimization */
        if (opaque % 7 == 0) {
            result += complex_expr * 2;
        } else if (opaque % 5 == 0) {
            result += complex_expr / 2;
        } else {
            result += complex_expr;
        }
        
        /* Volatile update to prevent loop optimizations */
        seed += vol_d;
    }
    
    return result;
}

/* Another stress function with different patterns */
int stress_memory_access(int seed, int n) {
    int total = 0;
    volatile int vol_counter = 0;
    
    /* Loop with volatile bound to prevent unrolling */
    for (volatile int i = 0; i < n; i = i + 1) {
        /* Long dependency chain */
        int a = seed + i;
        int b = a * vol_a - vol_b;
        int c = b / (vol_c + 1) + vol_d;
        int d = c ^ (a * b);
        int e = d % (c + 2) * (b - 3);
        
        /* Multi-use value in different contexts */
        int multi_use = e * 2 - a;
        
        /* Use in different expressions */
        if (i % 3 == 0) {
            total += multi_use * 3;
        } else if (i % 3 == 1) {
            total += multi_use / 2;
        } else {
            total += multi_use + b;
        }
        
        /* More inline assembly clobbering */
        asm volatile (
            "# More register clobbering\n"
            "add r3, %0, %1\n"
            "sub r4, %0, %1\n"
            : 
            : "r" (total), "r" (multi_use)
            : "r3", "r4", "cc"
        );
        
        vol_counter++;
    }
    
    return total;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use rand() to create compiler-opaque values */
    static int initialized = 0;
    if (!initialized) {
        srand(seed);
        initialized = 1;
    }
    return rand() % 1000 + 1;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int result = 0;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Initialize volatile variables with some data */
    vol_a = (argc > 2) ? atoi(argv[2]) : 1;
    vol_b = (argc > 3) ? atoi(argv[3]) : 2;
    vol_c = (argc > 4) ? atoi(argv[4]) : 3;
    vol_d = (argc > 5) ? atoi(argv[5]) : 4;
    
    /* Call stress functions multiple times from different contexts */
    result += stress_computation(42, iterations);
    result += stress_computation(123, iterations / 2);
    result += stress_memory_access(456, iterations);
    result += stress_computation(789, iterations / 3);
    
    /* Ensure result is used to prevent elimination */
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
