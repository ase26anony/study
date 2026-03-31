/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-gcse test.c -o test */
/* Additional stress options: -O3 -funroll-loops -march=native -fno-schedule-insns */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Volatile variables to prevent optimization */
volatile int v1 = 1;
volatile int v2 = 2;
volatile int v3 = 3;
volatile int v4 = 4;
volatile int v5 = 5;
volatile int v6 = 6;
volatile int v7 = 7;
volatile int v8 = 8;

/* External function to create opaque values */
extern int rand(void);

/* Stress function with complex arithmetic and register pressure */
static int __attribute__((noinline)) 
stress_computation(int seed, int iterations) {
    int result = 0;
    int i, j;
    
    /* Complex arithmetic expression creating many temporaries */
    int a = rand() % 100 + seed;
    int b = rand() % 100 + v1;
    int c = rand() % 100 + v2;
    int d = rand() % 100 + v3;
    int e = rand() % 100 + v4;
    int f = rand() % 100 + v5;
    int g = rand() % 100 + v6;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base = (a * b + c / (d ? d : 1) - e % (f ? f : 1)) * g;
    
    /* Inline assembly clobbering registers to increase pressure */
    asm volatile (
        "# Clobber hard registers\n"
        : 
        : 
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
    );
    
    /* Loop with volatile variables to prevent optimization */
    for (i = 0; i < iterations; i += v7) {
        int temp = 0;
        
        /* Complex expression using volatile and base value */
        temp = (base * v1 + v2 * v3 - v4 / (v5 ? v5 : 1)) % (v6 ? v6 : 1);
        
        /* Switch with multiple uses of base - forces rematerialization decisions */
        switch (i % 4) {
            case 0:
                result += base + temp * 2;
                break;
            case 1:
                result += base - temp / 2;
                break;
            case 2:
                result += base * temp;
                break;
            case 3:
                result += base % (temp ? temp : 1);
                break;
        }
        
        /* Address computation with multiple offsets - base address reuse */
        int array[100];
        int *ptr = &array[i % 50];
        
        /* Multiple offset uses - may trigger register recreation */
        ptr[0] = base + v1;
        ptr[1] = base + v2;
        ptr[2] = base + v3;
        ptr[3] = base + v4;
        
        /* Nested loop for additional pressure */
        for (j = 0; j < (i % 10); j++) {
            /* More complex arithmetic with volatile */
            int t1 = v1 * v2 + v3 - v4;
            int t2 = v5 / (v6 ? v6 : 1) % v7;
            result += (t1 * t2 + base) * (j + 1);
        }
        
        /* Update volatile to prevent loop optimizations */
        v8 = i;
    }
    
    /* Another complex expression chain */
    int x1 = rand() % 100;
    int x2 = rand() % 100 + v1;
    int x3 = rand() % 100 + v2;
    int x4 = rand() % 100 + v3;
    int x5 = rand() % 100 + v4;
    
    result += (x1 * x2 + x3 / (x4 ? x4 : 1) - x5 % (x1 ? x1 : 1)) * base;
    
    return result;
}

/* Second stress function with different pattern */
static int __attribute__((noinline))
stress_computation2(int seed, int n) {
    int sum = 0;
    volatile int bound = n;
    
    /* Long dependency chain */
    int a = seed + v1;
    int b = a * v2 - v3;
    int c = b / (v4 ? v4 : 1) + v5;
    int d = c % (v6 ? v6 : 1) * v7;
    int e = d - v8 + a;
    int f = e * b / (c ? c : 1);
    int g = f % (d ? d : 1) + e;
    int h = g * a - b;
    
    /* Use all computed values in different control flow paths */
    if (bound > 10) {
        sum = a + b + c + d;
    } else if (bound > 5) {
        sum = e + f + g + h;
    } else {
        sum = a * e - b * f + c * g - d * h;
    }
    
    /* More inline assembly for register pressure */
    #ifdef __OPTIMIZE__
    asm volatile (
        "# More register clobbering\n"
        : 
        : 
        : "memory", "cc", 
          "r16", "r17", "r18", "r19", "r20", "r21", "r22", "r23"
    );
    #endif
    
    return sum;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int result = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    /* Call stress functions multiple times */
    for (int i = 0; i < 10; i++) {
        result += stress_computation(i * 17, iterations);
        result += stress_computation2(i * 23, iterations / 2);
        
        /* Modify volatiles between calls */
        v1 = (v1 * 3) % 100;
        v2 = (v2 * 5) % 100;
        v3 = (v3 * 7) % 100;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
