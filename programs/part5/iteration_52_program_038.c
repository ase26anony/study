/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-gcse -o test test.c */
/* Additional options for more stress: -O3 -funroll-loops -march=native -fno-schedule-insns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Volatile variables to prevent optimization */
volatile int v1 = 7;
volatile int v2 = 13;
volatile int v3 = 19;
volatile int v4 = 23;
volatile int v5 = 29;

/* External function to create opaque values */
extern int rand(void);

/* Inline assembly to clobber registers and increase pressure */
#define CLOBBER_REGS() \
    __asm__ volatile ("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7")

/* Complex arithmetic expression creating many temporaries */
static inline int complex_expr(int a, int b, int c, int d, int e, int f, int g) {
    /* This creates a long dependency chain of temporaries */
    int t1 = a * b;
    int t2 = c + d;
    int t3 = e - f;
    int t4 = g % v1;
    int t5 = t1 / (t2 ? t2 : 1);
    int t6 = t3 * t4;
    int t7 = t5 + t6;
    
    /* Use volatile to force loads/stores */
    t7 += v2;
    t7 -= v3;
    
    return t7;
}

/* Function with multi-use temporary values in different control flow paths */
static int multi_use_temporary(int x, int y, int selector) {
    /* Compute a value once, use it in multiple places */
    int base = x * y + v4;
    
    /* Multi-use in different switch arms */
    switch (selector % 4) {
        case 0: {
            /* Use base with different offsets */
            int offset1 = base + 100;
            int offset2 = base - 50;
            CLOBBER_REGS();
            return offset1 * offset2 / (v1 ? v1 : 1);
        }
        case 1: {
            /* Different computation with same base */
            int offset3 = base * 2;
            int offset4 = base / 3;
            CLOBBER_REGS();
            return offset3 + offset4 - v2;
        }
        case 2: {
            /* Another use pattern */
            int offset5 = base % 17;
            int offset6 = base + v3;
            CLOBBER_REGS();
            return offset5 * offset6;
        }
        default: {
            /* Yet another pattern */
            int offset7 = base - v4;
            int offset8 = base + v5;
            CLOBBER_REGS();
            return offset7 + offset8;
        }
    }
}

/* Main stress function with loop-carried dependencies */
int stress_computation(int seed, int n) {
    int result = 0;
    volatile int loop_bound = n;  /* Prevent optimization */
    
    /* Use opaque function results */
    int r1 = rand() % 100 + 1;
    int r2 = rand() % 100 + 1;
    int r3 = rand() % 100 + 1;
    
    /* Loop with volatile increment to prevent unrolling */
    for (volatile int i = 0; i < loop_bound; i = i + 1) {
        /* Complex expression creating register pressure */
        int temp = complex_expr(
            seed + i, 
            r1 + v1, 
            r2 + v2, 
            r3 + v3, 
            v4, 
            v5, 
            i % 7 + 1
        );
        
        /* Multi-use of computed value */
        result += multi_use_temporary(temp, i, seed);
        
        /* More arithmetic to increase pressure */
        result -= complex_expr(
            result, i, r1, r2, r3, 
            v1 + v2, 
            v3 + v4
        );
        
        /* Address computation with multiple offsets */
        int base_addr = result * 4;
        int offset1 = base_addr + 16;
        int offset2 = base_addr - 8;
        int offset3 = base_addr + 32;
        
        /* Use all offsets in computation */
        result = (offset1 * offset2) / ((offset3 ? offset3 : 1) + 1);
        
        /* Clobber registers periodically */
        if (i % 3 == 0) {
            CLOBBER_REGS();
        }
    }
    
    return result;
}

/* Second stress function with different pattern */
int stress_computation2(int seed, int n) {
    int array[100];
    int result = 0;
    
    /* Initialize array with volatile values */
    for (int i = 0; i < 100; i++) {
        array[i] = (i * v1 + v2) % (v3 ? v3 : 1);
    }
    
    /* Nested loops with complex addressing */
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < 10; j++) {
            /* Compute base address multiple times */
            int base = &array[i % 100] - array;
            
            /* Use base with different offsets - candidate for rematerialization */
            int val1 = array[(base + j) % 100];
            int val2 = array[(base + j * 2) % 100];
            int val3 = array[(base + j * 3) % 100];
            
            /* Complex expression using all values */
            result += complex_expr(val1, val2, val3, i, j, v4, v5);
            
            /* More register pressure */
            result -= multi_use_temporary(result, j, seed);
        }
        
        /* Clobber between outer loop iterations */
        CLOBBER_REGS();
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 42;
    
    /* Use command line arguments to vary inputs */
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    srand(seed);
    
    /* Call stress functions multiple times */
    int total = 0;
    for (int i = 0; i < 5; i++) {
        total += stress_computation(seed + i, iterations);
        total += stress_computation2(seed + i * 2, iterations / 2);
        
        /* Vary volatile values to prevent constant propagation */
        v1 = (v1 * 13 + 7) % 97;
        v2 = (v2 * 17 + 11) % 101;
    }
    
    /* Ensure result is used */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
