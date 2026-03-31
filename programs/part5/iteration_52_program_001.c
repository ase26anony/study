/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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
        /* Opaque function call to prevent optimization */
        int base = get_opaque_value(v1 + i);
        
        /* Complex expression with multiple temporaries */
        int temp1 = base * v2 + v3 / (v4 + 1);
        int temp2 = temp1 % (v2 + 1) - v3 * v4;
        int temp3 = (temp2 << 2) | (temp1 & 0xFF);
        
        /* Multi-use temporary in different contexts */
        int multi_use = temp3 * 7 + base;
        
        /* Use multi_use in different control flow paths */
        if (i % 3 == 0) {
            result += multi_use * 2;
        } else if (i % 3 == 1) {
            result += multi_use / 3;
        } else {
            result += multi_use - temp2;
        }
        
        /* Address computation with multiple offsets */
        int array[10];
        for (int j = 0; j < 5; j++) {
            /* Base address computation that might be rematerialized */
            int* base_ptr = &array[j];
            result += *(base_ptr + 0) + *(base_ptr + 1) + *(base_ptr + 2);
        }
        
        /* Inline assembly to clobber registers and increase pressure */
        #ifdef __OPTIMIZE__
        asm volatile (
            "# Clobber multiple registers\n"
            "mov r0, %0\n"
            "mov r1, %1\n"
            "add r0, r0, r1\n"
            : 
            : "r" (temp1), "r" (temp2)
            : "r0", "r1", "memory", "cc"
        );
        #endif
        
        /* More complex floating point operations for FP register pressure */
        volatile float f1 = (float)v1;
        volatile float f2 = (float)v2;
        float f_temp = f1 * f2 + (float)temp3 / f1 - f2 * f1;
        result += (int)f_temp;
    }
    
    return result;
}

/* Another stress function with different pattern */
int stress_computation2(int seed, int n) {
    volatile long v1 = seed;
    volatile long v2 = seed * 3;
    int result = 0;
    
    /* Switch statement with multi-use temporaries */
    for (int i = 0; i < n; i++) {
        int base = get_opaque_value(seed + i);
        
        switch (i % 4) {
            case 0: {
                int temp = base * 2 + v1;
                result += temp * 3;
                /* Use temp again later in same basic block */
                result -= temp / 2;
                break;
            }
            case 1: {
                int temp = base + v2;
                result += temp << 1;
                /* Different use pattern */
                result |= temp & 0xFFFF;
                break;
            }
            case 2: {
                int temp = base - v1;
                result += temp % 100;
                /* Force another computation */
                result ^= temp * 7;
                break;
            }
            default: {
                int temp = base ^ v2;
                result += temp;
                /* Multiple uses in complex expression */
                result += (temp * 3) / (temp + 1);
                break;
            }
        }
        
        /* Chain of dependent computations */
        int chain1 = base + i;
        int chain2 = chain1 * chain1 - base;
        int chain3 = chain2 % 256 + chain1;
        int chain4 = chain3 | (chain2 & 0xFF00);
        result += chain4;
    }
    
    return result;
}

/* Opaque function implementation */
int get_opaque_value(int seed) {
    /* Use system rand() to prevent compiler analysis */
    srand(seed);
    return rand() % 1000;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    int total = 0;
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < 10; i++) {
        total += stress_computation(i * 17, iterations);
        total += stress_computation2(i * 23, iterations / 2);
        
        /* Alternate between different call patterns */
        if (i % 2 == 0) {
            total += stress_computation(total, 5);
        } else {
            total += stress_computation2(total, 5);
        }
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
