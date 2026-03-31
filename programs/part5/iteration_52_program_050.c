/* Test program to trigger virtual register creation and rematerialization logic */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Complex arithmetic with volatile variables */
static int stress_computation(int seed, int n) {
    volatile int v1 = seed;
    volatile int v2 = seed * 2;
    volatile int v3 = seed + 1;
    volatile int v4 = seed - 1;
    
    int result = 0;
    
    /* Complex arithmetic chain creating many temporaries */
    for (int i = 0; i < n; i++) {
        /* Multi-use temporary value - candidate for rematerialization */
        int base = v1 * v2 + v3 / (v4 + 1) - (v2 % (v3 + 1));
        
        /* Use base in multiple contexts with different operations */
        if (i % 3 == 0) {
            result += base * 2 + get_external_value();
        } else if (i % 3 == 1) {
            result -= base / 2 + (v1 ^ v2);
        } else {
            result ^= base + v3 - v4;
        }
        
        /* Address computation with multiple offsets */
        int array[10];
        int *ptr = &array[0];
        
        /* Multiple uses of ptr with different offsets */
        ptr[i % 10] = base + i;
        ptr[(i + 1) % 10] = base - i;
        ptr[(i + 2) % 10] = base * i;
        
        /* Inline assembly that clobbers registers */
        /* This increases register pressure */
        asm volatile (
            "# Dummy assembly to clobber registers\n"
            "mov %0, %0\n"
            : "+r" (result)
            : 
            : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
        );
        
        /* More complex arithmetic with volatile accesses */
        v1 = v1 * 1103515245 + 12345;
        v2 = v2 ^ (v1 >> 16);
        v3 = v3 + (v2 % 256);
        v4 = v4 - (v3 & 0xFF);
    }
    
    return result;
}

/* Another stress function with different patterns */
static int stress_computation2(int seed, int n) {
    volatile long v1 = seed;
    volatile long v2 = seed * 3;
    volatile long v3 = seed + 2;
    
    long result = 0;
    
    /* Loop with volatile bounds to prevent optimization */
    volatile int loop_bound = n;
    for (volatile int i = 0; i < loop_bound; i = i + 1) {
        /* Very long dependency chain */
        long temp1 = v1 * v2 + v3;
        long temp2 = temp1 / (v2 + 1) * v3;
        long temp3 = temp2 - v1 % (v3 + 1);
        long temp4 = temp3 ^ (v2 & 0xFFFF);
        long temp5 = temp4 + (v1 >> 8);
        long temp6 = temp5 * 16807 % 2147483647;
        
        /* Multi-use of temp6 in switch statement */
        switch (i % 4) {
            case 0:
                result += temp6 * 2;
                break;
            case 1:
                result -= temp6 / 3;
                break;
            case 2:
                result ^= temp6 + 1;
                break;
            case 3:
                result = result * 3 + temp6;
                break;
        }
        
        /* More register pressure */
        asm volatile (
            "# More register clobbering\n"
            : 
            : 
            : "r6", "r7", "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15"
        );
    }
    
    return (int)result;
}

/* Function with floating point stress */
static float stress_float_computation(float seed, int n) {
    volatile float f1 = seed;
    volatile float f2 = seed * 1.5f;
    volatile float f3 = seed + 0.5f;
    
    float result = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Complex FP expressions creating many virtual registers */
        float temp = f1 * f2 + f3 / (f2 + 1.0f) - f1 * f3;
        
        /* Use temp multiple times in different expressions */
        result += temp * 2.0f;
        result -= temp / 1.5f;
        result *= temp + 1.0f;
        
        /* Update volatiles to prevent optimization */
        f1 = f1 * 1.1f + 0.1f;
        f2 = f2 * 0.9f - 0.1f;
        f3 = f3 + 0.01f;
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    
    srand(time(NULL));
    int seed = rand();
    
    printf("Starting stress tests...\n");
    
    /* Call stress functions multiple times from different contexts */
    int total = 0;
    
    /* First computation path */
    total += stress_computation(seed, iterations);
    
    /* Second with different parameters */
    total += stress_computation(seed + 1, iterations / 2);
    
    /* Third with different function */
    total += stress_computation2(seed, iterations);
    
    /* Floating point stress */
    float ftotal = stress_float_computation((float)seed / 1000.0f, iterations);
    total += (int)ftotal;
    
    /* More calls to increase code size and register pressure */
    for (int i = 0; i < 5; i++) {
        total += stress_computation(seed + i * 100, 10);
    }
    
    printf("Result: %d\n", total);
    
    /* Prevent dead code elimination */
    volatile int dummy = total;
    asm volatile ("" : : "r" (dummy));
    
    return total != 0 ? 0 : 1;
}

/* Dummy implementation of external function */
int get_external_value(void) {
    static int counter = 0;
    return counter++ * 1103515245 + 12345;
}
