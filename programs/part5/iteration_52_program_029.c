/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-gcse -o remat_test remat_test.c */
/* Also try: gcc -O3 -funroll-loops -fno-gcse -march=native -fno-schedule-insns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Complex arithmetic with volatile and function calls */
static int complex_arithmetic(int seed, int iterations) {
    int result = 0;
    
    /* Use seed to create varying values */
    int a = seed + vol_a;
    int b = vol_b * seed;
    int c = vol_c + get_external_value() % 100;
    
    /* Multi-use temporary value - candidate for rematerialization */
    int base_computation = (a * b + c / (vol_d + 1)) % 1000;
    
    /* Complex floating point chain - creates many virtual registers */
    float f_base = (float)base_computation;
    float f1 = f_base * vol_f1 + vol_f2 / (vol_f3 + 0.1f);
    float f2 = f1 * 1.618034f - vol_f1 / vol_f2;
    float f3 = f2 + f_base * 0.707106f;
    
    /* Inline assembly that clobbers registers */
    /* This reduces available physical registers */
    asm volatile (
        "# Clobber many registers to increase pressure\n\t"
        "mov %0, %0\n\t"  /* Use the input */
        : 
        : "r" (f3)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "r12", "r13", "r14", "r15",
          "memory"
    );
    
    /* Use base_computation in multiple, separated contexts */
    /* This pattern might trigger validate_change for rematerialization */
    for (int i = 0; i < iterations; i++) {
        /* Different uses of the same computed value */
        if (i % 3 == 0) {
            result += base_computation * i;
        } else if (i % 3 == 1) {
            result -= base_computation / (i + 1);
        } else {
            result ^= base_computation + i;
        }
        
        /* More complex arithmetic creating register pressure */
        float temp = f_base * i + f1 / (i + 1) - f2 * 0.5f + f3;
        
        /* Address computation with multiple offsets - another remat candidate */
        int array[10];
        for (int j = 0; j < 5; j++) {
            /* Base address recomputation for different offsets */
            int* ptr = &array[0] + j;
            *ptr = base_computation + j * i + (int)temp;
            
            /* Use ptr with different offsets */
            if (j > 0) {
                *(ptr - 1) += *ptr / 2;
            }
        }
        
        /* Prevent loop unrolling with volatile */
        vol_a = vol_a + 1;
    }
    
    return result + (int)f3;
}

/* Stress function with control flow to obstruct optimization */
int stress_computation(int seed, int n) {
    int total = 0;
    
    /* Loop with volatile bounds to prevent optimization */
    volatile int volatile_bound = n;
    
    for (int outer = 0; outer < volatile_bound; outer++) {
        /* Opaque function call in loop - value unknown to compiler */
        int opaque = get_external_value() + outer;
        
        /* Switch with multi-use temporaries */
        switch (opaque % 4) {
            case 0: {
                /* Long dependency chain */
                int t1 = opaque * seed + vol_b;
                int t2 = t1 / (vol_c + 1) - vol_d;
                int t3 = t2 * t2 + t1;
                int t4 = t3 % 100 + t2;
                total += t4;
                
                /* Use t1 again later - candidate for rematerialization */
                if (outer % 2 == 0) {
                    total -= t1 * 2;
                }
                break;
            }
            case 1: {
                /* Different arithmetic pattern */
                float ft1 = (float)opaque * vol_f1;
                float ft2 = ft1 / vol_f2 + vol_f3;
                float ft3 = ft2 * ft2 - ft1;
                total += (int)ft3;
                break;
            }
            case 2: {
                /* Integer arithmetic with many intermediates */
                int x1 = opaque + seed;
                int x2 = x1 * x1 - seed;
                int x3 = x2 / (opaque + 1) + x1;
                int x4 = x3 * 3 - x2;
                int x5 = x4 % 50 + x3;
                total ^= x5;
                break;
            }
            default: {
                /* Mixed computation */
                total += (opaque * 1103515245 + 12345) & 0x7fffffff;
                break;
            }
        }
        
        /* Additional register pressure between switch cases */
        if (outer % 10 == 0) {
            /* Force another complex computation */
            total += complex_arithmetic(seed + outer, 3);
        }
    }
    
    return total;
}

/* Simulate external function */
int get_external_value(void) {
    static int counter = 0;
    return (counter++ * 127) % 7919;  /* Prime number for variety */
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 12345;
    
    /* Use command line arguments for variability */
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations > 1000) iterations = 1000;
        if (iterations < 10) iterations = 10;
    }
    
    srand(seed);
    
    printf("Starting stress test with seed=%d, iterations=%d\n", seed, iterations);
    
    /* Call stress function multiple times from different contexts */
    int result1 = stress_computation(seed, iterations);
    printf("Result 1: %d\n", result1);
    
    /* Change volatile values between calls */
    vol_a = rand() % 100;
    vol_b = rand() % 100;
    
    int result2 = stress_computation(seed * 2, iterations / 2);
    printf("Result 2: %d\n", result2);
    
    /* More calls with different patterns */
    int final_result = result1 + result2;
    
    for (int i = 0; i < 5; i++) {
        final_result ^= stress_computation(seed + i * 100, 20);
    }
    
    printf("Final result: %d\n", final_result);
    
    return final_result != 0 ? 0 : 1;
}
