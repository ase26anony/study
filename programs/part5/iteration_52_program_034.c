/* Compile with: gcc -O2 -fno-omit-frame-pointer -fno-gcse -o test test.c */
/* Additional options for more stress: -O3 -funroll-loops -march=native -fno-schedule-insns */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* External function to create opaque values */
extern int get_external_value(void);

/* Volatile variables to prevent optimization */
volatile int vol_a = 1, vol_b = 2, vol_c = 3, vol_d = 4;
volatile float vol_f1 = 1.5f, vol_f2 = 2.5f, vol_f3 = 3.5f;

/* Stress function with complex arithmetic and register pressure */
int stress_computation(int seed, int n) {
    int result = 0;
    int i, j;
    
    /* Complex arithmetic with volatile variables - creates many temporaries */
    int base1 = vol_a * vol_b + vol_c / vol_d - seed % 7;
    float base2 = vol_f1 * vol_f2 + vol_f3 / vol_f1 - (float)(seed % 11);
    
    /* Multi-use temporary values in different contexts */
    int multi_use_temp = base1 * 3 - base2;
    
    /* Inline assembly to clobber hard registers (adjust for your architecture) */
    #ifdef __OPTIMIZE__
    asm volatile (
        "# Clobber registers to increase pressure\n\t"
        "mov %0, %0\n\t"
        : 
        : "r" (seed)
        : "r0", "r1", "r2", "r3", "r4", "r5", "memory"
    );
    #endif
    
    /* Loop with volatile bounds to prevent optimization */
    volatile int loop_bound = n;
    for (i = 0; i < loop_bound; i++) {
        /* Complex floating-point expression chain */
        float f1 = (float)vol_a * 1.1f + (float)vol_b * 2.2f;
        float f2 = (float)vol_c * 3.3f / (float)vol_d * 4.4f;
        float f3 = f1 * f2 - (float)(i % 5) + base2;
        
        /* Integer arithmetic with opaque function calls */
        int opaque_val = get_external_value() % 100;
        int chain1 = opaque_val * 7 + seed / 3;
        int chain2 = chain1 % 13 * 11 - i;
        int chain3 = chain2 + multi_use_temp * (i % 3);
        
        /* Address computation with multiple offsets */
        int array[100];
        int *base_ptr = &array[i % 50];
        
        /* Use base pointer with different offsets - candidate for rematerialization */
        int val1 = base_ptr[0] + chain3;
        int val2 = base_ptr[5] * chain3;
        int val3 = base_ptr[10] - chain3;
        int val4 = base_ptr[15] / (chain3 != 0 ? chain3 : 1);
        
        /* Switch with multi-use temporary in different arms */
        switch (i % 4) {
            case 0:
                result += val1 * multi_use_temp;
                break;
            case 1:
                result += val2 + multi_use_temp / 2;
                break;
            case 2:
                result += val3 - multi_use_temp * 3;
                break;
            case 3:
                result += val4 % (multi_use_temp != 0 ? multi_use_temp : 1);
                break;
        }
        
        /* More complex arithmetic to increase register pressure */
        for (j = 0; j < 3; j++) {
            float complex_f = f3 * (float)j + (float)result * 0.1f;
            int complex_i = (int)complex_f * vol_a + vol_b - vol_c % vol_d;
            result ^= complex_i;
            
            /* Another inline asm to clobber registers mid-loop */
            #ifdef __OPTIMIZE__
            asm volatile (
                "# Mid-loop clobber\n\t"
                : 
                : 
                : "r0", "r1", "r2", "r3", "memory"
            );
            #endif
        }
        
        /* Volatile operation to force memory access */
        vol_a = (vol_a + 1) % 100;
    }
    
    /* Final complex expression using all accumulated values */
    result = result * base1 + (int)base2 - multi_use_temp * 2;
    
    return result;
}

/* Second stress function with different pattern */
int stress_computation2(int seed, int n) {
    int result = seed;
    volatile int v = n;
    
    /* Long dependency chain of integer arithmetic */
    int a = get_external_value() % 256;
    int b = a * 3 + seed;
    int c = b / 2 - a % 7;
    int d = c * 5 + seed % 11;
    int e = d ^ (a * b);
    int f = e + c * d - b;
    int g = f % 97 + a * c;
    int h = g * 3 - d / 2;
    
    /* Use all computed values in different expressions */
    int *ptr_array[10];
    int data[100];
    
    for (int i = 0; i < v && i < 10; i++) {
        ptr_array[i] = &data[i * 10];
        
        /* Compute address with multiple uses */
        int *base = ptr_array[i];
        int offset1 = (a + i) % 10;
        int offset2 = (b + i * 2) % 10;
        int offset3 = (c + i * 3) % 10;
        
        /* Multiple uses of base with different offsets */
        base[offset1] = h + i;
        base[offset2] = g * i;
        base[offset3] = f - i;
        
        result += base[offset1] + base[offset2] - base[offset3];
    }
    
    return result;
}

/* Mock external function */
int get_external_value(void) {
    static int counter = 0;
    return counter++ + rand();
}

int main(int argc, char *argv[]) {
    int iterations = 100;
    int seed = 12345;
    int total_result = 0;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 100;
    }
    if (argc > 2) {
        seed = atoi(argv[2]);
    }
    
    srand(seed);
    
    /* Call stress functions multiple times from different contexts */
    for (int i = 0; i < iterations; i++) {
        int n = 10 + (i % 20);  /* Varying size parameter */
        
        /* Alternate between two different stress functions */
        if (i % 2 == 0) {
            total_result += stress_computation(seed + i, n);
        } else {
            total_result += stress_computation2(seed - i, n);
        }
        
        /* Modify volatile between calls */
        vol_b = (vol_b * 3) % 100;
        vol_f2 = vol_f2 * 1.1f;
        
        /* Prevent loop unrolling from simplifying too much */
        if (i % 7 == 0) {
            seed = get_external_value() % 1000;
        }
    }
    
    printf("Result: %d\n", total_result);
    return total_result != 0 ? 0 : 1;
}
